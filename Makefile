
CC = g++
STD = c++23

#ARCH = arm64
#ARCH = x86_64

CFLAGS_TEST = -std=$(STD) -Wall -Wextra -Wpedantic -l wiringPi # -Werror -Wpedantic -Wno-unused-parameter -Wno-unused-variable -Wno-unused-function -Wno-unused-private-field -l wiringPi
CFLAGS_PROD = -std=$(STD) -flto -Wall -Wextra -O3 -l wiringPi # z-march=aarch64-linux-gnu
# CFLAGS_PROD = -arch $(ARCH) -std=$(STD) -Wall -Wextra -O3

DIR_SRC = ./src/
DIR_TARGET = ./target/
DIR_SCRIPTS = ./scripts/

DIR_COMMONS = ./src/commons
DIR_FLATBUFFERS = ./src/commons/src/flatbuffers/generated
DIR_WIRINGPI = ./src/commons/src/zumo/wiringpi

VERSION_FILE = VERSION
MAIN = main.cpp
TARGET = main.out

THIS_FILE := $(lastword $(MAKEFILE_LIST))
SRC = $(shell find $(DIR_SRC) -type f -name '*.cpp')
# SRC = $(wildcard $(DIR_SRC)*.cpp)

#DATAFILES = $(wildcard $(DIR_SRC)*.txt) $(wildcard $(DIR_SRC)*.csv)

setup:
	@echo "creating target directory"
	@if [ ! -d $(DIR_TARGET) ]; then mkdir $(DIR_TARGET); fi

test:
	@$(MAKE) -f $(THIS_FILE) setup
	@echo "$(CC) $(SRC) -o $(DIR_TARGET)$(TARGET) $(CFLAGS_TEST)"
	@$(CC) $(SRC) -o $(DIR_TARGET)$(TARGET) $(CFLAGS_PROD)
	@echo "Build complete."

build: 
	@$(MAKE) -f $(THIS_FILE) setup
	@echo "Building..."
	@echo "$(CC) $(CFLAGS_PROD) $(SRC) -o $(DIR_TARGET)$(TARGET)"
	@aarch64-linux-gnu-g++ $(SRC) -o $(DIR_TARGET)$(TARGET) $(CFLAGS_TEST)
	@echo "Build complete."
	@echo "Copying scripts..."
	@cp $(DIR_SCRIPTS)/* $(DIR_TARGET)
	@for file in *.out *.sh; do if [ -e "$file" ]; then chmod +x "$file"; fi; done

run: 
	@$(MAKE) -f $(THIS_FILE) setup
	@if [ ! -d $(DIR_TARGET)/$(TARGET) ]; then $(MAKE) -f $(THIS_FILE) test ; fi
	@echo "Running..."
	@cd $(DIR_TARGET) && ./$(TARGET) ./config.env

debug:
	@if [ ! -d $(DIR_TARGET)/$(TARGET) ]; then $(MAKE) -f $(THIS_FILE) test ; fi
	@echo "Running in debug mode..."
	@cd $(DIR_TARGET) && AP_DEBUG=1 ./$(TARGET)

clean:
	@echo "Cleaning..."
	@rm -rf $(DIR_TARGET)
	@$(MAKE) -f $(THIS_FILE) setup

submodules:
	@echo "Auto downloading all submodules..."
	@echo "---commons---"
	@read -p "Github Token: " AuthToken; \
	wget --header "Authorization: token $$AuthToken" https://github.com/UOHHIVE/commons_cpp/archive/refs/heads/main.tar.gz
	rm -fr $(DIR_COMMONS)
	tar -xvzf main.tar.gz
	mv commons_cpp-main commons
	mv commons $(DIR_COMMONS)
	rm -f main.tar.gz
	@echo "---commons done---"

	@echo "---commons_flatbuffers---"
	@read -p "Github Token: " AuthToken; \
	wget --header "Authorization: token $$AuthToken" https://github.com/UOHHIVE/commons_flatbuffers/archive/refs/heads/main.tar.gz
	rm -fr $(DIR_FLATBUFFERS)
	tar -xvzf main.tar.gz
	mv commons_flatbuffers-main generated
	mv generated $(DIR_FLATBUFFERS)
	rm -f main.tar.gz
	@echo "---commons_flatbuffers done---"

	@echo "---wiringpi---"
	curl -LO https://github.com/WiringPi/WiringPi/archive/refs/heads/master.tar.gz
	rm -fr $(DIR_WIRINGPI)
	tar -xvzf master.tar.gz
	mv WiringPi-master wiringpi
	mv wiringpi $(DIR_WIRINGPI)
	rm -f master.tar.gz
	@echo "---wiringpi done---"

	@echo "Downloaded all submodules successfully"

remove_submodules:
	@echo "Removing submodules..."
	rm -fr $(DIR_COMMONS)
	mkdir $(DIR_COMMONS)
	@echo "Removed all submodules successfully"

bump_major:
	@echo "Bumping major version..."
	@$(eval VERSION=$(shell cat $(VERSION_FILE)))
	@$(eval MAJOR=$(shell echo $(VERSION) | cut -d. -f1))
	@$(eval MINOR=$(shell echo $(VERSION) | cut -d. -f2))
	@$(eval NEW_MAJOR=$(shell echo $$(($(MAJOR) + 1))))
	@echo "$(NEW_MAJOR).0" > $(VERSION_FILE)
	@echo "Version bumped to $(NEW_MAJOR).0"

bump_minor:
	@echo "Bumping minor version..."
	@$(eval VERSION=$(shell cat $(VERSION_FILE)))
	@$(eval MAJOR=$(shell echo $(VERSION) | cut -d. -f1))
	@$(eval MINOR=$(shell echo $(VERSION) | cut -d. -f2))
	@$(eval NEW_MINOR=$(shell echo $$(($(MINOR) + 1))))
	@echo "$(MAJOR).$(NEW_MINOR)" > $(VERSION_FILE)
	@echo "Version bumped to $(MAJOR).$(NEW_MINOR)"

bump_patch:
	@echo "Bumping patch version..."
	@$(eval VERSION=$(shell cat $(VERSION_FILE)))
	@$(eval MAJOR=$(shell echo $(VERSION) | cut -d. -f1))
	@$(eval MINOR=$(shell echo $(VERSION) | cut -d. -f2))
	@$(eval PATCH=$(shell git rev-list --count HEAD --perl-regexp --author='^((?!RPI WiFi Controller Auto-Build).*)$''))
	@echo "$(MAJOR).$(MINOR).$(PATCH)" > $(VERSION_FILE)
	@echo "Version bumped to $(MAJOR).$(MINOR).$(PATCH)"
