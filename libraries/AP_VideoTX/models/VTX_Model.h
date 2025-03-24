#pragma once
#include <cstdint>

#define VTX_MODEL_BANDS 10
#define VTX_MODEL_CHANNELS 8

class VTX_Model {
public:
    virtual ~VTX_Model() = default;

    static const uint16_t VIDEO_CHANNELS[VTX_MODEL_BANDS][VTX_MODEL_CHANNELS];
    static const char* band_names[VTX_MODEL_BANDS];
    static const uint16_t max_power;

    virtual const char* (&getBandNames() const)[VTX_MODEL_BANDS] = 0;
    virtual const uint16_t (&getVideoChannels() const)[VTX_MODEL_BANDS][VTX_MODEL_CHANNELS] = 0;
};