FROM ubuntu:mantic
RUN apt -y update --allow-unauthenticated --allow-insecure-repositories \
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
    && update-alternatives --install /usr/bin/clang++ clang++ /usr/bin/clang++-17 17 \
    && apt -y install git libunistring-dev libudev-dev cmake

WORKDIR /opt/

RUN git clone https://github.com/scylladb/seastar.git \
    && cd seastar \
    && /opt/seastar/install-dependencies.sh \
    && ./configure.py --mode=release --prefix=/usr/local \
    && ninja -C build/release install

WORKDIR /opt/catch2
RUN git clone https://github.com/catchorg/Catch2.git \
    && cd Catch2/ \
    && cmake -Bbuild -H. -DBUILD_TESTING=OFF \
    && cmake --build build/ --target install
RUN mkdir /tmp/fastkv/
WORKDIR /home/src/
CMD /bin/bash