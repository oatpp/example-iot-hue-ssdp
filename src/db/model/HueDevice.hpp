
#ifndef db_HueDevice_hpp
#define db_HueDevice_hpp

#include "oatpp/core/Types.hpp"

/**
 *  Object of HueDevice stored in the Demo-Database.
 */
class HueDevice {
public:
  v_int32 id;
  oatpp::String name;
  oatpp::Boolean on = false;
  oatpp::UInt8 bri = (v_uint8)0;
  oatpp::UInt8 sat = (v_uint8)0;
  oatpp::UInt16 hue = (v_uint16)0;
  oatpp::UInt16 ct = (v_uint16)500;
};

#endif /* db_HueDevice_hpp */
