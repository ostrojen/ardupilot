#include "UltraLongRange3W.h"

//TODO dac and dbm values
PowerLevel UltraLongRange3W::power_levels[VTX_MAX_POWER_LEVELS] = {
    { 1,    250,  14, 7    },
    { 2,    500,  27, 25   },
    { 3,    1000, 27, 25   },
    { 4,    2000, 27, 25   },
    { 5,    3000, 34, 40   },
};

const char* UltraLongRange3W::band_names[VTX_MODEL_BANDS] = {"A","B","E","F","R"};

const uint16_t UltraLongRange3W::VIDEO_CHANNELS[VTX_MODEL_BANDS][VTX_MODEL_CHANNELS] = {
    /* Band A */{ 5865, 5845, 5825, 5805, 5785, 5765, 5745, 5725 },
    /* Band b */{ 5733, 5752, 5771, 5790, 5809, 5828, 5847, 5866 },
    /* Band E */{ 5705, 5685, 5665, 0,    5885, 5905, 0,    0    },
    /* Band F */{ 5740, 5760, 5780, 5800, 5820, 5840, 5860, 5880 },
    /* Band R */{ 5658, 5695, 5732, 5769, 5806, 5843, 5880, 5917 },
};

const uint16_t UltraLongRange3W::max_power = 3000;
