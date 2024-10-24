#! /usr/bin/env bash


# set vars
DIR_REM="https://github.com/UoH-HIVE/raspberry_pi_wifi_controller.git"
DIR_SRC="/root/src"
DIR_LOCAL="/root/local"

# if /usr/src/hive_rpi/ doesnt exist, make it
if [ ! -d "$DIR_SRC" ]; then mkdir $DIR_SRC; fi

# clone repo to src
git clone "$DIR_REM" "$DIR_SRC" --recurse-submodules

# run setup
chmod +x "$DIR_SRC/scripts/setup.sh"
bash "$DIR_SRC/scripts/setup.sh"
