#include "ble_service.h"

#include <Arduino.h>
#include <BLE2902.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>

#include <string.h>

#include "ble_protocol.h"

namespace {

// History dump framing (see app/constants/ble-protocol.md):
//   record frame: uint16 seq (0..count-1) + 24-byte history::Record = 26 B
//   end frame:    uint16 0xFFFF + uint16 sent + uint16 status = 6 B
// A 26-byte notification needs ATT MTU >= 29 (3-byte notify overhead).
constexpr uint16_t kEndSeq = 0xFFFF;
constexpr uint16_t kStatusOk = 0;
constexpr uint16_t kStatusMtuTooSmall = 1;
constexpr uint16_t kMinMtuForDump = 29;
// Paced so Bluedroid's notification buffers never congest (~200 frames/s at
// loop's 10 ms tick; a full 720-record dump streams in ~3.6 s). The frame
// seq numbers let the app detect any drop and re-request.
constexpr uint8_t kFramesPerPoll = 2;
constexpr uint32_t kAdvRestartDelayMs = 500;  // Phase-2 reconnect pattern

struct State {
  BLEServer* server = nullptr;

  // Frozen Phase-2 characteristics
  BLECharacteristic* co2 = nullptr;
  BLECharacteristic* temp = nullptr;
  BLECharacteristic* hum = nullptr;
  BLECharacteristic* voc = nullptr;
  BLECharacteristic* nox = nullptr;

  // v4 additions
  BLECharacteristic* pressure = nullptr;
  BLECharacteristic* pm1Std = nullptr;
  BLECharacteristic* pm25Std = nullptr;
  BLECharacteristic* pm10Std = nullptr;
  BLECharacteristic* pm1 = nullptr;
  BLECharacteristic* pm25 = nullptr;
  BLECharacteristic* pm10 = nullptr;
  BLECharacteristic* cadenceChar = nullptr;
  BLECharacteristic* historyChar = nullptr;

  history::Buffer* buf = nullptr;

  // Set from Bluedroid callbacks, drained by poll() in loop context.
  volatile bool connected = false;
  volatile bool connectPending = false;
  volatile bool disconnectPending = false;
  volatile uint16_t mtu = 23;
  volatile bool cadenceWritePending = false;
  uint8_t cadenceWriteBuf[8] = {0};
  volatile uint8_t cadenceWriteLen = 0;
  volatile uint8_t historyCmdPending = 0;  // 0 none, 1 start, 2 abort

  // Mirror of the persisted cadence wire value (mode u8 + customSeconds u32,
  // LE). Written by setPersisted() in loop context, read by the Bluedroid
  // task in CadenceCb::onRead — guarded like history::Buffer::snapshot.
  portMUX_TYPE persistedMux = portMUX_INITIALIZER_UNLOCKED;
  uint8_t persistedValue[5] = {0};

  // poll()-owned
  uint32_t advRestartAt = 0;  // 0 = no restart scheduled
  bool dumpActive = false;
  uint16_t dumpIdx = 0;
  uint16_t dumpCount = 0;  // snapshot of count() at dump start
};

State g;

void setU16(BLECharacteristic* c, uint16_t v) {
  c->setValue(reinterpret_cast<uint8_t*>(&v), 2);  // ESP32 is little-endian
}
void setI16(BLECharacteristic* c, int16_t v) {
  c->setValue(reinterpret_cast<uint8_t*>(&v), 2);
}
void setU32(BLECharacteristic* c, uint32_t v) {
  c->setValue(reinterpret_cast<uint8_t*>(&v), 4);
}

class ServerCb : public BLEServerCallbacks {
  void onConnect(BLEServer*) override {
    g.connected = true;
    g.connectPending = true;
  }
  void onDisconnect(BLEServer*) override {
    g.connected = false;
    g.disconnectPending = true;
  }
  void onMtuChanged(BLEServer*, esp_ble_gatts_cb_param_t* param) override {
    g.mtu = param->mtu.mtu;
  }
};

class CadenceCb : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic* c) override {
    std::string v = c->getValue();
    size_t n = v.size();
    if (n > sizeof(g.cadenceWriteBuf)) n = sizeof(g.cadenceWriteBuf);
    memcpy(g.cadenceWriteBuf, v.data(), n);
    g.cadenceWriteLen = static_cast<uint8_t>(n);
    g.cadenceWritePending = true;
  }
  void onRead(BLECharacteristic* c) override {
    // The GATT stack stores any written bytes (valid or not, 1- or 5-byte
    // form) into the characteristic before onWrite runs, so READ must be
    // regenerated from the persisted setting. Runs in the Bluedroid task.
    uint8_t v[5];
    portENTER_CRITICAL(&g.persistedMux);
    memcpy(v, g.persistedValue, sizeof(v));
    portEXIT_CRITICAL(&g.persistedMux);
    c->setValue(v, sizeof(v));
  }
};

