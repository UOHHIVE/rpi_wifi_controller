# raspberry_pi_wifi_controller

## Installation

1. Curl latest target.tar.gz from the releases page
2. Extract the tar.gz
3. chmod +x the executable
4. Run the executable
5. Profit

## Submodules

This repository contains submodules.
To use these submodules for dev purposes, you must initialise them one by one on the robots (can be done by curl the release).

### Update
To update the submodules: 

1. Ensure you have updated the submodule's repository first
2. Then you can open cmd in the local rpi_wifi_controller folder and update it (using "git submodule update --init" or "git submodule update --remote") and then commiting it with a message
3. Open your file explorer, find the local folder and press view -> show -> hidden items
4. Open .git folder and delete index.lock file
5. Open github desktop and push your commit
