
#include "controller/SsdpController.hpp"
#include "controller/HueDeviceController.hpp"
#include "AppComponent.hpp"

#include "oatpp-swagger/Controller.hpp"

#include "oatpp/network/Server.hpp"

#include <iostream>
#include <thread>

/**
 *  run() method.
 *  1) set Environment components.
 *  2) add ApiController's endpoints to router
 *  3) run server
 */

void run() {
  
  std::shared_ptr<AppComponent> components = std::make_shared<AppComponent>(); // Create scope Environment components

  /* Get Database instance to add devices to it */
  auto db = components->database.getObject();

  /* Add a device called 'Oat' */
  db->registerHueDevice("Oat");

  /* Add another device called 'Grain' */
  db->registerHueDevice("Grain");


  /* get the router for HTTP calls */
  auto router = components->httpRouter.getObject();

  /* create the Swagger endpoint documentation engine*/
  oatpp::web::server::api::Endpoints docEndpoints;

  /* create the Hue HTTP REST controller */
  docEndpoints.append(router->addController(HueDeviceController::createShared())->getEndpoints());

  /* create swagger UI controller */
  router->addController(oatpp::swagger::Controller::createShared(docEndpoints));

  /* create the SSDP-Router and SSDP-Controller and add its endpoints to the SSDP-Router */
  auto ssdpRouter = components->ssdpRouter.getObject();

  ssdpRouter->addController(SsdpController::createShared());

  OATPP_LOGD("SSDPRouter", "Mappings:");
  ssdpRouter->logRouterMappings();
  OATPP_LOGD("HTTPRouter", "Mappings:");
  router->logRouterMappings();


  /* create http and ssdp server in separate thread to have them run in parallel */
  std::thread http([components](){
    oatpp::network::Server server(components->serverConnectionProvider.getObject(),
                                  components->serverConnectionHandler.getObject());

    OATPP_LOGD("Server", "Running HTTP on port %s...", components->serverConnectionProvider.getObject()->getProperty("port").toString()->c_str());

    server.run();
  });

  std::thread ssdp([components](){
    oatpp::network::Server server(components->ssdpConnectionProvider.getObject(),
      components->ssdpStreamHandler.getObject());

    OATPP_LOGD("Server", "Running SSDP on port %s...", components->ssdpConnectionProvider.getObject()->getProperty("port").toString()->c_str());

    server.run();
  });

  if (http.joinable())
    http.join();

  if (ssdp.joinable())
    ssdp.join();

}

/**
 *  main
 */
int main(int argc, const char * argv[]) {

  oatpp::base::Environment::init();

  run();
  
  /* Print how much objects were created during app running, and what have left-probably leaked */
  /* Disable object counting for release builds using '-D OATPP_DISABLE_ENV_OBJECT_COUNTERS' flag for better performance */
  std::cout << "\nEnvironment:\n";
  std::cout << "objectsCount = " << oatpp::base::Environment::getObjectsCount() << "\n";
  std::cout << "objectsCreated = " << oatpp::base::Environment::getObjectsCreated() << "\n\n";
  
  oatpp::base::Environment::destroy();
  
  return 0;
}
