#!/usr/bin/env bash

set -e

# Add a 4GiB swap space (needed for large compilations on Raspberry Pi)
if [ ! -f /swapfile ]; then
    sudo fallocate -l 4G /swapfile
    sudo chmod 600 /swapfile 
    sudo mkswap -L swap /swapfile 
    sudo swapon /swapfile
    echo "/swapfile	none	swap	sw	0	0" | sudo tee -a /etc/fstab
fi

# Install the necessary packages and libraries
# sudo apt update
sudo apt install -y git gcc-8 g++-8 make cmake liblapacke-dev python3-dev python3-pip python3-numpy python3-pytest python3-opencv &

# Clone the necessary GitHub repositories
cd
git clone https://github.com/google/googletest.git &
git clone https://github.com/tttapa/EAGLE.git &
git clone https://github.com/tttapa/GridFinder.git &
git clone https://github.com/pybind/pybind11.git &
wait

export CC=gcc-8
export CXX=g++-8

# Install pybind11
mkdir -p pybind11/build && cd pybind11/build
cmake ..
sudo make install/fast

# Build GridFinder and run the tests
cd ~/GridFinder/
cp -r cmake.example cmake
mkdir -p build && cd build
cmake ..
make -j$(nproc) && make test

# Install the GridFinder Python module
make python
