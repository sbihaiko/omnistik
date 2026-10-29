# Hardware do Omnistik — topologia, fiação e bancada

Documento de trabalho do protótipo V1 (PS5). Complementa o [README](../README.md); nada aqui foi validado em bancada ainda.

## Topologia

Uma porta host só, com hub alimentado, serve o dongle de autenticação e todos os controles de entrada.

```text
PS5  <---- cabo USB (micro-USB) ----  Pico 2 W  ---- host PIO-USB ----  hub USB alimentado
                                       (device)                        ├── dongle de auth
                                                                       ├── gamepad 1
                                                                       ├── gamepad 2
                                                                       └── receptor 2,4 GHz
```

O PS5 enxerga **um único controle**. O hub e o dongle ficam invisíveis para ele.

## Papéis USB do Pico 2 W

| Porta | Papel | Implementação |
|---|---|---|
| Nativa (micro-USB) | device | USB nativo do RP2350 |
| Host | host | `Pico-PIO-USB` em duas GPIO |

O RP2350 não faz host e device simultâneos pelo USB nativo; a porta do hub é feita por PIO.

> **Risco principal:** `Pico-PIO-USB` foi escrito para o RP2040. O suporte ao RP2350 precisa ser confirmado antes de qualquer montagem — se não rodar, a topologia toda muda.

## Fiação do host PIO-USB

```
GPIO 12 ──[27 Ω]── D+
GPIO 13 ──[27 Ω]── D-
VSYS ──[load switch, limite ~100 mA]── VBUS do conector
GND ── GND e shield
```

GPIO 12/13 por ficarem longe do conector USB nativo e do módulo RF. Duas GPIOs quaisquer servem.

| Ponto | Detalhe |
|---|---|
| Resistores de série | 27 Ω em D+ e D−, um por linha |
| VBUS | Via load switch (AP2553, TPS2051B) — nunca direto do VSYS |
| Capacitância | 100 µF + 100 nF no VBUS do conector |
| Par D+/D− | Curto e trançado, longe do módulo WiFi e do cristal |

## O hub

Requisitos:

- **Autoalimentado** (fonte externa). O Pico não alimenta os dispositivos.
- **Sem backfeed**: o hub não pode devolver 5 V na porta upstream. Muitos hubs baratos realimentam — verificar com multímetro antes de ligar no Pico.
- O Pico ainda precisa fornecer VBUS na upstream (é assim que o hub detecta a conexão), mas com corrente limitada.

Por que um hub só: economiza uma porta host inteira e o desenho da placa. O custo é que o dongle passa a disputar polling com os controles.

## Lista de material

| Peça | Especificação | Para quê |
|---|---|---|
| USB-A fêmea | 2× conector de painel/DIP | Host e reserva |
| Resistores 27 Ω | 2× | D+ / D− |
| Load switch | AP2553 ou TPS2051B | VBUS com limite |
| Capacitor | 100 µF + 100 nF | Hot-plug |
| Perfboard | ~4×6 cm | Montagem do Pico |
| Header 1×3 | GND / D+ / D− | Pontos de teste |
| Cabo micro-USB → USB-A | 1 | Pico → PS5 |
| Hub USB alimentado | Fonte externa, sem backfeed | Concentrador |
| Dongle de auth | Haute42/COSMOX Booter5 | Autenticação PS5 |

## Testes de bancada, nesta ordem

1. **Pico como device**: enumerar como controle HID no PC. Valida toolchain e USB nativo.
2. **Pico como host**: ler um mouse/teclado via PIO-USB. Valida PIO-USB no RP2350.
3. **Host + device simultâneos**: host lendo um dispositivo enquanto o Pico enumera no PC. É o teste decisivo da arquitetura.
4. **Hub + um gamepad**: hub alimentado na porta host, um controle lido.
5. **Hub + dongle**: o dongle aparece e responde atrás do hub.
6. **Hub + gamepad + dongle**: os dois ao mesmo tempo, com medição de latência.

Cada passo isola uma variável. Falha no 2 ou 3 muda o hardware; falha no 6 muda o software.

## Múltiplos jogadores

A entrada é resolvida por um único Pico: Bluepad32 recebe até 4 controles Bluetooth simultâneos. A saída é que limita — o PS5 atribui jogador por dispositivo USB, e o RP2350 tem um controlador USB.

Ordem de investigação:

1. **Dispositivo composto com 4 interfaces HID.** Se o PS5 registrar 4 jogadores, um Pico basta e não há hardware extra.
2. Se não registrar: **4 Picos**, um por jogador (mais simples), ou **1 cérebro + 4 bocas** (1 Pico com BT e 4 RP2040 Zero por SPI/UART, mais barato e mais complexo).

**Um Pico não pode fingir ser um hub.** Hub é peça elétrica: precisa de portas físicas e de dispositivos atrás delas. Um hub emulado não teria nada para distribuir, e o console continuaria vendo um jogador.

## Plano B

Se o hub atrapalhar o dongle ou o PIO-USB não lidar bem com splits:

- **Porta host dedicada para o dongle**, via `MAX3421E` (SPI) ou via um segundo MCU encarregado só do dongle.
- O hub continua servindo os controles de entrada.

A montagem com duas portas não perde nada do que foi feito nos testes 1–3.

## Pendências

- Confirmar suporte do `Pico-PIO-USB` ao RP2350.
- Confirmar comportamento de hub com `Pico-PIO-USB` (split transactions).
- Medir consumo e surto do dongle no hot-plug.
- Verificar backfeed do hub escolhido antes de conectar ao Pico.
