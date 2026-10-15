// AirSense v4 (rev B) firmware — tickets 05 + 06: sensor stack, cadence
// scheduler, BLE (frozen Phase-2 contract + v4 extensions), history buffer.
// Serial protocol:
//   boot:   CADENCE loaded ... / INIT <name> ok|absent / BLE advertising /
//           READY
//   cycle:  DATA t=<ms> co2= temp= rh= voc= nox= pm1_0= pm2_5= pm10= press=
//           CYCLE n=<n> next_in=<s>s mode=<mode>
//   input:  cadence continuous | cadence 60 | cadence 300 | cadence custom <s>
//           cadence? | sample | reboot (persistence check)
// BLE protocol: app/constants/ble-protocol.md. An active app connection
// forces continuous sampling; the persisted cadence returns on disconnect.
#include <Arduino.h>
#include <Wire.h>

#include "ble_service.h"
#include "cadence.h"
#include "history.h"
#include "pins.h"
#include "scheduler.h"
#include "ui.h"

#ifdef AIRSENSE_FAKE_SENSORS
#include "fake_drivers.h"
static fake::FakeScd41 scd41;
static fake::FakeSgp41 sgp41;
static fake::FakeSen50 sen50;
static fake::FakeBmp3xx bmp3xx;
#else
#include "bmp3xx_driver.h"
#include "scd41_driver.h"
#include "sen50_driver.h"
#include "sgp41_driver.h"
static Scd41Driver scd41;
static Sgp41Driver sgp41(scd41);  // RH/T compensation from the SCD41
static Sen50Driver sen50;
static Bmp3xxDriver bmp3xx;
#endif

// Order matters: SCD41 reads first so its RH/T feeds SGP41 compensation.
static ISensorDriver* drivers[] = {&scd41, &bmp3xx, &sen50, &sgp41};
constexpr size_t kDriverCount = sizeof(drivers) / sizeof(drivers[0]);

static CadenceScheduler scheduler(drivers, kDriverCount, pins::kBoostEn);
static history::Buffer historyBuf;
static BleService ble;
static Ui ui;
// The user's persisted cadence. The scheduler runs a forced-continuous
// override while an app is connected OR the screen is on (spec: both force
// continuous sampling); this is what returns when the override ends.
static cadence::Cadence g_cadence;
static bool forcedContinuous = false;

static void handleCommand(const String& line);
static void onUiCadence(const cadence::Cadence& c);

static void onCycleReport(const Readings& r, uint32_t tMs) {
  // No pushes while a dump streams: the ring's oldest-record mapping must
  // stay frozen, and the app gets those cycles live via notifications.
  if (!ble.isDumping()) historyBuf.push(r, tMs);
  ble.publish(r, tMs);
  ui.setData(r, tMs);
}

// Level-based override: track (BLE connected || screen on) and switch the
// scheduler between forced continuous and the persisted cadence on edges.
static void updateForcing() {
  bool want = ble.isConnected() || ui.screenOn();
  if (want == forcedContinuous) return;
  forcedContinuous = want;
  cadence::Cadence target;
  if (want) {
    target.mode = cadence::Mode::Continuous;
  } else {
    target = g_cadence;
  }
  // Skip the no-op transition (e.g. persisted cadence IS continuous): a
  // reconfigure would needlessly reset the SGP41 gas-index algorithms.
  if (target.mode == scheduler.current().mode &&
      target.intervalMs() == scheduler.current().intervalMs()) {
    return;
  }
  scheduler.applyCadence(target, /*firstDelayMs=*/0);
}

