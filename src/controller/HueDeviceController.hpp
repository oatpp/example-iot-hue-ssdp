
#ifndef HueDeviceController_hpp
#define HueDeviceController_hpp

#include <DeviceDescriptorComponent.hpp>
#include "../db/Database.hpp"

#include "../dto/UserRegisterDto.hpp"
#include "../dto/GenericResponseDto.hpp"

#include "oatpp/web/server/api/ApiController.hpp"
#include "oatpp/parser/json/mapping/ObjectMapper.hpp"
#include "oatpp/core/macro/codegen.hpp"
#include "oatpp/core/macro/component.hpp"

/**
 *  EXAMPLE ApiController
 *  Basic examples of howto create ENDPOINTs
 *  More details on oatpp.io
 */
class HueDeviceController : public oatpp::web::server::api::ApiController {
public:
  HueDeviceController(const std::shared_ptr<ObjectMapper>& objectMapper)
    : oatpp::web::server::api::ApiController(objectMapper)
  {
  }
private:
  
  /**
   *  Inject Database component
   */
  OATPP_COMPONENT(std::shared_ptr<Database>, m_database);
  OATPP_COMPONENT(std::shared_ptr<DeviceDescriptorComponent::DeviceDescriptor>, m_desc);
public:
  
  /**
   *  Inject @objectMapper component here as default parameter
   *  Do not return bare Controllable* object! use shared_ptr!
   */
  static std::shared_ptr<HueDeviceController> createShared(OATPP_COMPONENT(std::shared_ptr<ObjectMapper>,
                                                                           objectMapper)){
    return std::make_shared<HueDeviceController>(objectMapper);
  }

  void gen_random(char *s, const int len) {
    static const char alphanum[] =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";

    for (int i = 0; i < len; ++i) {
      s[i] = alphanum[rand() % (sizeof(alphanum) - 1)];
    }

    s[len] = 0;
  }

  std::shared_ptr<OutgoingResponse> addHueHeaders(std::shared_ptr<OutgoingResponse> rsp) {
    //rsp->putHeader("Server", "FreeRTOS/6.0.5, UPnP/1.0, IpBridge/1.17.0");
    rsp->putHeader("Connection", "close");
    return rsp;
  }

  /**
   *  Begin ENDPOINTs generation ('ApiController' codegen)
   */
#include OATPP_CODEGEN_BEGIN(ApiController)

  ENDPOINT("GET", "/description.xml", description) {
    OATPP_LOGD("HueDeviceController", "Request for description");
    oatpp::String xml =
        "<?xml version=\"1.0\"?>"
        "<root xmlns=\"urn:schemas-upnp-org:device-1-0\">"
          "<specVersion>"
            "<major>1</major>"
            "<minor>0</minor>"
          "</specVersion>"
          "<URLBase>http://" + m_desc->ipPort + "/</URLBase>"
          "<device>"
            "<deviceType>urn:schemas-upnp-org:device:Basic:1</deviceType>"
            "<friendlyName>Philips hue (" + m_desc->ipPort + ")</friendlyName>"
            "<manufacturer>Royal Philips Electronics</manufacturer>"
            "<manufacturerURL>http://www.philips.com</manufacturerURL>"
            "<modelDescription>Philips hue Personal Wireless Lighting</modelDescription>"
            "<modelName>Philips hue bridge 2012</modelName>"
            "<modelNumber>" + m_desc->sn + "</modelNumber>"
            "<modelURL>http://www.meethue.com</modelURL>"
            "<serialNumber>" + m_desc->mac + "</serialNumber>"
            "<UDN>uuid:" + m_desc->uuid + "</UDN>"
            "<presentationURL>index.html</presentationURL>"
          "</device>"
        "</root>";

    return addHueHeaders(createResponse(Status::CODE_200, xml));
  }

