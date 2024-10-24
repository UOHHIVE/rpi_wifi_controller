#! /usr/bin/env bash

# make dirs
mkdir /root/src/
mkdir /root/local/

# set vars
DIR_REM=    "https://github.com/UoH-HIVE/raspberry_pi_wifi_controller.git"
DIR_SRC=    "/root/src/"
DIR_LOCAL=  "/root/local/"

# clone repo to src
git clone "$DIR_REM" "$DIR_SRC" --recurse-submodules

# run setup
chmod +x "$DIR_SRC/scripts/setup.sh"
bash "$DIR_SRC/scripts/setup.sh"
