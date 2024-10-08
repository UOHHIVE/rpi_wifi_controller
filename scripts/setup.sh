

DIR_NAME  = "hive_rpi"
DIR_SRC   = "/usr/src/$DIR_NAME"
DIR_LOCAL = "/usr/local/$DIR_NAME"


# if /usr/src/hive_rpi/ doesnt exist, make it
# if /usr/local/hive_rpi/ exists, remove it

if [ ! -d "$DIR_SRC" ]; then mkdir $DIR_SRC; fi
if [ -d "$DIR_LOCAL" ]; then rm "$DIR_LOCAL/"; fi


# cd to /usr/src/hive_rpi
# git pull
# make build

cd "$DIR_SRC"
git pull --origin main
make build


# link /usr/src/hive_rpi/target/ to /usr/local/hive_rpi/
# start and enable rpi_controller.service and rpi_updater.service

ln -s "$DIR_SRC/target" "$DIR_LOCAL"

for service_file in "$DIR_SRC"/*.service; do
    if [ -f "$service_file" ]; then
        service_name=$(basename "$service_file")

        systemctl stop "$service_name"
        systemctl enable "$service_name"
        systemctl start "$service_name"
    fi
done
