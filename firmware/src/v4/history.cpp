#include "history.h"

#include "ble_protocol.h"

namespace history {

void Buffer::push(const Readings& r, uint32_t tMs) {
  taskENTER_CRITICAL(&mux_);
  Record& rec = records_[head_];
  rec.tSeconds = tMs / 1000;
  rec.co2Ppm = (r.scd41Valid && !isnan(r.co2Ppm))
                   ? bleproto::clampU16(r.co2Ppm)
                   : bleproto::kInvalidU16;
  rec.temp100 = (r.scd41Valid && !isnan(r.tempC)) ? bleproto::packTemp(r)
                                                  : bleproto::kInvalidTemp;
  rec.rh100 = (r.scd41Valid && !isnan(r.rhPct))
                  ? bleproto::clampU16(r.rhPct * 100.0f)
                  : bleproto::kInvalidU16;
  rec.vocIndex = bleproto::packVoc(r);
  rec.noxIndex = bleproto::packNox(r);
  rec.pm1x10 = bleproto::packPmX10(r.pm1_0, r.sen50Valid);
  rec.pm25x10 = bleproto::packPmX10(r.pm2_5, r.sen50Valid);
  rec.pm10x10 = bleproto::packPmX10(r.pm10, r.sen50Valid);
  rec.pressPax10 = bleproto::packPressurePax10(r);

  head_ = static_cast<uint16_t>((head_ + 1) % kCapacity);
  if (count_ < kCapacity) ++count_;
  taskEXIT_CRITICAL(&mux_);
}

const Record& Buffer::at(uint16_t idx) const {
  // Oldest record sits at head_ once the ring has wrapped, else at 0.
  uint16_t oldest = (count_ == kCapacity) ? head_ : 0;
  return records_[(oldest + idx) % kCapacity];
}

void Buffer::snapshot(uint16_t& count, uint32_t& oldestT, uint32_t& newestT) {
  taskENTER_CRITICAL(&mux_);
  count = count_;
  oldestT = count ? at(0).tSeconds : 0;
  newestT = count ? at(count - 1).tSeconds : 0;
  taskEXIT_CRITICAL(&mux_);
}

}  // namespace history