class HistoryCb : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic* c) override {
    std::string v = c->getValue();
    if (v.size() >= 1) g.historyCmdPending = (v[0] == 0x01) ? 1 : 2;
  }
  void onRead(BLECharacteristic* c) override {
    // 16-byte status: count u16, capacity u16, oldest_t u32, newest_t u32,
    // now_t u32 (all LE). now_t lets the app map uptime to wall-clock time.
    // Runs in the Bluedroid task: use the buffer's consistent snapshot.
    uint8_t s[16] = {0};
    uint16_t count = 0;
    uint32_t oldest = 0, newest = 0;
    uint16_t cap = g.buf ? g.buf->capacity() : 0;
    if (g.buf) g.buf->snapshot(count, oldest, newest);
    uint32_t now = millis() / 1000;
    memcpy(s + 0, &count, 2);
    memcpy(s + 2, &cap, 2);
    memcpy(s + 4, &oldest, 4);
    memcpy(s + 8, &newest, 4);
    memcpy(s + 12, &now, 4);
    c->setValue(s, sizeof(s));
  }
};

ServerCb serverCb;
CadenceCb cadenceCb;
HistoryCb historyCb;

BLECharacteristic* makeChar(BLEService* svc, const char* uuid,
                            uint32_t props) {
  BLECharacteristic* c = svc->createCharacteristic(uuid, props);
  c->addDescriptor(new BLE2902());
  return c;
}

void sendEndFrame(uint16_t sent, uint16_t status) {
  uint8_t f[6];
  memcpy(f + 0, &kEndSeq, 2);
  memcpy(f + 2, &sent, 2);
  memcpy(f + 4, &status, 2);
  g.historyChar->setValue(f, sizeof(f));
  g.historyChar->notify();
}

}  // namespace

void BleService::begin(const cadence::Cadence& persisted,
                       history::Buffer* buf) {
  g.buf = buf;

  BLEDevice::init(bleproto::kDeviceName);
  BLEDevice::setMTU(247);  // allow large MTU; the client still negotiates
  g.server = BLEDevice::createServer();
  g.server->setCallbacks(&serverCb);

  constexpr uint32_t kValueProps =
      BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY;

  // Environmental Sensing service: 7 characteristics x ~4 handles + service.
  BLEService* env =
      g.server->createService(BLEUUID(bleproto::kEnvServiceUuid), 32);
  g.co2 = makeChar(env, bleproto::kCo2Uuid, kValueProps);
  g.temp = makeChar(env, bleproto::kTemperatureUuid, kValueProps);
  g.hum = makeChar(env, bleproto::kHumidityUuid, kValueProps);
  g.pressure = makeChar(env, bleproto::kPressureUuid, kValueProps);
  g.pm1Std = makeChar(env, bleproto::kPm1StdUuid, kValueProps);
  g.pm25Std = makeChar(env, bleproto::kPm25StdUuid, kValueProps);
  g.pm10Std = makeChar(env, bleproto::kPm10StdUuid, kValueProps);

  // Custom service: 7 characteristics.
  BLEService* custom =
      g.server->createService(BLEUUID(bleproto::kCustomServiceUuid), 32);
  g.voc = makeChar(custom, bleproto::kVocUuid, kValueProps);
  g.nox = makeChar(custom, bleproto::kNoxUuid, kValueProps);
  g.pm1 = makeChar(custom, bleproto::kPm1Uuid, kValueProps);
  g.pm25 = makeChar(custom, bleproto::kPm25Uuid, kValueProps);
  g.pm10 = makeChar(custom, bleproto::kPm10Uuid, kValueProps);
  g.cadenceChar = makeChar(custom, bleproto::kCadenceUuid,
                           kValueProps | BLECharacteristic::PROPERTY_WRITE);
  g.cadenceChar->setCallbacks(&cadenceCb);
  g.historyChar = makeChar(custom, bleproto::kHistoryUuid,
                           kValueProps | BLECharacteristic::PROPERTY_WRITE);
  g.historyChar->setCallbacks(&historyCb);

  setPersisted(persisted);

  env->start();
  custom->start();

  // Frozen Phase-2 advertising pattern.
  BLEAdvertising* adv = BLEDevice::getAdvertising();
  adv->addServiceUUID(bleproto::kEnvServiceUuid);
  adv->addServiceUUID(bleproto::kCustomServiceUuid);
  adv->setScanResponse(true);
  adv->setMinPreferred(0x06);
  adv->setMinPreferred(0x12);
  BLEDevice::startAdvertising();
}

void BleService::publish(const Readings& r, uint32_t /*tMs*/) {
  setU16(g.co2, bleproto::packCo2(r));
  setI16(g.temp, bleproto::packTemp(r));
  setU16(g.hum, bleproto::packHumidity(r));
  setU16(g.voc, bleproto::packVoc(r));
  setU16(g.nox, bleproto::packNox(r));
  setU32(g.pressure, bleproto::packPressurePax10(r));
  setU16(g.pm1Std, bleproto::packPmSfloat(r.pm1_0, r.sen50Valid));
  setU16(g.pm25Std, bleproto::packPmSfloat(r.pm2_5, r.sen50Valid));
  setU16(g.pm10Std, bleproto::packPmSfloat(r.pm10, r.sen50Valid));
  setU16(g.pm1, bleproto::packPmX10(r.pm1_0, r.sen50Valid));
  setU16(g.pm25, bleproto::packPmX10(r.pm2_5, r.sen50Valid));
  setU16(g.pm10, bleproto::packPmX10(r.pm10, r.sen50Valid));
  if (!g.connected) return;
  g.co2->notify();
  g.temp->notify();
  g.hum->notify();
  g.voc->notify();
  g.nox->notify();
  g.pressure->notify();
  g.pm1Std->notify();
  g.pm25Std->notify();
  g.pm10Std->notify();
  g.pm1->notify();
  g.pm25->notify();
  g.pm10->notify();
}

