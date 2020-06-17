# Example-IoT-Hue [![Build Status](https://dev.azure.com/lganzzzo/lganzzzo/_apis/build/status/oatpp.example-iot-hue-ssdp?branchName=master)](https://dev.azure.com/lganzzzo/lganzzzo/_build/latest?definitionId=29&branchName=master)

Example project how-to create an Philips Hue compatible REST-API that is discovered and controllable by Hue compatible Smart-Home devices like Amazon Alexa or Google Echo.

It demonstrates how Oat++ can be used to develop an Amazon Alexa or Google Home compatible REST-API which emulates Philips Hue bulbs. Oat++ answers to search requests of you favorite SmartHome hub and you can register your fake bulbs to it.  
After the registration of your fake bulbs to your Hub/Alexa/Google Home, you can control your Oat++ application with  
🗣️"Alexa, turn on &lt;your fake device name&gt;"!


For this discoverability, the `oatpp-ssdp` module is used to receive and answer SSDP searches.

This REST-API was implemented with the help of the Hue API unofficial reference documentation by burgestrand.se

See more:

- [Oat++ Website](https://oatpp.io/)
- [Oat++ Github Repository](https://github.com/oatpp/oatpp)
- [Get Started](https://oatpp.io/docs/start)
- [Philips Hue API — Unofficial Reference Documentation](http://www.burgestrand.se/hue-api/)

## Overview

This project is using [oatpp](https://github.com/oatpp/oatpp), [oatpp-swagger](https://github.com/oatpp/oatpp-swagger) and  [oatpp-ssdp](https://github.com/oatpp/oatpp-ssdp) modules.

### Project layout

```
|- CMakeLists.txt                        // projects CMakeLists.txt
|- src/
|   |
|   |- controller/                       // Folder containing HueDeviceController and SsdpController where all endpoints are declared
|   |- db/                               // Folder with database mock
|   |- dto/                              // DTOs are declared here
|   |- SwaggerComponent.hpp              // Swagger-UI config
|   |- DeviceDescriptorComponent.hpp     // Component describing your "Hue Hub" (YOU HAVE TO CONFIGURE THIS FILE TO FIT YOUR ENVIRONMENT)
|   |- AppComponent.hpp                  // Service config
|   |- App.cpp                           // main() is here
|
|- test/                                 // test folder
|- utility/install-oatpp-modules.sh      // utility script to install required oatpp-modules.
```

---

### Build and Run

Before you run this example you have to edit `src/DeviceDescriptorComponent.hpp` to match your IP address.
Since this is only an example and to keep it simple this is not automated or parameterised!
You have to come up with your own implementation that fits your environment.

```c++
OATPP_CREATE_COMPONENT(std::shared_ptr<DeviceDescriptor>, deviceDescriptor)("deviceDescriptor", [] {
auto desc = std::make_shared<DeviceDescriptor>();
// ToDo: Add your machines Address and Port here! You have to come up with your own way to automate this...
desc->ipPort = "192.168.100.100:80"; // your real IP and Port your HTTP-Controller is running on

// assignable
desc->mac = "be5t0a70cafe"; // can be a fake one

// fixed
desc->sn = "1000000471337";
desc->uuid = "2f402f80-da50-11e1-9b23-" + desc->mac;
return desc;
}());
```

#### Using CMake

**Requires**

- `oatpp`, `oatpp-ssdp` and `oatpp-swagger` modules installed. You may run `utility/install-oatpp-modules.sh` 
script to install required oatpp modules.

```
$ mkdir build && cd build
$ cmake ..
$ make 
$ ./example-iot-hue-ssdp-exe        # - run application.
```

#### In Docker

```
$ docker build -t example-iot-hue-ssdp .
$ docker run -p 8000:8000 -t example-iot-hue-ssdp
```

---

### Endpoints declaration

All implemented endpoints are compatible to a Philips Hue bridge (V1 and V3).
**Their path and structure are fixed!**

#### SSDP: Search Responder
```c++
ENDPOINT("M-SEARCH", "*", star)
```
This Endpoint accepts and answers to `M-SEARCH` SSDP packets like a Philips Hue hub would do.

#### HTTP: description.xml
```c++
ENDPOINT("GET", "/description.xml", description)
```
In the discovery answer, a reference to this endpoint is send back.
This endpoints emulates a static `desciption.xml` which includes all necessary information required to act as an Philips Hue hub.

See [Bridge discovery (burgestrand.se)](http://www.burgestrand.se/hue-api/api/discovery/)

#### HTTP: One-Shot 'user' registration
```c++
ENDPOINT("POST", "/api", appRegister, BODY_DTO(oatpp::Object<UserRegisterDto>, userRegister))
```

This endpoint just emulates a valid user-registration on a Philips Hue hub.

See [Application registration (burgestrand.se)](http://www.burgestrand.se/hue-api/api/auth/registration/)

#### HTTP: Get all 'lights'
```c++
ENDPOINT("GET", "/api/{username}/lights", getLights, PATH(String, username))
```

This endpoint returns a **object** of all devices in a Philips Hue compatible fashion.
However, formally this endpoint should just return the names. But returning the full list is fine too.

See [Lights (burgestrand.se)](http://www.burgestrand.se/hue-api/api/lights/)

#### HTTP: Get state of a specific light
```c++
ENDPOINT("GET", "/api/{username}/lights/{hueId}", getLight, PATH(String, username), PATH(Int32, hueId))
```
This endpoint returns the state of the light given in `{hueId}` in a Philips Hue compatible fashion.

See [Lights (burgestrand.se)](http://www.burgestrand.se/hue-api/api/lights/)

#### HTTP: Set state of a specific light
```c++
ENDPOINT("PUT", "/api/{username}/lights/{hueId}/state", updateState,
      PATH(String, username),
      PATH(Int32, hueId),
      BODY_DTO(Object<HueDeviceStateDto>, state))
```

This endpoint accepts a Philips Hue compatible state-object and sets the state in the internal database accordingly.
It is called e.g. by Alexa if you tell it  "Alexa, turn on <devicename>".
Finally it returns a "success" or "error" object.

See [Lights (burgestrand.se)](http://www.burgestrand.se/hue-api/api/lights/)
