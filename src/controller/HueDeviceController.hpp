
#ifndef HueDeviceController_hpp
#define HueDeviceController_hpp

#include "DeviceDescriptorComponent.hpp"

#include "db/Database.hpp"

#include "dto/UserRegisterDto.hpp"
#include "dto/GenericResponseDto.hpp"

#include "oatpp/web/server/api/ApiController.hpp"
#include "oatpp/parser/json/mapping/ObjectMapper.hpp"
#include "oatpp/core/macro/codegen.hpp"
#include "oatpp/core/macro/component.hpp"


#include OATPP_CODEGEN_BEGIN(ApiController) //< Begin codegen section

/**
 *  EXAMPLE ApiController
 *  Basic examples of howto create ENDPOINTs
 *  More details on oatpp.io
 */
class HueDeviceController : public oatpp::web::server::api::ApiController {
public:
  HueDeviceController(const std::shared_ptr<ObjectMapper>& objectMapper)
    : oatpp::web::server::api::ApiController(objectMapper)
  {}
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

  ENDPOINT_INFO(description) {
    info->description = "Answers with a correct XML-Description for this hue-hub implementation";
  }
  ENDPOINT("GET", "/description.xml", description) {

    OATPP_LOGD("HueDeviceController", "Request for description");

    oatpp::data::stream::BufferOutputStream ss;
    ss <<
      "<?xml version=\"1.0\"?>\n"
      "<root xmlns=\"urn:schemas-upnp-org:device-1-0\">\n"
      "  <specVersion>\n"
      "    <major>1</major>\n"
      "    <minor>0</minor>\n"
      "  </specVersion>\n"
      "  <URLBase>http://" << m_desc->ipPort << "/</URLBase>\n"
      "  <device>\n"
      "    <deviceType>urn:schemas-upnp-org:device:Basic:1</deviceType>\n"
      "    <friendlyName>Philips hue (" << m_desc->ipPort << ")</friendlyName>\n"
      "    <manufacturer>Royal Philips Electronics</manufacturer>\n"
      "    <manufacturerURL>http://www.philips.com</manufacturerURL>\n"
      "    <modelDescription>Philips hue Personal Wireless Lighting</modelDescription>\n"
      "    <modelName>Philips hue bridge 2012</modelName>\n"
      "    <modelNumber>" << m_desc->sn << "</modelNumber>\n"
      "    <modelURL>http://www.meethue.com</modelURL>\n"
      "    <serialNumber>" << m_desc->mac << "</serialNumber>\n"
      "    <UDN>uuid:" + m_desc->uuid + "</UDN>\n"
      "    <presentationURL>index.html</presentationURL>\n"
      "    <serviceList>\n"
      "      <service>\n"
      "        <serviceType>(null)</serviceType>\n"
      "        <serviceId>(null)</serviceId>\n"
      "        <controlURL>(null)</controlURL>\n"
      "        <eventSubURL>(null)</eventSubURL>\n"
      "        <SCPDURL>(null)</SCPDURL>\n"
      "      </service>\n"
      "    </serviceList>\n"
      "  </device>\n"
      "</root>";

    return addHueHeaders(createResponse(Status::CODE_200, ss.toString()));
  }

