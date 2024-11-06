#! /usr/bin/env bash

# update repos, install updates
apt update -y
apt upgrade -y

# make sure required software is installed
apt install software-properties-common wget xz-utils gcc build-essential clang -y
apt install python3 python3-pip -y
apt install gpiod libgpoid-dev -y
apt install fakeroot gettext-base -y

# pip install RPi.GPIO
# pip install gpiod

# clone wiring pi 
git clone https://github.com/WiringPi/WiringPi.git
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

# set vars
DIR_REM="https://github.com/UoH-HIVE/raspberry_pi_wifi_controller.git"
DIR_SRC="/root/src"
DIR_LOCAL="/root/local"

# if /usr/src/hive_rpi/ doesnt exist, make it
if [ ! -d "$DIR_SRC" ]; then mkdir $DIR_SRC; fi

# clone repo to src
git clone "$DIR_REM" "$DIR_SRC" --recurse-submodules

# cd to repo and pull
cd "$DIR_SRC"
git pull

# setup the folders and build the project
make setup
make build

# if target dir isnt linked, then link it
if [ ! -d "$DIR_LOCAL" ]; then ln -s "$DIR_SRC/target" "$DIR_LOCAL"; fi

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

