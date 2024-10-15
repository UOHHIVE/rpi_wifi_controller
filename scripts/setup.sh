#! /usr/bin/env bash

# set vars
DIR_NAME  = "hive_rpi"
DIR_SRC   = "/usr/src/$DIR_NAME"
DIR_LOCAL = "/usr/local/$DIR_NAME"

# if /usr/src/hive_rpi/ doesnt exist, make it
if [ ! -d "$DIR_SRC" ]; then mkdir $DIR_SRC; fi

# cd to /usr/src/hive_rpi and pull
cd "$DIR_SRC"
git pull

# setup the folders and build the project
make setup
make build

# if target dir isnt linked, then link it
if [ ! -d "$DIR_LOCAL" ]; then ln -s "$DIR_SRC/target" "$DIR_LOCAL"; fi

# start and enable rpi_controller.service and rpi_updater.service
for service_file in "$DIR_SRC"/*.service; do
    if [ -f "$service_file" ]; then
        service_name=$(basename "$service_file")

        systemctl stop "$service_name"
        systemctl enable "$service_name"
        systemctl start "$service_name"
    fi
done
