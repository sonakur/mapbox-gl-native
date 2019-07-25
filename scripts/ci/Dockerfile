FROM ubuntu:disco-20190718

ENV DEBIAN_FRONTEND="noninteractive"
ENV ANDROID_HOME="/opt/android"

WORKDIR /src

# Use faster mirrors from Finland
COPY etc/apt/sources.list /etc/apt

RUN set -eu && apt-get update

# CI requirements
RUN set -eu && apt-get install -y \
    ca-certificates \
    git \
    gzip \
    ssh \
    tar

# Base dependencies
RUN set -eu && apt-get install -y \
    ccache \
    clang-8 \
    clang-format-8 \
    clang-tidy-8 \
    cmake \
    g++-8 \
    libc++-8-dev \
    libc++abi-8-dev \
    mesa-common-dev \
    ninja-build \
    nodejs \
    npm \
    pkg-config \
    software-properties-common \
    xvfb

# Linux dependencies
RUN set -eu && apt-get install -y \
    libcurl4-openssl-dev \
    libgl1-mesa-dev \
    libglfw3-dev \
    libicu-dev \
    libjpeg-turbo8-dev \
    libpng-dev \
    libuv1-dev \
    zlib1g-dev

# Qt dependencies
RUN set -eu && apt-get install -y \
    qt5-default

# Android dependencies
RUN set -eu && apt-get install -y \
    coreutils \
    curl \
    openjdk-8-jdk-headless \
    unzip

# Install old compilers
RUN set -eu \
    && apt-key adv --keyserver keyserver.ubuntu.com --recv-keys 1E9377A2BA9EF27F \ 
    && add-apt-repository "deb http://ppa.launchpad.net/ubuntu-toolchain-r/test/ubuntu xenial main" \
    && add-apt-repository "deb http://us.archive.ubuntu.com/ubuntu/ xenial main universe" \
    && apt-get install -y \
        g++-4.9 \
        g++-5

# Install Android NDK
RUN set -eu \
    && mkdir -p ${ANDROID_HOME} && cd ${ANDROID_HOME} \
    && curl -L --retry 3 https://dl.google.com/android/repository/android-ndk-r19-linux-x86_64.zip -o ndk.zip \
    && (echo "f02ad84cb5b6e1ff3eea9e6168037c823408c8ac  ndk.zip" | sha1sum -c) \
    && unzip -q ndk.zip && rm ndk.zip && mv android-ndk-r* ndk-bundle

# Configure ccache
RUN set -eu && /usr/sbin/update-ccache-symlinks

# Cleanup
RUN set -eu && apt-get clean
