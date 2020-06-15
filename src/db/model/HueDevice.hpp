
#ifndef db_User_hpp
#define db_User_hpp

#include "oatpp/core/Types.hpp"

/**
 *  Object of HueDevice stored in the Demo-Database.
 */
class HueDevice {
public:
  v_int32 id;
  oatpp::String name;
  oatpp::Boolean on = false;
  oatpp::UInt8 bri = (uint8_t)0;
  oatpp::UInt8 sat = (uint8_t)0;
  oatpp::UInt16 hue = (uint8_t)0;
  oatpp::UInt16 ct = (uint16_t)500;
};

#endif /* db_User_hpp */
