
#ifndef AppComponent_hpp
#define AppComponent_hpp

#include <connection/SimpleUDPConnectionProvider.hpp>
#include "db/Database.hpp"

#include "SwaggerComponent.hpp"
#include "DeviceDescriptorComponent.hpp"

#include "oatpp/web/server/HttpConnectionHandler.hpp"
#include "oatpp/web/server/HttpRouter.hpp"
#include "oatpp/network/server/SimpleTCPConnectionProvider.hpp"

#include "oatpp/parser/json/mapping/Serializer.hpp"
#include "oatpp/parser/json/mapping/Deserializer.hpp"

#include "oatpp/core/macro/component.hpp"

/**
 *  Class which creates and holds Application components and registers components in oatpp::base::Environment
 *  Order of components initialization is from top to bottom
 */
class AppComponent {
public:
  
  /**
   *  Swagger component
   */
  SwaggerComponent swaggerComponent;

  DeviceDescriptorComponent deviceComponent;
  
  /**
   *  Create ConnectionProvider component which listens on the port
   */
  OATPP_CREATE_COMPONENT(std::shared_ptr<oatpp::network::ServerConnectionProvider>, serverConnectionProvider)("httpConnectionProvider", [] {
    return oatpp::network::server::SimpleTCPConnectionProvider::createShared(80);
  }());

  OATPP_CREATE_COMPONENT(std::shared_ptr<oatpp::network::ServerConnectionProvider>, udpConnectionProvider)("udpConnectionProvider", [] {
    return oatpp::network::server::SimpleUDPConnectionProvider::createShared(1900);
  }());
  
  /**
   *  Create Router component
   */
  OATPP_CREATE_COMPONENT(std::shared_ptr<oatpp::web::server::HttpRouter>, httpRouter)("httpRouter", [] {
    return oatpp::web::server::HttpRouter::createShared();
  }());

  OATPP_CREATE_COMPONENT(std::shared_ptr<oatpp::web::server::HttpRouter>, ssdpRouter)("ssdpRouter", [] {
    return oatpp::web::server::HttpRouter::createShared();
  }());
  
  /**
   *  Create ConnectionHandler component which uses Router component to route requests
   */
  OATPP_CREATE_COMPONENT(std::shared_ptr<oatpp::network::server::ConnectionHandler>, serverConnectionHandler)("httpConnectionHandler", [] {
    OATPP_COMPONENT(std::shared_ptr<oatpp::web::server::HttpRouter>, router, "httpRouter"); // get Router component
    return oatpp::web::server::HttpConnectionHandler::createShared(router);
  }());

  /**
   *  Create ConnectionHandler component which uses Router component to route requests
   */
  OATPP_CREATE_COMPONENT(std::shared_ptr<oatpp::network::server::ConnectionHandler>, udpConnectionHandler)("ssdpConnectionHandler", [] {
    OATPP_COMPONENT(std::shared_ptr<oatpp::web::server::HttpRouter>, router, "ssdpRouter"); // get Router component
    return oatpp::web::server::HttpConnectionHandler::createShared(router);
  }());
  
  /**
   *  Create ObjectMapper component to serialize/deserialize DTOs in Contoller's API
   */
  OATPP_CREATE_COMPONENT(std::shared_ptr<oatpp::data::mapping::ObjectMapper>, apiObjectMapper)([] {
    auto objectMapper = oatpp::parser::json::mapping::ObjectMapper::createShared();
    objectMapper->getDeserializer()->getConfig()->allowUnknownFields = true;
    objectMapper->getSerializer()->getConfig()->includeNullFields = false;
    return objectMapper;
  }());
  
  /**
   *  Create Demo-Database component which stores information about users
   */
  OATPP_CREATE_COMPONENT(std::shared_ptr<Database>, database)([] {
    return std::make_shared<Database>();
  }());

};

#endif /* AppComponent_hpp */
