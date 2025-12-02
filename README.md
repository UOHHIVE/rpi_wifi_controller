# rpi_wifi_controller
*raspberry pi wifi controller*

## Connect to data centre

The robots need to be connected to the data centre to send and receive data - work with all the other devices and programs.

1. Run data centre
2. Turn on robot

The robot should connect in less than a few minutes (less than a minute ideally).

If the robot is not connecting then consider the following possible issues:

 - Flatbuffers out of date / changed
 - Date/time out of sync
 - Robot is low on battery power
 - Networking issue (if you suspect this, check with tech office)
 - If all the above are not the issue, then as a last resort a reflash may be needed to reset the pi completely (ask tech office for details)

## SSH

This will allow you to connect to the robots from your pc. Can view logs, move files, downlaod repos, etc.

1. Ensure the robot is on and connected to data centre.
2. Open cmd
3. Type: ssh user@hostIP (The user should be “root” and hostIP should be the IP of the robot when connected to data centre)
4. Type “yes” when it asks if youre sure you want to continue connecting
5. Password: hive-P00* --> Asterisk being replaced by the number on the robot. E.g. R2-D2’s number is P004, so the password would be “hive-P004”.

## Installation

To install the repository on the pi:

1. Ssh into robot
2. Curl latest target.tar.gz from the release pages
3. Extract the tar.gz using tar
4. Open the extracted file
5. Find either of the update shell commands
6. Give yourself executable permissions on that file using chmod +x
7. Move the executable to root
8. Run the executable (Can take an hour to complete if cpu is underclocked). It will be complete once it asks you for the next instruction
9. Can also untar a target file if there isn’t one on the root already (Might work without)
10. There should appear a flatbuffer file that you will have to rename to just flatbuffers instead of “flatbuffers-25.2.10”

## Submodules

This repository contains submodules.
To use these submodules for dev purposes, you must initialise them one by one on the robots (can be done by curl the release).

### Update

Ensure you've already ssh into the robot. To update the submodules: 

1. Ensure you have updated the submodule's repository first
2. Open cmd in the local rpi_wifi_controller folder
3. git submodule update --init
4. git submodule update --remote
5. git add .
6. git commit -m "<*Your commit message here*>"
7. git push

## Dev

At some point, you may need to do dev directly on the pi. To do that, you will need to curl and extract all the submodules of this repository. Ensure you've already ssh into the robot.

1. Change directories to the empty file in place of the submodule
2. Create a release on the GitHub repo or if there is already one, click on it to display
3. Check if repo is public or private
   - If public, copy link of tar.gz file in releases and use curl in robot to download it
   - If private, ensure you are a collaborator of the repository, create a Personal Access Token on GitHub in your settings, and then use wget in the robot like this:
      wget --header "Authorization: token <*a Personal Access Token from your account*>" https://github.com/???
