FROM lganzzzo/alpine-cmake:latest

ADD . /service

WORKDIR /service/utility

RUN ./install-oatpp-modules.sh Release

WORKDIR /service/build

RUN cmake ..
RUN make

EXPOSE 80 80
EXPOSE 1900 1900

ENTRYPOINT ["./example-iot-hue-ssdp-exe"]
