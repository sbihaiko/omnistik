#ifndef OMNISTIK_ADAPTERS_INPUT_H
#define OMNISTIK_ADAPTERS_INPUT_H

#include <stdint.h>

#include "core/pad.h"

// Porta de entrada: de onde os controles chegam.
// Implementações: synthetic/ (onda 1), bluepad32/ (onda 5), pio-usb/ (trilha do hub).
// Contrato provisório: nasceu com a estrutura, antes da primeira implementação.

typedef struct {
    // Prepara a fonte e devolve quantos controles ela espera entregar (>= 0).
    int (*start)(void);

    // Lê a fonte e atualiza o estado interno. 0 = ok.
    int (*poll)(void);

    // Estado normalizado do controle `index`. 0 = ok.
    int (*pad_state)(uint8_t index, pad_state_t *out);

    // Quantos controles a fonte tem hoje.
    uint8_t (*count)(void);
} input_port_t;

#endif
