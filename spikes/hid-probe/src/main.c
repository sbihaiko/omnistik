// Sonda de saída (spec #1, tickets #2 e #5): enumera como um dispositivo USB
// composto com OMNISTIK_PAD_COUNT interfaces HID de gamepad, alimentadas por
// input sintético. Não lê controle nenhum: o objetivo é isolar a saída.
//
// Cada interface aceita um comando pelo seu relatório de saída (Report ID 2),
// para que o host controle a sequência em vez de adivinhar pelo relógio.

#include <stdint.h>

#include "pico/stdlib.h"
#include "tusb.h"
#include "tusb_config.h"

#define REPORT_LEN 7   // 1 de Report ID + 2 de botões + 4 de eixos
#define CMD_REPORT_ID 2

// Comandos aceitos no relatório de saída, um por interface.
typedef enum {
    CMD_NEUTRAL = 0,  // solta o botão da interface
    CMD_PRESS = 1,    // pressiona o botão da interface
    CMD_RELEASE = 2,  // solta (explícito, para diferenciar de neutro no log)
    CMD_PAUSE = 3,    // não muda nada
    CMD_AUTO = 4,     // volta ao ciclo automático
} command_t;

// Estado por interface. O padrão é automático, para que a bancada de PC da
// onda 1 funcione sem enviar comando nenhum.
static volatile bool auto_mode[OMNISTIK_PAD_COUNT];
static volatile bool pressed[OMNISTIK_PAD_COUNT];

// Sequência automática: neutro -> pressiona -> solta -> pausa.
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

static bool button_down(uint8_t instance) {
    if (auto_mode[instance]) return phase == PHASE_PRESS;
    return pressed[instance];
}

static uint16_t buttons_for(uint8_t instance) {
    return button_down(instance) ? (uint16_t)(1u << instance) : 0;
}

static void apply_command(uint8_t instance, uint8_t cmd) {
    switch (cmd) {
        case CMD_PRESS:
            auto_mode[instance] = false;
            pressed[instance] = true;
            break;
        case CMD_RELEASE:
        case CMD_NEUTRAL:
            auto_mode[instance] = false;
            pressed[instance] = false;
            break;
        case CMD_PAUSE:
            auto_mode[instance] = false;
            break;
        case CMD_AUTO:
            auto_mode[instance] = true;
            break;
        default:
            break;  // comando desconhecido é ignorado
    }
}

int main(void) {
    for (uint8_t i = 0; i < OMNISTIK_PAD_COUNT; i++) {
        auto_mode[i] = true;
        pressed[i] = false;
    }

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

// ---------------------------------------------------------------------------
// Callbacks HID: cada interface recebe o seu comando separadamente.
// ---------------------------------------------------------------------------
void tud_hid_set_report_cb(uint8_t instance, uint8_t report_id,
                           hid_report_type_t report_type, uint8_t const *buffer,
                           uint16_t bufsize) {
    if (report_type != HID_REPORT_TYPE_OUTPUT) return;
    if (report_id != CMD_REPORT_ID) return;
    if (bufsize < 1) return;
    if (instance >= OMNISTIK_PAD_COUNT) return;

    apply_command(instance, buffer[0]);
}

uint16_t tud_hid_get_report_cb(uint8_t instance, uint8_t report_id,
                               hid_report_type_t report_type, uint8_t *buffer,
                               uint16_t reqlen) {
    (void)instance;
    (void)report_id;
    (void)report_type;
    (void)buffer;
    (void)reqlen;
    return 0;  // a sonda não responde GET_REPORT
}
