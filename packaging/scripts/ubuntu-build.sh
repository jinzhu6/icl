#!/bin/bash

rm -r build
mkdir build && cd build

cmake -DCMAKE_SKIP_RPATH=ON \
      -DBUILD_WITH_OPENCV=ON  -DBUILD_WITH_QT=ON \
      -DBUILD_WITH_OPENGL=ON \
      -DBUILD_WITH_EIGEN3=ON \
      -DBUILD_WITH_IMAGEMAGICK=ON \
      -DBUILD_WITH_LIBAV=ON  -DBUILD_WITH_LIBDC=ON \
      -DBUILD_WITH_LIBFREENECT=ON \
      -DBUILD_WITH_V4L=ON \
      -DBUILD_WITH_LIBUSB=ON -DBUILD_WITH_ZMQ=ON \
      -DBUILD_WITH_OPENCL=ON \
      -DBUILD_WITH_OPENNI=ON -DBUILD_WITH_PCL=ON \
      -DBUILD_EXAMPLES=ON -DBUILD_DEMOS=ON \
      -DBUILD_APPS=ON -DBUILD_DEBS=ON ..