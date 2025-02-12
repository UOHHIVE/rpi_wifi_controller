#! /usr/bin/env bash

# set vars
DIR_REM="https://github.com/UoH-HIVE/raspberry_pi_wifi_controller.git"
DIR_SRC="/root/src"
DIR_TARGET="$DIR_SRC/target"
DIR_LOCAL="/root/local"

# if /usr/src/hive_rpi/ doesnt exist, make it
if [ ! -d "$DIR_SRC" ]; then mkdir $DIR_SRC; fi

# clone repo to src
git clone "$DIR_REM" "$DIR_SRC" --recurse-submodules

# cd to repo and pull
cd "$DIR_SRC"

git pull
git submodule update --init --recursive --remote

# # setup the folders and build the project
# make setup
# make build

# check if target dir exists
if [ ! -d "$DIR_TARGET" ]; then mkdir "$DIR_TARGET"; fi

# unzip target tarball into target dir
tar -xvf target.tar.gz -C "$DIR_TARGET"

# if target dir isnt linked, then link it
if [ ! -d "$DIR_LOCAL" ]; then ln -s "$DIR_SRC/target" "$DIR_LOCAL"; fi

# copy config file to root
cp /root/local/config.env /root/config.env

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

