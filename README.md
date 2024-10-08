# raspberry_pi_wifi_controller

## TODO:

- [x] Decide on language (c)
- [ ] Interface with GPIO
- [ ] Make script to poll the repo to check for changes
- [x] Write build script
- [ ] Write service files
- [ ] Subscribe to data stream from datacentre
- [ ] Decode data from datastream
- [ ] Isolate current instruction and set pins accordingly
- [ ] Write systemd file

## Installation

1. copy over a scoped access token and [`./scripts/bootstrap.sh`](./scripts/bootstrap.sh)
2. run bootstrap.sh making sure to pass in the scoped access token as an argument or in a text file called "token.txt"
3. the `rpi_controller` and `rpi_updater` services should be enabled, if so you are good to go!!

## Formatting

This repo is formatted using the LLVM style-guide