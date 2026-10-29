#ifndef OMNISTIK_CORE_PAD_H
#define OMNISTIK_CORE_PAD_H

#include <stdint.h>

// Estado normalizado de um controle: não sabe de onde o input veio nem
// para onde vai. C puro, sem Pico SDK, testável no host.

typedef enum {
    PAD_BTN_SOUTH = 0,   // Cross / A
    PAD_BTN_EAST,        // Circle / B
    PAD_BTN_NORTH,       // Triangle / Y
    PAD_BTN_WEST,        // Square / X
    PAD_BTN_L1,
    PAD_BTN_R1,
    PAD_BTN_L2,
    PAD_BTN_R2,
    PAD_BTN_L3,
    PAD_BTN_R3,
    PAD_BTN_SELECT,
    PAD_BTN_START,
    PAD_BTN_UP,
    PAD_BTN_DOWN,
    PAD_BTN_LEFT,
    PAD_BTN_RIGHT,
    PAD_BTN_HOME,
    PAD_BTN_COUNT
} pad_button_t;

typedef struct {
    uint32_t buttons;        // bit 1 << pad_button_t
    uint8_t lx, ly;          // analógico esquerdo, 0..255, 128 = centro
    uint8_t rx, ry;          // analógico direito, 0..255, 128 = centro
} pad_state_t;

#endif
