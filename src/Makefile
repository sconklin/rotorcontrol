#
# This is a mess. I had to copy a bunch of the arduino library files here to make it work.
#

ARDUINO_DIR = /usr/share/arduino
ARDUINO_LIB_PATH = $(ARDUINO_DIR)/libraries
ARDUINO_CORE_PATH = $(ARDUINO_DIR)/hardware/arduino/cores/arduino
AVR_TOOLS_PATH = /usr/bin
TARGET = rotor
ARDUINO_LIBS = LiquidCrystal Wire
MCU          = atmega328p
F_CPU        = 16000000
ARDUINO_PORT = /dev/ttyUSB0

AVRDUDE_CONF = /etc/avrdude.conf

include /usr/local/share/Arduino.mk