  ENDPOINT_INFO(appRegister) {
    info->description = "Handles the HUE-User Registration";
    info->addConsumes<oatpp::Object<UserRegisterDto>>("application/json");
    info->addResponse<oatpp::Object<ResponseTypeDto>>(Status::CODE_200, "application/json");
  }
  ENDPOINT("POST", "/api", appRegister, BODY_DTO(oatpp::Object<UserRegisterDto>, userRegister)) {
    if (userRegister->username == nullptr) {
      userRegister->username = "OatppSsdpHueDefaultUser_________________";
      gen_random((char*)userRegister->username->getData()+23, 17);
      OATPP_LOGD("HueDeviceController", "POST on /api with empty user, generated '%s'",userRegister->username->getData());
    } else {
      OATPP_LOGD("HueDeviceController", "POST on /api for user '%s'", userRegister->username->c_str());
    }
    OATPP_LOGD("HueDeviceController", "Devicetype: %s", userRegister->devicetype->c_str());
    auto responseDto = GenericResponseDto::createShared();
    responseDto->push_back(oatpp::Object<ResponseTypeDto>::createShared());
    responseDto->front()->success = {{"username", userRegister->username}};

    auto response = createDtoResponse(Status::CODE_200, responseDto);
    return addHueHeaders(response);
  }

  ENDPOINT("GET", "/api/{username}/lights", getLights, PATH(String, username)) {
    OATPP_LOGD("HueDeviceController", "GET on /api/{username}/lights");
    // list all
    auto devices = m_database->getHueDevices();
    auto response = Fields<oatpp::Object<HueDeviceDto>>::createShared();
    for (unsigned long d = 0; d < devices->size(); ++d) {
      char num[32];
      memset(num, 0, 18);
      snprintf(num, 18, "%lu", d+1);
      response[num] = devices[d];
    }
    return addHueHeaders(createDtoResponse(Status::CODE_200, response));
  }

  ENDPOINT("GET", "/api/{username}/lights/{hueId}", getLight, PATH(String, username), PATH(Int32, hueId)) {
    OATPP_LOGD("HueDeviceController", "GET on /api/%s/lights/%d", username->c_str(), *hueId.get());
    // list all
    if (hueId == 0) {
      return getLights(username);
    }
    // list specific
    auto specific = m_database->getHueDeviceById(hueId - 1);
    if (specific == nullptr) {
      char num[32];
      auto responseDto = GenericResponseDto::createShared();
      responseDto->push_back(oatpp::Object<ResponseTypeDto>::createShared());
      memset(num, 0, 32);
      snprintf(num, 32, "/lights/%d", *hueId.get());
      responseDto->back()->error = {{oatpp::String(num), oatpp::String("Not Found")}};
      auto response = createDtoResponse(Status::CODE_404, responseDto);
    }
    return addHueHeaders(createDtoResponse(Status::CODE_200, specific));
  }

  ENDPOINT("PUT", "/api/{username}/lights/{hueId}/state", updateState,
      PATH(String, username),
      PATH(Int32, hueId),
      BODY_DTO(Object<HueDeviceStateDto>, state)) {
    OATPP_LOGD("HueDeviceController", "PUT on /api/%s/lights/%d/state", username->c_str(), *hueId.get());
    v_int32 id = hueId;
    id--;
    char num[32];
    auto responseDto = GenericResponseDto::createShared();
    auto updated = m_database->updateHueDeviceState(id, state);

    if (updated == nullptr) {
      responseDto->push_back(oatpp::Object<ResponseTypeDto>::createShared());
      memset(num, 0, 32);
      snprintf(num, 32, "/lights/%d/state/on", *hueId.get());
      responseDto->back()->error = {{oatpp::String(num), state->on}};
      auto response = createDtoResponse(Status::CODE_200, responseDto);
    }

    responseDto->push_back(oatpp::Object<ResponseTypeDto>::createShared());
    memset(num, 0, 32);
    snprintf(num, 32, "/lights/%d/state/on", *hueId.get());
    responseDto->back()->success = {{oatpp::String(num), updated->state->on}};

    responseDto->push_back(oatpp::Object<ResponseTypeDto>::createShared());
    memset(num, 0, 32);
    snprintf(num, 32, "/lights/%d/state/bri", *hueId.get());
    responseDto->back()->success = {{"username", updated->state->bri}};

    auto response = createDtoResponse(Status::CODE_200, responseDto);
    return addHueHeaders(response);
  }

  /**
   *  Finish ENDPOINTs generation ('ApiController' codegen)
   */
#include OATPP_CODEGEN_END(ApiController)
  
};

#endif /* HueDeviceController_hpp */
