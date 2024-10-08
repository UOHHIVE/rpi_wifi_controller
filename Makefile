
CC = clang
STD = c17

#ARCH = arm64
#ARCH = x86_64

CFLAGS_TEST = -std=$(STD) -Wall -Wextra -Werror -Wpedantic -Wno-unused-parameter -Wno-unused-variable -Wno-unused-function -Wno-unused-private-field
CFLAGS_PROD = -std=$(STD) -Wall -Wextra -O3
# CFLAGS_PROD = -arch $(ARCH) -std=$(STD) -Wall -Wextra -O3

DIR_SRC = ./src/
DIR_TARGET = ./target/

MAIN = main.c
TARGET = main.out

SRC = $(wildcard $(DIR_SRC)*.c)

#DATAFILES = $(wildcard $(DIR_SRC)*.txt) $(wildcard $(DIR_SRC)*.csv)

test:
	

build: 
	@echo "creating target directory"
	@if [ ! -d $(DIR_TARGET) ]; then mkdir $(DIR_TARGET); fi
	@echo "Building..."
	@echo "$(CC) $(CFLAGS) $(SRC) -o $(DIR_TARGET)$(TARGET)"
	@$(CC) $(CFLAGS) $(SRC) -o $(DIR_TARGET)$(TARGET)
	@echo "Build complete."
#	@echo "Copying data files..."
#	@cp $(DATAFILES) $(DIR_TARGET)

run: 
	@echo "Running..."
	@cd $(DIR_TARGET) && ./$(TARGET)

debug:
	@echo "Running in debug mode..."
	@cd $(DIR_TARGET) && AP_DEBUG=1 ./$(TARGET)

clean:
	@echo "Cleaning..."
	@rm -rf $(DIR_TARGET)