#include "MaxSolo.h"

PowerLevel MaxSolo::power_levels[VTX_MAX_POWER_LEVELS] = {
    { 0,    25,  14, 7    },
    { 1,    500,  27, 22   },
    { 2,    1000,  30, 25   },
    { 3,    2500,  33, 32   },
};

const char* MaxSolo::band_names[VTX_MODEL_BANDS] =  {"A","B","E","F","R","L"};

const uint16_t MaxSolo::VIDEO_CHANNELS[VTX_MODEL_BANDS][VTX_MODEL_CHANNELS] = {
    /* Band A */{ 5865, 5845, 5825, 5805, 5785, 5765, 5745, 5725},
    /* Band B */{ 5733, 5752, 5771, 5790, 5809, 5828, 5847, 5866},
    /* Band E */{ 5705, 5685, 5665, 5645, 5885, 5905, 5925, 5945},
    /* Band F */{ 5740, 5760, 5780, 5800, 5820, 5840, 5860, 5880},
    /* Band R */{ 5658, 5695, 5732, 5769, 5806, 5843, 5880, 5917},
    /* Band L */{ 5658, 5695, 5732, 5769, 5806, 5843, 5880, 5917},
};

const uint16_t MaxSolo::max_power = 2500;
