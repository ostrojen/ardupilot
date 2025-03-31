#pragma once

#include <AP_VideoTX/models/VTX_Model.h>

class Alpha8W : public VTX_Model {
public:
    static PowerLevel power_levels[VTX_MAX_POWER_LEVELS];
    static const char* band_names[VTX_MODEL_BANDS];
    static const uint16_t VIDEO_CHANNELS[VTX_MODEL_BANDS][VTX_MODEL_CHANNELS];
    static const uint16_t max_power;

    const char* getName() const override {
      return "AKK Alpha 8W";
    }

    PowerLevel (&getPowerLevels() override)[VTX_MAX_POWER_LEVELS] {
        return power_levels;
    }

    uint16_t getMaxPower() const override {
        return max_power;
    }

    const char* (&getBandNames() const override)[VTX_MODEL_BANDS] {
        return band_names;
    }

    const uint16_t (&getVideoChannels() const override)[VTX_MODEL_BANDS][VTX_MODEL_CHANNELS] {
        return VIDEO_CHANNELS;
    }
};