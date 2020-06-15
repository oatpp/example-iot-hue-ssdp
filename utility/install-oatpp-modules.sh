#!/bin/sh

installOatppModule() {
  MODULE_NAME="$1"

  git clone --depth=1 https://github.com/oatpp/$MODULE_NAME

  cd $MODULE_NAME
  mkdir build
  cd build

  cmake ..
  make install

  cd ../../
}

rm -rf tmp

mkdir tmp
cd tmp

##########################################################
## install oatpp
installOatppModule "oatpp"


##########################################################
## install oatpp-swagger
installOatppModule "oatpp-swagger"

##########################################################
## install oatpp-ssdp
installOatppModule "oatpp-ssdp"

##########################################################

cd ../

rm -rf tmp