// Spike de diagnóstico: só sobe o USB pela stdio do SDK (CDC), sem TinyUSB
// configurado à mão. Serve para separar "USB da placa funciona" de
// "nossos descritores/sonda estão errados".

#include <stdio.h>

#include "pico/stdlib.h"

int main(void) {
    stdio_init_all();

    while (true) {
        printf("usb-hello vivo\n");
        sleep_ms(1000);
    }
}
