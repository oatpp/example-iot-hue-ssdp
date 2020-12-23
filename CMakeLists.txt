cmake_minimum_required(VERSION 3.1)
project(example-iot-hue-ssdp)

set(CMAKE_CXX_STANDARD 11)

add_library(example-iot-hue-ssdp-lib
        src/AppComponent.hpp
        src/SwaggerComponent.hpp
        src/DeviceDescriptorComponent.hpp
        src/controller/HueDeviceController.hpp
        src/controller/SsdpController.hpp
        src/db/Database.cpp
        src/db/Database.hpp
        src/db/model/HueDevice.hpp
        src/dto/HueDeviceDto.hpp
        src/dto/UserRegisterDto.hpp
        src/dto/GenericResponseDto.hpp)

## include directories

target_include_directories(example-iot-hue-ssdp-lib PUBLIC src)


## link libs

find_package(oatpp          1.2.5 REQUIRED)
find_package(oatpp-swagger  1.2.5 REQUIRED)
find_package(oatpp-ssdp     1.2.5 REQUIRED)

target_link_libraries(example-iot-hue-ssdp-lib
        PUBLIC oatpp::oatpp
        PUBLIC oatpp::oatpp-swagger
        PUBLIC oatpp::oatpp-ssdp
)

## define path to swagger-ui res folder

add_definitions(-DOATPP_SWAGGER_RES_PATH="${OATPP_BASE_DIR}/bin/oatpp-swagger/res")


## add executables

add_executable(example-iot-hue-ssdp-exe src/App.cpp)
target_link_libraries(example-iot-hue-ssdp-exe example-iot-hue-ssdp-lib)

add_executable(example-iot-hue-ssdp-test
        test/tests.cpp
)
target_link_libraries(example-iot-hue-ssdp-test example-iot-hue-ssdp-lib)

enable_testing()
add_test(project-tests example-iot-hue-ssdp-test)
