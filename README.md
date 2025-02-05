# raspberry_pi_wifi_controller

## TODO:

- [x] Decide on language (c)
- [ ] Interface with GPIO
- [x] Make script to poll the repo to check for changes
- [x] Write build script
- [x] Write service files
- [ ] Subscribe to data stream from datacentre
- [ ] Decode data from datastream
- [ ] Isolate current instruction and set pins accordingly
- [x] Write systemd file

## Installation

1. Copy over `./scripts/bootstrap.sh`
2. SSH into pi
3. Run `bootstrap.sh` as root
4. Enter GH creds
5. Profit!!

## Formatting

This repo is formatted using the LLVM style-guide

## Severity Levels:

- 0: key information
- 1: critical information
- 2: detailed information
- 3: debug information
- 4: verbose information