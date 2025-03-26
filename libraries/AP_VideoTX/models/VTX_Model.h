#pragma once
#include <cstdint>

#define VTX_MODEL_BANDS 10
#define VTX_MODEL_CHANNELS 8
#define VTX_MAX_POWER_LEVELS 5

enum class PowerActive {
    Unknown,
    Active,
    Inactive
};

struct PowerLevel {
    uint8_t level;
    uint16_t mw;
    uint8_t dbm;
    uint8_t dac; // SmartAudio v1 dac value
    PowerActive active;
};

class VTX_Model {
public:
    virtual ~VTX_Model() = default;

    virtual const char* getName() const;
    virtual PowerLevel (&getPowerLevels())[VTX_MAX_POWER_LEVELS];
    virtual uint16_t getMaxPower() const;
    virtual const char* (&getBandNames() const)[VTX_MODEL_BANDS];
    virtual const uint16_t (&getVideoChannels() const)[VTX_MODEL_BANDS][VTX_MODEL_CHANNELS];
};