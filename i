#!/bin/sh

cd build
make test
make install/strip
cd ..
