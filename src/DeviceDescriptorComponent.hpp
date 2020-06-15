
#ifndef DeviceDescriptorComponent_hpp
#define DeviceDescriptorComponent_hpp

#include <oatpp/core/Types.hpp>
#include <oatpp/core/macro/component.hpp>

class DeviceDescriptorComponent {
 public:
  class DeviceDescriptor {
   public:
    oatpp::String sn;
    oatpp::String uuid;
    oatpp::String ipPort;
    oatpp::String mac;
  };

  OATPP_CREATE_COMPONENT(std::shared_ptr<DeviceDescriptor>, deviceDescriptor)("deviceDescriptor", [] {
    auto desc = std::make_shared<DeviceDescriptor>();
    // ToDo: Add your machines Address and Port here! You have to come up with your own way to automate this...
    desc->ipPort = "192.168.166.136:80"; // your real IP and Port your HTTP-Controller is running on

    // assignable
    desc->mac = "be5t0a70cafe"; // can be a fake one

    // fixed
    desc->sn = "1000000471337";
    desc->uuid = "2f402f80-da50-11e1-9b23-" + desc->mac;
    return desc;
  }());

};

#endif //DeviceDescriptorComponent_hpp
