#ifndef OMNISTIK_ADAPTERS_OUTPUT_H
#define OMNISTIK_ADAPTERS_OUTPUT_H

#include <stdint.h>

#include "core/pad.h"

// Porta de saída: onde os controles são apresentados.
// Implementações: ps5-pico/ (device USB composto que o PS5 enxerga), pc-hid/ (bancada).
// O número de controles é escolha de build da implementação, não da porta.
// Contrato provisório: nasceu com a estrutura, antes da primeira implementação.

typedef struct {
    // Prepara a saída com `pads` controles. Devolve quantos conseguiu oferecer.
    int (*start)(uint8_t pads);

    // Entrega o estado do controle `index`. 0 = ok, negativo = ocupado/descartado.
    int (*send)(uint8_t index, const pad_state_t *state);

    // Quantos controles a saída está oferecendo agora.
    uint8_t (*count)(void);
} output_port_t;

#endif
