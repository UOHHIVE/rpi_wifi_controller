
CC = clang++
STD = c17

#ARCH = arm64
#ARCH = x86_64

CFLAGS_TEST = -Wall -Wextra -l wiringPi # -Werror -Wpedantic -Wno-unused-parameter -Wno-unused-variable -Wno-unused-function -Wno-unused-private-field -l wiringPi
CFLAGS_PROD = -Wall -Wextra -O3 -l wiringPi
# CFLAGS_PROD = -arch $(ARCH) -std=$(STD) -Wall -Wextra -O3

DIR_SRC = ./src/
DIR_TARGET = ./target/
DIR_SCRIPTS = ./scripts/

MAIN = main.cpp
TARGET = main.out

THIS_FILE := $(lastword $(MAKEFILE_LIST))
SRC = $(wildcard $(DIR_SRC)*.cpp)

#DATAFILES = $(wildcard $(DIR_SRC)*.txt) $(wildcard $(DIR_SRC)*.csv)

setup:
	@echo "creating target directory"
	@if [ ! -d $(DIR_TARGET) ]; then mkdir $(DIR_TARGET); fi

test:
	@$(MAKE) -f $(THIS_FILE) setup
	@echo "$(CC) $(CFLAGS_TEST) $(SRC) -o $(DIR_TARGET)$(TARGET)"
	@$(CC) $(CFLAGS_TEST) $(SRC) -o $(DIR_TARGET)$(TARGET)
	@echo "Build complete."
	@for file in *.out *.sh; do if [ -e "$file" ]; then chmod +x "$file"; fi; done

build: 
	@$(MAKE) -f $(THIS_FILE) setup
	@echo "Building..."
	@echo "$(CC) $(CFLAGS_PROD) $(SRC) -o $(DIR_TARGET)$(TARGET)"
	@$(CC) $(CFLAGS_PROD) $(SRC) -o $(DIR_TARGET)$(TARGET)
	@echo "Build complete."
	@echo "Copying scripts..."
	@cp $(DIR_SCRIPTS)/* $(DIR_TARGET)
	@for file in *.out *.sh; do if [ -e "$file" ]; then chmod +x "$file"; fi; done

run: 
	@$(MAKE) -f $(THIS_FILE) setup
	@if [ ! -d $(DIR_TARGET)/$(TARGET) ]; then $(MAKE) -f $(THIS_FILE) test ; fi
	@echo "Running..."
	@cd $(DIR_TARGET) && ./$(TARGET)

debug:
	@if [ ! -d $(DIR_TARGET)/$(TARGET) ]; then $(MAKE) -f $(THIS_FILE) test ; fi
	@echo "Running in debug mode..."
	@cd $(DIR_TARGET) && AP_DEBUG=1 ./$(TARGET)

clean:
	@echo "Cleaning..."
	@rm -rf $(DIR_TARGET)
	@$(MAKE) -f $(THIS_FILE) setup