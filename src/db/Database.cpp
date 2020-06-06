
#include "Database.hpp"
#include "oatpp/core/parser/Caret.hpp"

HueDevice Database::updateFromStateDto(v_int32 id, const oatpp::Object<HueDeviceStateDto> &hueDeviceStateDto) {
  std::lock_guard<oatpp::concurrency::SpinLock> lock(m_lock);

  auto it = m_HueDevicesById.find(id);
  if(it == m_HueDevicesById.end()){
    throw std::runtime_error("Unable to find HueDevice with ID");
  }

  if (hueDeviceStateDto->bri != nullptr) {
    it->second.bri = hueDeviceStateDto->bri;
  }

  if (hueDeviceStateDto->on != nullptr) {
    it->second.on = hueDeviceStateDto->on;
  }

  if (hueDeviceStateDto->hue != nullptr) {
    it->second.hue = hueDeviceStateDto->hue;
  }

  if (hueDeviceStateDto->sat != nullptr) {
    it->second.sat = hueDeviceStateDto->sat;
  }

  if (hueDeviceStateDto->ct != nullptr) {
    it->second.ct = hueDeviceStateDto->ct;
  }

  return it->second;
}

HueDevice Database::serializeFromDto(const oatpp::Object<HueDeviceDto>& hueDeviceDto){
  HueDevice hueDevice = HueDevice();
  if (hueDeviceDto->state) {
    if (hueDeviceDto->state->on != nullptr)
      hueDevice.on = hueDeviceDto->state->on;
    if (hueDeviceDto->state->bri != nullptr)
      hueDevice.bri = hueDeviceDto->state->bri;
    if (hueDeviceDto->state->hue != nullptr)
      hueDevice.hue = hueDeviceDto->state->hue;
    if (hueDeviceDto->state->sat != nullptr)
      hueDevice.sat = hueDeviceDto->state->sat;
    if (hueDeviceDto->state->ct != nullptr)
      hueDevice.ct = hueDeviceDto->state->ct;
  }
  if(hueDeviceDto->uniqueid){
    oatpp::parser::Caret caret(hueDeviceDto->uniqueid);
    if (!caret.findChar('-'))
      throw std::runtime_error("Malformed uniqueid: '-' not found");
    caret.inc();
    v_int32 id = caret.parseInt();
    if (caret.hasError())
      throw std::runtime_error("Malformed uniqueid: Unable to parse id integer");
    hueDevice.id = id;
  }
  return hueDevice;
}

oatpp::Object<HueDeviceDto> Database::deserializeToDto(const HueDevice& hueDevice){
  auto dto = HueDeviceDto::createShared();
  dto->uniqueid = oatpp::String("BE570A70CAFE") + "-" + oatpp::String(hueDevice.id + 1); // BEST OAT CAFE!
  dto->name = hueDevice.name;
  dto->state->bri = hueDevice.bri;
  dto->state->on = hueDevice.on;
  dto->state->ct = hueDevice.ct;
  dto->state->hue = hueDevice.hue;
  dto->state->sat = hueDevice.sat;
  return dto;
}

oatpp::Object<HueDeviceDto> Database::createHueDevice(const oatpp::Object<HueDeviceDto>& hueDeviceDto){
  std::lock_guard<oatpp::concurrency::SpinLock> lock(m_lock);
  auto hueDevice = serializeFromDto(hueDeviceDto);
  hueDevice.id = m_idCounter++;
  m_HueDevicesById[hueDevice.id] = hueDevice;
  return deserializeToDto(hueDevice);
}

oatpp::Object<HueDeviceDto> Database::updateHueDeviceState(v_int32 id,
                                                           const oatpp::Object<HueDeviceStateDto> &hueDeviceStateDto) {
  return deserializeToDto(updateFromStateDto(id, hueDeviceStateDto));
}

oatpp::Object<HueDeviceDto> Database::updateHueDevice(const oatpp::Object<HueDeviceDto>& hueDeviceDto){
  std::lock_guard<oatpp::concurrency::SpinLock> lock(m_lock);
  auto hueDevice = serializeFromDto(hueDeviceDto);
  if(hueDevice.id < 0){
    throw std::runtime_error("HueDevice Id cannot be less than 0");
  }
  auto it = m_HueDevicesById.find(hueDevice.id);
  if(it != m_HueDevicesById.end()) {
    m_HueDevicesById[hueDevice.id] = hueDevice;
  } else {
    throw new std::runtime_error("Such HueDevice not found");
  }
  return deserializeToDto(hueDevice);
}

oatpp::Object<HueDeviceDto> Database::getHueDeviceById(v_int32 id) {
  std::lock_guard<oatpp::concurrency::SpinLock> lock(m_lock);
  auto it = m_HueDevicesById.find(id);
  if(it == m_HueDevicesById.end()){
    return nullptr;
  }
  return deserializeToDto(it->second);
}

oatpp::List<oatpp::Object<HueDeviceDto>> Database::getHueDevices(){
  std::lock_guard<oatpp::concurrency::SpinLock> lock(m_lock);
  oatpp::List<oatpp::Object<HueDeviceDto>> result({});
  auto it = m_HueDevicesById.begin();
  while (it != m_HueDevicesById.end()) {
    result->push_back(deserializeToDto(it->second));
    it++;
  }
  return result;
}

bool Database::deleteHueDevice(v_int32 id){
  std::lock_guard<oatpp::concurrency::SpinLock> lock(m_lock);
  auto it = m_HueDevicesById.find(id);
  if(it == m_HueDevicesById.end()){
    return false;
  }
  m_HueDevicesById.erase(it);
  return true;
}

v_int32 Database::registerHueDevice(const oatpp::String &name, const oatpp::Boolean &on, const oatpp::Int32 &bri) {
  std::lock_guard<oatpp::concurrency::SpinLock> lock(m_lock);
  HueDevice hueDevice;
  hueDevice.name = name;
  hueDevice.on = on;
  hueDevice.bri = bri;
  hueDevice.id = m_idCounter++;
  m_HueDevicesById[hueDevice.id] = hueDevice;
  return hueDevice.id;
}

