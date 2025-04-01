FROM mcr.microsoft.com/devcontainers/cpp:1-ubuntu-24.04

# Prevent interactive prompts
ENV DEBIAN_FRONTEND=noninteractive

# Install required dependencies
RUN apt update && apt install -y --no-install-recommends \
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
    git \
    qttools5-dev \
    qttools5-dev-tools && \
    rm -rf /var/lib/apt/lists/*

# Set working directory
WORKDIR /workspaces/DataPlotter
