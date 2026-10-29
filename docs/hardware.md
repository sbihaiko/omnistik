# Hardware do Omnistik — topologia, fiação e bancada

Documento de trabalho do protótipo V1 (PS5). Complementa o [README](../README.md); nada aqui foi validado em bancada ainda.

## Topologia

Uma porta host só, com hub alimentado, serve todos os controles de entrada.

```text
PS5  <---- cabo USB (micro-USB) ----  Pico 2 W  ---- host PIO-USB ----  hub USB alimentado
                                       (device)                        ├── gamepad 1
                                                                       ├── gamepad 2
                                                                       └── receptor 2,4 GHz
```

O PS5 enxerga **o gateway**. O hub fica invisível para ele.

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

Por que um hub só: economiza uma porta host inteira e o desenho da placa. O custo é que os controles passam a disputar polling entre si.

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

## Testes de bancada, nesta ordem

1. **Pico como device**: enumerar como controle HID no PC. Valida toolchain e USB nativo.
2. **Pico como host**: ler um mouse/teclado via PIO-USB. Valida PIO-USB no RP2350.
3. **Host + device simultâneos**: host lendo um dispositivo enquanto o Pico enumera no PC. É o teste decisivo da arquitetura.
4. **Hub + um gamepad**: hub alimentado na porta host, um controle lido.
5. **Hub + dois gamepads**: os dois ao mesmo tempo, com medição de latência.
6. **Pico no PS5 desbloqueado**: com o payload ativo, verificar se o console aceita o dispositivo e quantos jogadores atribui.
7. **Payload de controle virtual**: quatro controles criados por `scePadVirtualDevice*`, medindo input e latência.

Cada passo isola uma variável. Falha no 2 ou 3 muda o hardware; falha no 4 ou 5 muda o software.

## Múltiplos jogadores

A entrada é resolvida por um único Pico: Bluepad32 recebe até 4 controles Bluetooth simultâneos. A saída é que limita — o PS5 atribui jogador por dispositivo USB, e o RP2350 tem um controlador USB.

Ordem de investigação:

1. **Dispositivo composto com 4 interfaces HID.** Se o PS5 registrar 4 jogadores, um Pico basta e não há hardware extra.
2. Se não registrar: **controles virtuais por payload** — um Pico entrega os controles e o payload do console cria até 4 dispositivos virtuais (`MAX_SLOTS 4` na referência).
3. Se nenhum dos dois: **1 cérebro + 4 bocas** (1 Pico com BT e 4 RP2040 Zero por SPI/UART) ou **4 Picos**, um por jogador.

**Um Pico não pode fingir ser um hub.** Hub é peça elétrica: precisa de portas físicas e de dispositivos atrás delas. Um hub emulado não teria nada para distribuir, e o console continuaria vendo um jogador.

## Plano B

Se o PIO-USB não lidar bem com splits do hub:

- **Porta host dedicada por `MAX3421E` (SPI)** ou por um segundo MCU, com o hub servindo só os controles de entrada.
- A montagem com duas portas não perde nada do que foi feito nos testes 1–3.

## Manifesto da sonda (spec #1)

Configuração exata do artefato medido na bancada de PC. Qualquer resultado de console só vale para um UF2 listado aqui.

| Item | Valor |
|---|---|
| Placa | `pico2_w` (RP2350 + CYW43) |
| pico-sdk | 2.3.1, commit `079c6f3` (`/Users/bihaiko/pico/pico-sdk`) |
| Toolchain | ARM GNU 14.3.rel1 (`arm-gnu-toolchain-14.3.rel1-darwin-arm64-arm-none-eabi`) |
| Build | CMake + Ninja, `-DOMNISTIK_PADS=1\|2\|4` |
| UF2 N=1 | `spikes/hid-probe/build-n1/hid_probe.uf2`, sha256 `d82af4535645540de6bf3c0ff9a59f0d8a5a128af3d970c381b674987a7eaa83` |
| UF2 N=2 | `spikes/hid-probe/build-n2/hid_probe.uf2`, sha256 `e33954140ae8d0351c84eb0bf71a63088b8407e0f8818906ca8fa6f27a1ebc6a` |
| UF2 N=4 | `spikes/hid-probe/build-n4/hid_probe.uf2`, sha256 `20f1ccfc039d82dc6a1c66a4816776cf6eea2a4aab593f49e6233c733eba8213` |
| Host da bancada | macOS (Darwin 25.6), arm64 (M1), backend `hidapi` via Python `hid` |
| Identidade USB | VID `0xCAFE`, PID `0x4001` |
| Flash | arrastar o UF2 para o volume montado em BOOTSEL |