void BleService::setPersisted(const cadence::Cadence& c) {
  uint8_t v[5];
  v[0] = static_cast<uint8_t>(c.mode);
  uint32_t cs = c.customSeconds;
  memcpy(v + 1, &cs, 4);
  portENTER_CRITICAL(&g.persistedMux);
  memcpy(g.persistedValue, v, sizeof(v));
  portEXIT_CRITICAL(&g.persistedMux);
  g.cadenceChar->setValue(v, sizeof(v));
  if (g.connected) g.cadenceChar->notify();
}

bool BleService::isConnected() const { return g.connected; }

bool BleService::isDumping() const { return g.dumpActive; }

BleService::PollResult BleService::poll() {
  PollResult res;

  if (g.connectPending) {
    g.connectPending = false;
    res.connected = true;
    Serial.println("BLE connect");
  }
  if (g.disconnectPending) {
    g.disconnectPending = false;
    res.disconnected = true;
    g.dumpActive = false;  // a dump never survives its connection
    g.mtu = 23;
    g.advRestartAt = millis() + kAdvRestartDelayMs;
    Serial.println("BLE disconnect");
  }
  if (g.advRestartAt != 0 &&
      static_cast<int32_t>(millis() - g.advRestartAt) >= 0) {
    g.advRestartAt = 0;
    BLEDevice::startAdvertising();
    Serial.println("BLE advertising");
  }

  if (g.cadenceWritePending) {
    g.cadenceWritePending = false;
    uint8_t len = g.cadenceWriteLen;
    uint8_t mode = g.cadenceWriteBuf[0];
    bool ok = (len == 1 || len == 5) && mode <= 3;
    cadence::Cadence c;
    if (ok) {
      c.mode = static_cast<cadence::Mode>(mode);
      if (c.mode == cadence::Mode::Custom) {
        ok = (len == 5);
        if (ok) {
          memcpy(&c.customSeconds, g.cadenceWriteBuf + 1, 4);
          ok = c.customSeconds >= cadence::kMinCustomSeconds &&
               c.customSeconds <= cadence::kMaxCustomSeconds;
        }
      } else {
        // custom_seconds is meaningful only for mode 3 — keep the stored
        // value so a (mode, junk) 5-byte write cannot corrupt it.
        portENTER_CRITICAL(&g.persistedMux);
        memcpy(&c.customSeconds, g.persistedValue + 1, 4);
        portEXIT_CRITICAL(&g.persistedMux);
      }
    }
    if (ok) {
      res.cadenceChanged = true;
      res.newCadence = c;
    } else {
      // Put the persisted setting back into the characteristic — the GATT
      // stack already stored the rejected bytes there (see CadenceCb).
      uint8_t v[5];
      portENTER_CRITICAL(&g.persistedMux);
      memcpy(v, g.persistedValue, sizeof(v));
      portEXIT_CRITICAL(&g.persistedMux);
      g.cadenceChar->setValue(v, sizeof(v));
      Serial.println("BLE cadence write rejected");
    }
  }

  if (g.historyCmdPending != 0) {
    uint8_t cmd = g.historyCmdPending;
    g.historyCmdPending = 0;
    if (cmd == 1 && g.connected) {
      if (g.mtu < kMinMtuForDump) {
        Serial.println("BLE history dump refused: MTU too small");
        sendEndFrame(0, kStatusMtuTooSmall);
      } else {
        g.dumpActive = true;
        g.dumpIdx = 0;
        g.dumpCount = g.buf ? g.buf->count() : 0;
        Serial.print("BLE history dump n=");
        Serial.println(g.dumpCount);
      }
    } else {
      g.dumpActive = false;
    }
  }

  if (g.dumpActive) {
    uint8_t sentThisPoll = 0;
    while (g.dumpActive && sentThisPoll < kFramesPerPoll) {
      if (!g.connected) {
        g.dumpActive = false;
        break;
      }
      if (g.dumpIdx >= g.dumpCount) {
        g.dumpActive = false;
        sendEndFrame(g.dumpCount, kStatusOk);
        Serial.print("BLE history done sent=");
        Serial.println(g.dumpCount);
        break;
      }
      uint8_t frame[2 + sizeof(history::Record)];
      uint16_t seq = g.dumpIdx;
      memcpy(frame, &seq, 2);
      const history::Record& rec = g.buf->at(g.dumpIdx);
      memcpy(frame + 2, &rec, sizeof(rec));
      g.historyChar->setValue(frame, sizeof(frame));
      g.historyChar->notify();
      ++g.dumpIdx;
      ++sentThisPoll;
    }
  }

  return res;
}
