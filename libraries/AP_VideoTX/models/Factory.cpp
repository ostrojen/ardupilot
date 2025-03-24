#include "AkkAlpha5W.cpp"
#include "AkkUltraLongRange3WLX.cpp"
#include <AP_Param/AP_Param.h>

class Factory {
public:
  static VTX_Model* by_model_id_param(AP_Int8 model_id)
  {
    switch(model_id.get()) {
      case 2: return new AkkUltraLongRange3WLX();
      case 3:
      default: return new AkkAlpha5W();
    }
  }
};