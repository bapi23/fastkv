# FROM ubuntu
# WORKDIR /home/src
# RUN mkdir -p opt \
#     && dnf -y update \
#     && dnf -y install ccache git \
#     && git clone https://github.com/scylladb/seastar.git --depth=1 --branch=master /opt/seastar \
#     && /opt/seastar/install-dependencies.sh
# CMD /bin/bash

FROM ubuntu:mantic
RUN apt -y update \
    && apt -y install build-essential \
    && apt -y install gcc-12 g++-12 gcc-13 g++-13 pandoc \
    && update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-12 12 \
    && update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-12 12 \
    && update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-13 13 \
    && update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-13 13 \
    && apt -y install clang-16 clang-17 clang-tools-17 \
    && update-alternatives --install /usr/bin/clang clang /usr/bin/clang-16 16 \
    && update-alternatives --install /usr/bin/clang++ clang++ /usr/bin/clang++-16 16 \
    && update-alternatives --install /usr/bin/clang clang /usr/bin/clang-17 17 \
    && update-alternatives --install /usr/bin/clang++ clang++ /usr/bin/clang++-17 17
RUN apt -y install git libunistring-dev libudev-dev cmake
COPY ./seastar /opt/seastar
RUN /opt/seastar/install-dependencies.sh

# WORKDIR /temp/
# RUN git clone --branch v3.28.1 https://github.com/Kitware/CMake.git
# RUN apt install libssl-dev -y
# RUN cd CMake && ./bootstrap && make && make install

WORKDIR /opt/seastar
RUN ./configure.py --mode=release --prefix=/usr/local


 RUN ninja -C build/release install
 CMD /bin/bash