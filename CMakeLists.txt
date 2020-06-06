cmake_minimum_required(VERSION 3.1)
project(iot-hue)

set(CMAKE_CXX_STANDARD 11)

add_library(iot-hue-lib
        src/AppComponent.hpp
        src/SwaggerComponent.hpp
        src/controller/HueDeviceController.hpp
        src/db/Database.cpp
        src/db/Database.hpp
        src/db/model/HueDevice.hpp
        src/dto/HueDeviceDto.hpp
        src/dto/UserRegisterDto.hpp
        src/dto/GenericResponseDto.hpp src/controller/SsdpController.hpp src/DeviceDescriptorComponent.hpp)

## include directories

target_include_directories(iot-hue-lib PUBLIC src)


## link libs

find_package(oatpp          1.1.0 REQUIRED)
find_package(oatpp-swagger  1.1.0 REQUIRED)

target_link_libraries(iot-hue-lib
        PUBLIC oatpp::oatpp
        PUBLIC oatpp::oatpp-swagger
)

## define path to swagger-ui res folder

add_definitions(-DOATPP_SWAGGER_RES_PATH="${OATPP_BASE_DIR}/bin/oatpp-swagger/res")


## add executables

add_executable(iot-hue-exe src/App.cpp src/connection/SimpleUDPConnectionProvider.cpp src/connection/SimpleUDPConnectionProvider.hpp)
target_link_libraries(iot-hue-exe iot-hue-lib)

add_executable(iot-hue-test
        test/tests.cpp
)
target_link_libraries(iot-hue-test iot-hue-lib)

enable_testing()
add_test(project-tests iot-hue-test)
