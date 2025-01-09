
CC = g++
STD = c++23

#ARCH = arm64
#ARCH = x86_64

CFLAGS_TEST = -std=$(STD) -Wall -Wextra -Wpedantic -l wiringPi # -Werror -Wpedantic -Wno-unused-parameter -Wno-unused-variable -Wno-unused-function -Wno-unused-private-field -l wiringPi
CFLAGS_PROD = -std=$(STD) -flto -Wall -Wextra -O3 -l wiringPi
# CFLAGS_PROD = -arch $(ARCH) -std=$(STD) -Wall -Wextra -O3

DIR_SRC = ./src/
DIR_TARGET = ./target/
DIR_SCRIPTS = ./scripts/

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
	@$(CC) $(SRC) -o $(DIR_TARGET)$(TARGET) $(CFLAGS_TEST)
	@echo "Build complete."


build: 
	@$(MAKE) -f $(THIS_FILE) setup
	@echo "Building..."
	@echo "$(CC) $(CFLAGS_PROD) $(SRC) -o $(DIR_TARGET)$(TARGET)"
	@$(CC) $(SRC) -o $(DIR_TARGET)$(TARGET) $(CFLAGS_TEST)
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
