
#ifndef Database_hpp
#define Database_hpp

#include "dto/HueDeviceDto.hpp"
#include "db/model/HueDevice.hpp"

#include "oatpp/core/concurrency/SpinLock.hpp"
#include <unordered_map>

/**
 *  Trivial in-memory Database based on unordered_map container.
 *  For demo purposes only :)
 */
class Database {
private:
  oatpp::concurrency::SpinLock m_lock;
  v_int32 m_idCounter; ///< counter to generate HueDeviceIds
  std::unordered_map<v_int32, HueDevice> m_HueDevicesById; ///< Map HueDeviceId to HueDevice
private:
  HueDevice serializeFromDto(const oatpp::Object<HueDeviceDto>& hueDeviceDto);
  HueDevice updateFromStateDto(v_int32 id, const oatpp::Object<HueDeviceStateDto> &hueDeviceStateDto);
  oatpp::Object<HueDeviceDto> deserializeToDto(const HueDevice& hueDevice);
public:
  
  Database()
    : m_idCounter(0)
  {}

  v_int32 registerHueDevice(const oatpp::String &name, const oatpp::Boolean &on = false, const oatpp::Int32 &bri = 0);
  oatpp::Object<HueDeviceDto> createHueDevice(const oatpp::Object<HueDeviceDto>& hueDeviceDto);
  oatpp::Object<HueDeviceDto> updateHueDevice(const oatpp::Object<HueDeviceDto>& hueDeviceDto);
  oatpp::Object<HueDeviceDto> updateHueDeviceState(v_int32 id, const oatpp::Object<HueDeviceStateDto>& hueDeviceStateDto);
  oatpp::Object<HueDeviceDto> getHueDeviceById(v_int32 id);
  oatpp::List<oatpp::Object<HueDeviceDto>> getHueDevices();
  bool deleteHueDevice(v_int32 id);
  
};

#endif /* Database_hpp */