  ENDPOINT_INFO(appRegister) {
    info->description = "Handles the Hue-User Registration. Creates a random username is none is provided in the UserRegisterDto.";
    info->addConsumes<oatpp::Object<UserRegisterDto>>("application/json");
    info->addResponse<oatpp::Object<ResponseTypeDto>>(Status::CODE_200, "application/json");
  }
  ENDPOINT("POST", "/api", appRegister,
           BODY_DTO(oatpp::Object<UserRegisterDto>, userRegister))
  {
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

  ENDPOINT_INFO(getLights) {
    info->description = "Lists all available 'lights' known to this 'hub'";
    info->addResponse<Fields<oatpp::Object<HueDeviceDto>>>(Status::CODE_200, "application/json");
  }
  ENDPOINT("GET", "/api/{username}/lights", getLights,
           PATH(String, username))
  {
    OATPP_LOGD("HueDeviceController", "GET on /api/{username}/lights");
    // list all
    auto devices = m_database->getHueDevices();
    auto response = Fields<oatpp::Object<HueDeviceDto>>::createShared();
    for (auto device = devices->begin(); device != devices->end(); device++) {
      char num[32];
      memset(num, 0, 18);
      snprintf(num, 18, "%u", *(device->first.get()) + 1);
      if (device->second->state->colormode == nullptr) {
        device->second->state->colormode = "ct";
        if (device->second->state->ct == nullptr) {
          device->second->state->ct = 500;
        }
      }
      response[num] = device->second;
    }
    return addHueHeaders(createDtoResponse(Status::CODE_200, response));
  }

  ENDPOINT_INFO(getLight) {
    info->description = "Returns the state of 'light' no. `hueId`.";
    info->addResponse<oatpp::Object<ResponseTypeDto>>(Status::CODE_200, "application/json");
  }
  ENDPOINT("GET", "/api/{username}/lights/{hueId}", getLight,
           PATH(String, username),
           PATH(Int32, hueId))
  {
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
    if (specific->state->colormode == nullptr) {
      specific->state->colormode = "ct";
      if (specific->state->ct == nullptr) {
        specific->state->ct = 500;
      }
    }
    return addHueHeaders(createDtoResponse(Status::CODE_200, specific));
  }

  ENDPOINT_INFO(updateState) {
    info->description = "Sets the state for 'light' no. `hueId`. This endpoint is called by devices (i.E. Alexa) to control a light";
    info->addConsumes<oatpp::Object<HueDeviceStateDto>>("application/json");
    info->addResponse<oatpp::Object<ResponseTypeDto>>(Status::CODE_200, "application/json");
  }
  ENDPOINT("PUT", "/api/{username}/lights/{hueId}/state", updateState,
           PATH(String, username),
           PATH(Int32, hueId),
           BODY_DTO(Object<HueDeviceStateDto>, state))
  {
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

    /*
     * ToDo: Implement your "light turning on/off" here!
     * Better: Replace the Database with your state and control logic so the "database" is in sync to your logic.
     */
    OATPP_LOGI("HueDeviceController", "updateState: Setting light %d %s", *hueId.get(), updated->state->on ? "on" : "off");

    responseDto->push_back(oatpp::Object<ResponseTypeDto>::createShared());
    if (state->on != nullptr) {
      memset(num, 0, 32);
      snprintf(num, 32, "/lights/%d/state/on", *hueId.get());
      if (responseDto->back()->success.get() == nullptr) {
        responseDto->back()->success = {{oatpp::String(num), updated->state->on}};
      } else {
        responseDto->back()->success->push_back({oatpp::String(num), updated->state->on});
      }
    }

    if (state->bri != nullptr) {
      memset(num, 0, 32);
      snprintf(num, 32, "/lights/%d/state/bri", *hueId.get());
      if (responseDto->back()->success.get() == nullptr) {
        responseDto->back()->success = {{oatpp::String(num), updated->state->bri}};
      } else {
        responseDto->back()->success->push_back({oatpp::String(num), updated->state->bri});
      }
    }

    if (state->hue != nullptr) {
      memset(num, 0, 32);
      snprintf(num, 32, "/lights/%d/state/hue", *hueId.get());
      if (responseDto->back()->success.get() == nullptr) {
        responseDto->back()->success = {{oatpp::String(num), updated->state->hue}};
      } else {
        responseDto->back()->success->push_back({oatpp::String(num), updated->state->hue});
      }
    }

    if (state->sat != nullptr) {
      memset(num, 0, 32);
      snprintf(num, 32, "/lights/%d/state/sat", *hueId.get());
      if (responseDto->back()->success.get() == nullptr) {
        responseDto->back()->success = {{oatpp::String(num), updated->state->sat}};
      } else {
        responseDto->back()->success->push_back({oatpp::String(num), updated->state->sat});
      }
    }

    if (state->ct != nullptr) {
      memset(num, 0, 32);
      snprintf(num, 32, "/lights/%d/state/ct", *hueId.get());
      if (responseDto->back()->success.get() == nullptr) {
        responseDto->back()->success = {{oatpp::String(num), updated->state->ct}};
      } else {
        responseDto->back()->success->push_back({oatpp::String(num), updated->state->ct});
      }
    }

    auto response = createDtoResponse(Status::CODE_200, responseDto);
    return addHueHeaders(response);
  }

};

#include OATPP_CODEGEN_END(ApiController) //< End of codegen section

#endif /* HueDeviceController_hpp */
