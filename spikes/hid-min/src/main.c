// HID mínimo: só inicializa o device e bombeia a pilha. Sem stdio, sem envio
// de relatório. Isola "o dispositivo composto enumera?" de qualquer outra coisa.

#include "pico/stdlib.h"
#include "tusb.h"
#include "tusb_config.h"

int main(void) {
    tusb_init();

    while (true) {
        tud_task();
    }
}
