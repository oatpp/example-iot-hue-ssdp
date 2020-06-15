#ifndef HueDeviceDto_hpp
#define HueDeviceDto_hpp

#include "oatpp/core/macro/codegen.hpp"
#include "oatpp/core/Types.hpp"

#include OATPP_CODEGEN_BEGIN(DTO)

/*
 * DTOs to replicate the JSON's send and received for philips hue lights
 */

class HueDeviceCapabilitiesDto : public oatpp::DTO {

  DTO_INIT(HueDeviceCapabilitiesDto, DTO);

  // Fixed values
  DTO_FIELD(Boolean, certified) = false;
  DTO_FIELD(Fields<Boolean>, streaming) = {
      {"renderer", true},
      {"proxy", false}
  };
};

class HueDeviceStateDto : public oatpp::DTO {
  DTO_INIT(HueDeviceStateDto, DTO);

  // User values
  DTO_FIELD(Boolean, on);
  DTO_FIELD(UInt8, bri);
  DTO_FIELD(UInt8, sat);
  DTO_FIELD(UInt16, hue);
  DTO_FIELD(UInt16, ct); // white color temperature, 154 (cold) - 500 (warm)

  // Fixed values
  DTO_FIELD(List<Int32>, xy) = {0,0};
  DTO_FIELD(Boolean, reachable) = true;
};

class HueDeviceDto : public oatpp::DTO {
  
  DTO_INIT(HueDeviceDto, DTO);

  // User values
  DTO_FIELD(String, name);
  DTO_FIELD(oatpp::Object<HueDeviceStateDto>, state) = HueDeviceStateDto::createShared();
  DTO_FIELD(String, uniqueid); // name-number

  // Fixed values
  DTO_FIELD(String, type) = "Extended Color Light";
  DTO_FIELD(String, modelid) = "LCT007";
  DTO_FIELD(String, swversion) = "5.105.0.21169";
  DTO_FIELD(oatpp::Object<HueDeviceCapabilitiesDto>, capabilities) = HueDeviceCapabilitiesDto::createShared();
  
};

#include OATPP_CODEGEN_END(DTO)

#endif /* HueDeviceDto_hpp */
