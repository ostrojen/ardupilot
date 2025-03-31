#include "UltraLongRange3WAll.h"

//TODO dac and dbm values
PowerLevel UltraLongRange3WAll::power_levels[VTX_MAX_POWER_LEVELS] = {
//  { 0,    25,   7,  7    },
    { 1,    250,  14, 7    },
    { 2,    500,  27, 25   },
    { 3,    1000, 27, 25   },
    { 4,    2000, 27, 25   },
    { 5,    3000, 34, 40   },
};

const char* UltraLongRange3WAll::band_names[VTX_MODEL_BANDS] =  {"A","b","E","F","r","P","L","U","O","X"};

const uint16_t UltraLongRange3WAll::VIDEO_CHANNELS[VTX_MODEL_BANDS][VTX_MODEL_CHANNELS] = {
    /* Band A */{ 5474, 5492, 5510, 5528, 5546, 5564, 5582, 5600},
    /* Band b */{ 4990, 5020, 5050, 5080, 5110, 5140, 5170, 5200},
    /* Band E */{ 5705, 5685, 5665, 5645, 5885, 5905, 5925, 5945},
    /* Band F */{ 5740, 5760, 5780, 5800, 5820, 5840, 5860, 5880},
    /* Band r */{ 5658, 5695, 5732, 5769, 5806, 5843, 5880, 5917},
    /* Band P */{ 5653, 5693, 5733, 5773, 5813, 5853, 5893, 5933},
    /* Band L */{ 5333, 5373, 5413, 5453, 5493, 5533, 5573, 5613},
    /* Band U */{ 5325, 5348, 5366, 5384, 5402, 5420, 5438, 5456},
    /* Band O */{ 5865, 5845, 5825, 5805, 5785, 5765, 5745, 5725},
    /* Band X */{ 5733, 5752, 5771, 5790, 5809, 5828, 5847, 5866},
};

const uint16_t UltraLongRange3WAll::max_power = 3000;
