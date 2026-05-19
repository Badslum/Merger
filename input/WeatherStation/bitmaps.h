#include <stdint.h>

static const uint8_t ICON_TEMP[] PROGMEM = {
    0b00010000,
    0b00101000,
    0b00101000,
    0b00101000,
    0b00111000,
    0b01111100,
    0b01111100,
    0b00010000
};

static const uint8_t ICON_HUM[] PROGMEM = {
    0b11001100,
    0b00110011,
    0b00000000,
    0b11001100,
    0b00110011,
    0b00000000,
    0b11001100,
    0b00110011
};

static const uint8_t ICON_DP[] PROGMEM = {
    0b00010000,
    0b00111000,
    0b01111100,
    0b11111110,
    0b11111110,
    0b11111110,
    0b01111100,
    0b00111000
};

static const uint8_t ICON_PRES[] PROGMEM = {
    0b00111000,
    0b01000100,
    0b10000010,
    0b10111010,
    0b10000010,
    0b01000100,
    0b00111000,
    0b00000000
};