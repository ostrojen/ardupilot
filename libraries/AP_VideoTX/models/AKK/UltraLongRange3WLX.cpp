#include "UltraLongRange3WLX.h"

//TODO dac and dbm values
PowerLevel UltraLongRange3WLX::power_levels[VTX_MAX_POWER_LEVELS] = {
//  { 0,    25,   7,  7    },
    { 1,    250,  14, 7    },
    { 2,    500,  27, 25   },
    { 3,    1000, 27, 25   },
    { 4,    2000, 27, 25   },
    { 5,    3000, 34, 40   },
};

const char* UltraLongRange3WLX::band_names[VTX_MODEL_BANDS] = {"A","B","E","F","L","X"};

const uint16_t UltraLongRange3WLX::VIDEO_CHANNELS[VTX_MODEL_BANDS][VTX_MODEL_CHANNELS] = {
    /* Band A */{ 5865, 5845, 5825, 5805, 5785, 5765, 5745, 5725},
    /* Band b */{ 5733, 5752, 5771, 5790, 5809, 5828, 5847, 5866},
    /* Band E */{ 5705, 5685, 5665, 5645, 5885, 5905, 5925, 5945},
    /* Band F */{ 5740, 5760, 5780, 5800, 5820, 5840, 5860, 5880},
    /* Band L */{ 5362, 5399, 5436, 5473, 5510, 5547, 5584, 5621},
    /* Band X   { 4990, 5020, 5050, 5080, 5110, 5140, 5170, 5200}, doesn't work via smartaudio */
};

const uint16_t UltraLongRange3WLX::max_power = 3000;
