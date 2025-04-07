#include <AP_VideoTX/models/AKK/Alpha4W.cpp>
#include <AP_VideoTX/models/AKK/Alpha5W.cpp>
#include <AP_VideoTX/models/AKK/Alpha8W.cpp>
#include <AP_VideoTX/models/AKK/Alpha10W.cpp>
#include <AP_VideoTX/models/AKK/UltraLongRange3W.cpp>
#include <AP_VideoTX/models/AKK/UltraLongRange3WLX.cpp>
#include <AP_VideoTX/models/AKK/UltraLongRange3WAll.cpp>
#include <AP_VideoTX/models/RUSHFPV/MaxSolo.cpp>

class Factory {
public:
  static VTX_Model *by_model_id_param(uint8_t model_id)
  {
    switch(model_id) {
      case 0: return new MaxSolo();
      case 1: return new UltraLongRange3WAll();
      case 2: return new UltraLongRange3WLX();
      case 3: return new UltraLongRange3W();

      case 4: return new Alpha4W();
      case 5: return new Alpha5W();
      case 8: return new Alpha8W();
      case 10: return new Alpha10W();

      default: return default_model();
    }
  }

  static VTX_Model *default_model()
  {
    return new UltraLongRange3W();
  }
};