O relatório de saída (Report ID 2, 1 byte) é o canal de controle de bancada: `0` neutro, `1` pressiona, `2` solta, `3` pausa, `4` volta ao ciclo automático. Cada interface obedece ao seu próprio comando — é o que o harness usa no modo `--control`.

## Achados de bancada

Resultados medidos, não hipóteses. Cada linha diz o que foi testado e como.

| Achado | Condição | Evidência |
|---|---|---|
| A sonda enumera como gamepad | Pico 2 W, pico-sdk 2.3.1, GCC 14.3, USB composto com 1 interface HID | macOS registra `IOHIDDevice` com VendorID `0xCAFE`, ProductID `0x4001`, UsagePage 1 / Usage 5 (Game Pad), descritor de relatório idêntico ao definido |
| `sleep_ms()` no laço principal derruba a enumeração | mesmas condições; única diferença entre as builds | Dispositivo não aparece no host. Trocado por `busy_wait_us_32()` e passa a enumerar. Isolado por A/B: `stdio_init_all()`, `tud_hid_n_report()` e `tud_hid_n_ready()` foram descartados um a um antes |
| N=1, 2 e 4 enumera como um dispositivo pai com N interfaces | variantes compiladas com `-DOMNISTIK_PADS` | `hid.enumerate` devolve N caminhos com o mesmo serial; o harness confirma os descritores idênticos e nenhum botão cruzado entre interfaces |
| Cada interface é controlável pelo seu relatório de saída | Report ID 2, modo `--control` do harness | `PRESS` na interface i produz só o bit i; `PAUSE` não muda o estado; `RELEASE`/`NEUTRAL` voltam a zero |
| O macOS não devolve os caminhos HID em ordem | mesma placa, leituras sucessivas | Numa leitura a interface 0 vem primeiro, na seguinte a 1. O harness ordena por `interface_number`; sem isso o teste de N=4 acusa botão cruzado que não existe no firmware |
| O PS5 desbloqueado enumera o dispositivo, mas não o torna jogador | Pico 2 W com N=4 e depois N=1 na porta USB do console (11.20, kstuff), com o klog aberto | Duas conexões independentes, `ugen1.3: <Omnistik, Omnistik Probe, OMNI-PROBE>(VID=0xcafe PID=0x4001) at usbus1` e `usb_hid0` anexado em ambas. Nenhuma linha de pad no log, nenhuma reação na tela, nenhum jogador atribuído. Desconexão limpa, sem erro do console |

Sobre o segundo achado: o caminho de sono do RP2350 usa alarme + `wfe`, que tem histórico de problema ([pico-sdk#2216](https://github.com/raspberrypi/pico-sdk/issues/2216), [micropython#18990](https://github.com/micropython/micropython/issues/18990)). A correção citada no changelog do SDK para `best_effort_wfe_or_timeout()` não se manifesta nesta combinação. Enquanto não houver versão confirmada, **nada de `sleep_ms`/`sleep_us` no laço do firmware**.

## Pendências

- Confirmar suporte do `Pico-PIO-USB` ao RP2350.
- Confirmar comportamento de hub com `Pico-PIO-USB` (split transactions).
- Verificar backfeed do hub escolhido antes de conectar ao Pico.
- Confirmar com o payload ativo quantos jogadores o console aceita por interface HID.
