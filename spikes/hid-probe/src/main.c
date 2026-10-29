// Sonda de saída (spec #1, ticket #2): enumera como um dispositivo USB
// composto com OMNISTIK_PAD_COUNT interfaces HID de gamepad, alimentadas por
// input sintético. Não lê controle nenhum: o objetivo é isolar a saída.

#include <stdint.h>

#include "pico/stdlib.h"
#include "tusb.h"
#include "tusb_config.h"

#define REPORT_LEN 7  // 1 de Report ID + 2 de botões + 4 de eixos

// Sequência por interface: neutro -> pressiona -> solta -> pausa.
// Cada interface pressiona um botão diferente (bit = índice), para que o
// observador no host ou no console consiga dizer qual interface é qual.
typedef enum {
    PHASE_NEUTRAL = 0,
    PHASE_PRESS,
    PHASE_RELEASE,
    PHASE_PAUSE,
} phase_t;

static phase_t phase = PHASE_NEUTRAL;

static void build_report(uint16_t buttons, uint8_t out[REPORT_LEN]) {
    out[0] = 0x01;               // Report ID
    out[1] = (uint8_t)(buttons & 0xFF);
    out[2] = (uint8_t)(buttons >> 8);
    out[3] = 128;                // X
    out[4] = 128;                // Y
    out[5] = 128;                // Z
    out[6] = 128;                // Rz
}

static uint16_t buttons_for(uint8_t instance) {
    if (phase != PHASE_PRESS) return 0;
    return (uint16_t)(1u << instance);
}

int main(void) {
    tusb_init();

    uint32_t last_change = to_ms_since_boot(get_absolute_time());

    while (true) {
        tud_task();

        uint32_t now = to_ms_since_boot(get_absolute_time());
        if (now - last_change >= 500) {
            last_change = now;
            phase = (phase_t)((phase + 1) % 4);
        }

        for (uint8_t i = 0; i < OMNISTIK_PAD_COUNT; i++) {
            if (!tud_hid_n_ready(i)) continue;
            uint8_t rep[REPORT_LEN];
            build_report(buttons_for(i), rep);
            // Passa o payload sem o ID; o ID vai no argumento.
            tud_hid_n_report(i, rep[0], rep + 1, REPORT_LEN - 1);
        }

        // NÃO usar sleep_ms aqui: no RP2350 esse caminho (alarme + wfe) derruba
        // a enumeração USB. Medido em bancada com SDK 2.3.1 e GCC 14.3.
        busy_wait_us_32(1000);
    }
}
