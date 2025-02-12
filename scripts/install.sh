#! /usr/bin/env bash

# update repos, install updates
apt update -y
apt upgrade -y

# make sure required software is installed
apt install software-properties-common wget xz-utils gcc build-essential clang make cmake -y
apt install python3 python3-pip -y
apt install gpiod libgpoid-dev -y
apt install fakeroot gettext-base -y

# FLATBUFFER_VERISON = 24.3.6

# manually install flatbuffers header files bc its not packaged properly
if [ -d ~/flatbuffers ]; then rm -rf ~/flatbuffers; fi
curl -sSfL https://github.com/google/flatbuffers/archive/refs/tags/v24.12.23.tar.gz -o ~/flatbuffers.tar.gz
tar -xvzf ~/flatbuffers.tar.gz
mv ~/flatbuffers-* ~/flatbuffers

# symlink include directory
if [ ! -L /usr/include/flatbuffers/ ]; then ln -s ~/flatbuffers/include/flatbuffers/ /usr/include/flatbuffers; fi

# Setting up git
git config --global credential.helper store
# git config --global user.name 

# clone wiring pi 
git clone https://github.com/WiringPi/WiringPi.git --recursive
cd WiringPi

# build from source
./build debian

# give access to apt
chown -Rv _apt:root /var/cache/apt/archives/partial/
chmod -Rv 700 /var/cache/apt/archives/partial/

# install wiring pi
mv debian-template/wiringpi*.deb .
apt install ./wiringpi*.deb

# post install clean
cd .. 
rm -rf WiringPi/