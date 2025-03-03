#! /usr/bin/env bash

# set vars
DIR_REM="https://github.com/UoH-HIVE/rpi_wifi_controller.git"
DIR_SRC="/root/src"
DIR_TARGET="$DIR_SRC/target"
DIR_LOCAL="/root/local"
RPI_SLEEP_TIME=30

# update repos, install updates
dietpi-update 1
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

# Setting up git
git config --global credential.helper store

# if /usr/src/hive_rpi/ doesnt exist, make it
if [ ! -d "$DIR_SRC" ]; then mkdir $DIR_SRC; fi
if [ ! -d "$DIR_TARGET" ]; then mkdir "$DIR_TARGET"; fi

# clone repo to src
if [ -z "$NO_CLONE" ]; then git clone "$DIR_REM" "$DIR_SRC"; fi

# cd to repo and pull
cd "$DIR_SRC"
git pull origin main

# unzip target tarball into target dir
rm -rf "$DIR_TARGET"/*
tar -xvf target.tar.gz -C "$DIR_SRC"

# if target dir isnt linked, then link it
if [ ! -d "$DIR_LOCAL" ]; then ln -s "$DIR_TARGET" "$DIR_LOCAL"; fi

# copy config file to root if it doesnt exist
if [ ! -f /root/config.env ]; then cp /root/local/config.env /root/config.env; fi

# move into local dir
cd "$DIR_LOCAL"

# start and enable rpi_controller.service and rpi_updater.service
for service_file in "$DIR_SRC"/scripts/*.service; do
  if [ -f "$service_file" ]; then
    service_name=$(basename "$service_file")

    ln -sf "$service_file" "/etc/systemd/system/$service_name"

    systemctl stop "$service_name"
    systemctl enable "$service_name"
    systemctl start "$service_name"
  fi
done

# if RPI_SLEEP is set, sleep for 30 seconds
if [ -n "$RPI_SLEEP" ]; then sleep "$RPI_SLEEP_TIME"; fi