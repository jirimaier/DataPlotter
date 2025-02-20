FROM mcr.microsoft.com/devcontainers/cpp:1-ubuntu-20.04

# Install dependencies
RUN apt update && apt install -y \
    qt5-default \
    qtdeclarative5-dev \
    libqt5quick5 \
    libqt5quickcontrols2-5 \
    libqt5serialport5-dev \
    mesa-common-dev \
    libglu1-mesa-dev \
    qtquickcontrols2-5-dev \
    libqt5opengl5-dev \
    qt5dxcb-plugin \
    libxcb-util-dev \
    build-essential \
    cmake \
    ninja-build \
    pkg-config \
    git

# Set working directory
WORKDIR /workspaces
