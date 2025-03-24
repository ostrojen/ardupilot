#pragma once
#include "VTX_Model.h"

class AkkAlpha5W : public VTX_Model {
public:
    static const char* band_names[VTX_MODEL_BANDS];
    static const uint16_t VIDEO_CHANNELS[VTX_MODEL_BANDS][VTX_MODEL_CHANNELS];
    static const uint16_t max_power;

    const char* (&getBandNames() const override)[VTX_MODEL_BANDS] {
        return band_names;
    }

    const uint16_t (&getVideoChannels() const override)[VTX_MODEL_BANDS][VTX_MODEL_CHANNELS] {
        return VIDEO_CHANNELS;
    }
};