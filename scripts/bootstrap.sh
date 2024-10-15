#! /usr/bin/env bash

# set vars
DIR_REM   = "https://github.com/UoH-HIVE/raspberry_pi_wifi_controller.git"
DIR_NAME  = "hive_rpi"
DIR_SRC   = "/usr/src/$DIR_NAME"
DIR_LOCAL = "/usr/local/$DIR_NAME"

# clone repo to src
git clone "$DIR_REM" "$DIR_LOCAL"

# run setup
chmod +x "$DIR_SRC/scripts/setup.sh"
bash "$DIR_SRC/scripts/setup.sh"
