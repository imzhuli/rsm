#!/bin/sh

set -e
if [ -n $1 ];
then
    PARAL=$1
else
    PARAL=32
fi

rm -rf ./build
mkdir build

cd build
cmake -DCMAKE_BUILD_TYPE=Debug -Wno-dev ../
make -j ${PARAL}
make test
cd ..
