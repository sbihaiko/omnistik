#ifndef OMNISTIK_TUSB_CONFIG_H
#define OMNISTIK_TUSB_CONFIG_H

// Número de interfaces HID de gamepad que o dispositivo composto oferece.
// Definido no build (OMNISTIK_PAD_COUNT); 1 por padrão, que é o degrau da onda 1.
#ifndef OMNISTIK_PAD_COUNT
#define OMNISTIK_PAD_COUNT 1
#endif

#ifndef CFG_TUSB_MCU
#error "CFG_TUSB_MCU deve ser definido pelo pico-sdk"
#endif

#define CFG_TUSB_RHPORT0_MODE (OPT_MODE_DEVICE | OPT_MODE_FULL_SPEED)
#define CFG_TUSB_OS OPT_OS_PICO
#define CFG_TUD_ENDPOINT0_SIZE 64

// Uma interface HID por gamepad, todas no mesmo dispositivo.
#define CFG_TUD_HID OMNISTIK_PAD_COUNT
#define CFG_TUD_HID_EP_BUFSIZE 64

// O spike não usa áudio, CDC, MIDI nem MSC.
#define CFG_TUD_CDC 0
#define CFG_TUD_MSC 0
#define CFG_TUD_MIDI 0
#define CFG_TUD_AUDIO 0
#define CFG_TUD_VIDEO 0
#define CFG_TUD_VENDOR 0

#endif
