# Referência

Artefatos lidos de dispositivos reais, guardados para não depender de terceiros nem de memória.

## `dualsense-usb-descriptor.hex`

Descritor de relatório HID do DualSense (USB, `0x054C:0x0CE6`), 273 bytes, lido da interface 3 de um controle original com `hid.get_report_descriptor()` em 29/10/2026. Mesmo tamanho do publicado em [nondebug/dualsense](https://github.com/nondebug/dualsense).

O que ele mostra, e que importa para o projeto:

| Report ID | Direção | Tamanho | Papel |
|---|---|---|---|
| `0x01` | entrada | 64 B | 6 eixos, botões e o bloco vendor-defined por onde passam sensores e toque |
| `0x02` | saída | 47 B | LEDs, rumble, gatilhos adaptativos |
| `0x05`, `0x08`–`0x0A`, `0x20`–`0x22` | feature | vários | calibração, versão de firmware, número de série, capacidades |
| `0x80`–`0x85`, `0xA0`, `0xE0`, `0xF0`–`0xF5` | feature | 63 B, 15 B, 9 B, 2 B, 1 B | **autenticação** — o desafio `0xF0` e as respostas da família `0xF2` |

A presença dos relatórios de feature `0xF0`–`0xF5` no descritor de um controle original é a evidência de que a autenticação é um protocolo do próprio dispositivo, não uma checagem opcional do console. Não existe caminho para satisfazê-la sem a credencial.
