// Bissecção: só tud_task + tud_hid_n_ready. Se enumerar, a chamada ready é
// inocente e o suspeito passa a ser sleep_ms / time.

#include "pico/stdlib.h"
#include "tusb.h"
#include "tusb_config.h"

int main(void) {
    tusb_init();

    while (true) {
        tud_task();
        (void)tud_hid_n_ready(0);
        sleep_ms(10);
    }
}
