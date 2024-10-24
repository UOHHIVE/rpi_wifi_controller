#! /usr/bin/env bash

# set vars
DIR_SRC="/root/src"
DIR_LOCAL="/root/local"

# cd to /usr/src/hive_rpi and pull
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

        systemctl stop "$service_file"
        systemctl enable "$service_file"
        systemctl start "$service_file"
    fi
done
