#! /usr/bin/env bash

# set vars
DIR_REM="https://github.com/UOHHIVE/rpi_wifi_controller/raw/refs/heads/main/target.tar.gz"
DIR_SRC="/root/src"
DIR_TARGET="$DIR_SRC/target"
DIR_LOCAL="/root/local"
RPI_SLEEP_TIME=30

# update repos, install updates
dietpi-update 1
apt update -y
apt upgrade -y

# make sure required software is installed
apt install software-properties-common wget xz-utils gcc build-essential clang make cmake git -y
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

# remove old directories if they exist
if [ -d rpi_wifi_controller ]; then rm -rf rpi_wifi_controller; fi
if [ -d "$DIR_SRC" ]; then rm -rf "$DIR_SRC"; fi

# make sure target dirs exist
if [ ! -d "$DIR_SRC" ]; then mkdir $DIR_SRC; fi
if [ ! -d "$DIR_TARGET" ]; then mkdir "$DIR_TARGET"; fi

# cd into dir
# cd "$DIR_SRC"

# curl DIR_REM
git clone https://github.com/UOHHIVE/rpi_wifi_controller.git
mv rpi_wifi_controller/target.tar.gz .
tar -xvf ~/target.tar.gz -C "$DIR_SRC"

# unzip target tarball into target dir
rm -rf "$DIR_TARGET"/*
tar -xvf target.tar.gz -C "$DIR_SRC"

# if target dir isnt linked, then link it
if [ ! -d "$DIR_LOCAL" ]; then ln -s "$DIR_TARGET" "$DIR_LOCAL"; fi

# copy config file to root if it doesnt exist
if [ ! -f /root/config.env ]; then cp /root/local/config.env /root/config.env; fi

# start and enable rpi_controller.service
if [ -f "$DIR_SRC/scripts/rpi_controller.service" ]; then
  ln -sf "$DIR_SRC/scripts/rpi_controller.service" "/etc/systemd/system/rpi_controller.service"
  systemctl stop rpi_controller.service
  systemctl enable rpi_controller.service
  systemctl start rpi_controller.service
fi
