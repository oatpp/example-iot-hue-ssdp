
#ifndef SsdpController_hpp
#define SsdpController_hpp

#include "DeviceDescriptorComponent.hpp"
#include "db/Database.hpp"

#include "dto/UserRegisterDto.hpp"
#include "dto/GenericResponseDto.hpp"

#include "oatpp/web/server/api/ApiController.hpp"
#include "oatpp/parser/json/mapping/ObjectMapper.hpp"
#include "oatpp/core/macro/codegen.hpp"
#include "oatpp/core/macro/component.hpp"

/**
 *  EXAMPLE ApiController
 *  Basic examples of howto create ENDPOINTs
 *  More details on oatpp.io
 */
class SsdpController : public oatpp::web::server::api::ApiController {
 public:
  SsdpController(const std::shared_ptr<ObjectMapper>& objectMapper)
      : oatpp::web::server::api::ApiController(objectMapper)
  {}
 private:

  /**
   *  Inject Database component
   */
  OATPP_COMPONENT(std::shared_ptr<Database>, m_database);

  /**
   * Inject DeviceDescriptor component to easily syncronize all device specific data
   */
  OATPP_COMPONENT(std::shared_ptr<DeviceDescriptorComponent::DeviceDescriptor>, m_desc);
 public:

  /**
   *  Inject @objectMapper component here as default parameter
   *  Do not return bare Controllable* object! use shared_ptr!
   */
  static std::shared_ptr<SsdpController> createShared(OATPP_COMPONENT(std::shared_ptr<ObjectMapper>,
                                                                           objectMapper)){
    return std::make_shared<SsdpController>(objectMapper);
  }

#include OATPP_CODEGEN_BEGIN(ApiController)

  /**
   *  Other devices that want to discover you send 'M-SEARCH *' SSDP packages.
   *  You have to answer with a corresponding packet on this discovery.
   *  Here we answer with a Packet that mimics a Philips hue hub.
   */
  ENDPOINT("M-SEARCH", "*", star) {
    OATPP_LOGD("SsdpController", "'M-SEARCH *' Received");
    auto rsp = createResponse(Status::CODE_200, oatpp::String(""));
    rsp->putHeader("CACHE-CONTROL", "max-age=100");
    rsp->putHeader("EXT", "");
    rsp->putHeader("LOCATION", "http://" + m_desc->ipPort + "/description.xml");
    rsp->putHeader("SERVER", "FreeRTOS/6.0.5, UPnP/1.0, IpBridge/1.17.0");
    rsp->putHeader("ST", "urn:schemas-upnp-org:device:basic:1");
    rsp->putHeader("USN", "uuid:" + m_desc->uuid + "::upnp:rootdevice");
    return rsp;
  }

#include OATPP_CODEGEN_END(ApiController)

};

#endif /* SsdpController_hpp */