void setup() {
  Serial.begin(115200);
  delay(1500);  // USB-CDC settle
  Serial.println();
  Serial.println("AirSense v4 — sensor stack + cadence + BLE (tickets 05+06)");

  pinMode(pins::kBoostEn, OUTPUT);
  digitalWrite(pins::kBoostEn, HIGH);  // power the 5 V rail for init probing
  pinMode(pins::kButton, INPUT_PULLUP);
  Wire.begin(pins::kI2cSda, pins::kI2cScl);
  Wire.setClock(100000);  // SCD41 max is 100 kHz
  delay(100);  // rail + sensor power-on settle

  g_cadence = cadence::load();
  Serial.print("CADENCE loaded mode=");
  Serial.print(g_cadence.modeName());
  Serial.print(" interval=");
  Serial.print(g_cadence.intervalMs());
  Serial.println("ms");

  bool oledOk = ui.begin();  // boot splash shows during sensor init
  Serial.print("INIT OLED");
  Serial.println(oledOk ? " ok" : " absent");
  ui.setCadenceCallback(onUiCadence);

  for (size_t i = 0; i < kDriverCount; ++i) {
    bool ok = drivers[i]->begin();
    Serial.print("INIT ");
    Serial.print(drivers[i]->name());
    Serial.println(ok ? " ok" : " absent");
  }

  scheduler.setReportHook(onCycleReport);
  scheduler.applyCadence(g_cadence, /*firstDelayMs=*/0);
  ble.begin(g_cadence, &historyBuf);
  Serial.println("BLE advertising");
  ui.setStatus(g_cadence, false);
  ui.sleepAfterBoot();  // screen starts off; a button press wakes it
  Serial.println("READY");
}

// A cadence change (serial, BLE write, or UI menu) persists immediately but
// only drives the scheduler once no continuous override is active.
static void applyPersistedCadence() {
  cadence::save(g_cadence);
  ble.setPersisted(g_cadence);
  if (forcedContinuous) {
    Serial.print("CADENCE saved mode=");
    Serial.print(g_cadence.modeName());
    Serial.println(" (applies when continuous override ends)");
  } else {
    scheduler.applyCadence(g_cadence, /*firstDelayMs=*/0);
  }
}

static void onUiCadence(const cadence::Cadence& c) {
  g_cadence = c;
  applyPersistedCadence();
}

void loop() {
  scheduler.tick();

  BleService::PollResult pr = ble.poll();
  if (pr.cadenceChanged) {
    g_cadence = pr.newCadence;
    applyPersistedCadence();
  }
  ui.setStatus(g_cadence, ble.isConnected());
  ui.tick();
  updateForcing();  // reacts to BLE connect/disconnect and screen on/off

  static String lineBuf;
  while (Serial.available() > 0) {
    char ch = static_cast<char>(Serial.read());
    if (ch == '\n' || ch == '\r') {
      if (lineBuf.length() > 0) handleCommand(lineBuf);
      lineBuf = "";
    } else if (lineBuf.length() < 64) {
      lineBuf += ch;
    }
  }
  delay(10);
}

static void handleCommand(const String& line) {
  String cmd = line;
  cmd.trim();

  if (cmd == "sample") {
    scheduler.triggerNow();
    return;
  }
  if (cmd == "reboot") {
    Serial.println("REBOOT");
    Serial.flush();
    ESP.restart();
    return;
  }
  if (cmd == "cadence?") {
    Serial.print("CADENCE mode=");
    Serial.print(g_cadence.modeName());
    Serial.print(" interval=");
    Serial.print(g_cadence.intervalMs());
    Serial.print("ms");
    if (ble.isConnected()) Serial.print(" (forced continuous: app connected)");
    Serial.println();
    return;
  }
  if (!cmd.startsWith("cadence ")) {
    Serial.print("ERR unknown command: ");
    Serial.println(cmd);
    return;
  }

  String arg = cmd.substring(8);
  arg.trim();
  cadence::Cadence c = g_cadence;
  if (arg == "continuous") {
    c.mode = cadence::Mode::Continuous;
  } else if (arg == "60") {
    c.mode = cadence::Mode::Sec60;
  } else if (arg == "300") {
    c.mode = cadence::Mode::Sec300;
  } else if (arg.startsWith("custom ")) {
    long s = arg.substring(7).toInt();
    if (s < static_cast<long>(cadence::kMinCustomSeconds) ||
        s > static_cast<long>(cadence::kMaxCustomSeconds)) {
      Serial.print("ERR custom interval range is ");
      Serial.print(cadence::kMinCustomSeconds);
      Serial.print("s..");
      Serial.print(cadence::kMaxCustomSeconds);
      Serial.println("s");
      return;
    }
    c.mode = cadence::Mode::Custom;
    c.customSeconds = static_cast<uint32_t>(s);
  } else {
    Serial.print("ERR bad cadence arg: ");
    Serial.println(arg);
    return;
  }
  g_cadence = c;
  applyPersistedCadence();
}
