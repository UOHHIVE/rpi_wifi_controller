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

if [ ! -f "/etc/systemd/system/rpi_controller.service" ]; then ln -sf "/root/local/rpi_controller.service" "/etc/systemd/system/rpi_controller.service"; fi

# start and enable rpi_controller.service
if [ -f "/etc/systemd/system/rpi_controller.service" ]; then
  if systemctl list-units --type=service | grep -q "rpi_controller.service"; then
    systemctl stop rpi_controller.service
  fi
  systemctl enable rpi_controller.service
  systemctl start rpi_controller.service
fi

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

if [ ! -f "/etc/systemd/system/rpi_controller.service" ]; then ln -sf "/root/local/rpi_controller.service" "/etc/systemd/system/rpi_controller.service"; fi

# start and enable rpi_controller.service
if [ -f "/etc/systemd/system/rpi_controller.service" ]; then
  if systemctl list-units --type=service | grep -q "rpi_controller.service"; then
    systemctl stop rpi_controller.service
  fi
  systemctl enable rpi_controller.service
  systemctl start rpi_controller.service
fi



# Overwrite or create /boot/config.txt with the specified content
# no, this is not good practice, it just works though...
cat <<EOL > /boot/config.txt
# Docs: https://www.raspberrypi.com/documentation/computers/config_txt.html
# Overlays: https://github.com/raspberrypi/firmware/blob/master/boot/overlays/README

#-------Display---------
# Max allocated framebuffers: Set to "0" in headless mode to reduce memory usage
# - Defaults to "2" on RPi4 and "1" on earlier RPi models
#max_framebuffers=0

# If you get no picture, set the following to "1" to apply most compatible HDMI settings.
#hdmi_safe=1

# Uncomment to adjust the HDMI signal strength if you have interferences, blanking, or no display.
# - Ranges from "0" to "11", use values above "7" only if required, e.g. with very long HDMI cable.
# - Default on first RPi1 A/B is "2", else "5", on RPi4 this setting is ignored.
#config_hdmi_boost=5

# Uncomment if HDMI display is not detected and composite is being outputted.
#hdmi_force_hotplug=1

# Uncomment to disable HDMI even if plugged, e.g. to force composite output.
#hdmi_ignore_hotplug=1

# Uncomment to force a console size. By default it will be display's size minus overscan.
#framebuffer_width=1280
#framebuffer_height=720

# Uncomment to enable SDTV/composite output on RPi4. This has no effect on previous RPi models.
#enable_tvout=1
# SDTV mode
#sdtv_mode=0

# Uncomment to force a specific HDMI mode (this will force VGA).
#hdmi_group=1
#hdmi_mode=1

# Uncomment to force an HDMI mode rather than DVI. This enables HDMI audio in DMT modes.
#hdmi_drive=2

# Set "hdmi_blanking=1" to allow the display going into standby after 10 minutes without input.
# With default value "0", the display shows a blank screen instead, but will not go into standby.
# NB: Some legacy OpenMAX applications (OMXPlayer) cannot wake screens from real standby.
hdmi_blanking=1

# Set to "1" if your display has a black border of unused pixels visible.
disable_overscan=1

# Uncomment the following to adjust overscan.
# Use positive numbers if console goes off screen, and negative if there is too much border.
#overscan_left=16
#overscan_right=16
#overscan_top=16
#overscan_bottom=16

# Rotation
#display_hdmi_rotate=0
#lcd_rotate=0

#-------RPi camera module-------
#start_x=1
#disable_camera_led=1

#-------GPU memory splits-------
gpu_mem_256=16
gpu_mem_512=16
gpu_mem_1024=16

#-------Boot splash screen------
disable_splash=1

#-------Onboard sound-----------
dtparam=audio=off

#-------I2C-------------
dtparam=i2c_arm=on
#dtparam=i2c_arm_baudrate=100000

#-------SPI-------------
#dtparam=spi=off

#-------Serial/UART-----
# NB: "enable_uart=1" will enforce "core_freq=250" on RPi models with onboard WiFi.
enable_uart=0

#-------SD card HPD-----
# Comment to enable SD card hot-plug detection, while booting via USB or network.
# NB: This causes constant CPU load and kernel errors when no SD card is inserted.
dtparam=sd_poll_once

#-------Overclock-------
temp_limit=75
initial_turbo=20

#over_voltage=0
arm_freq=169
core_freq=42

#over_voltage_min=0
#arm_freq_min=300
#core_freq_min=250
#sdram_freq_min=400
arm_64bit=1
EOL

