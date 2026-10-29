# Omnistik — O Joystick Universal

Atualizado em 29/10/2026. Síntese das decisões e hipóteses discutidas; não representa hardware ou firmware já validado.

## Objetivo
Criar um gateway que receba controles diferentes (USB, Bluetooth e controles 2,4 GHz com dongle) e os apresente a sistemas de destino. Primeiro alvo: **PlayStation 5**, entregando os controles como joysticks de PS5 por cabo USB, sem abrir ou modificar os controles de origem.

A intenção final é funcionar em um **PS5 bloqueado**. O **PS5 desbloqueado** é o instrumento de bancada: serve para observar o handshake de autenticação, que será delegado a um dongle no caminho de produção.

## Escopo V1 — começa no console desbloqueado
O V1 é desenvolvido e validado no **PS5 desbloqueado**, onde a autenticação pode ser neutralizada por payload. Nessa fase o gateway ainda **não usa dongle**.

- Demonstrar **um gamepad de terceiros controlando o PS5**; depois expandir a dois jogadores.
- O gateway emula um controle comum (XInput/HID) via USB nativo.
- Entrada dos controles físicos por **hub USB alimentado**, ligado à porta host.
- Controles de origem mantidos intactos.
- O dongle de autenticação entra só na fase seguinte, quando o alvo passa a ser o console **bloqueado**.

## Duas trilhas

| Trilha | Console | Autenticação | Dongle |
|---|---|---|---|
| **Desenvolvimento (V1)** | desbloqueado | neutralizada por payload | não |
| **Produção (V2)** | bloqueado | resolvida pelo dongle | sim |

Separar as trilhas desacopla "o pipeline funciona?" de "a autenticação funciona?" — e permite começar hoje, com a placa que já está em mãos.

## Hardware considerado

![Placa Raspberry Pi Pico 2 W usada no protótipo](docs/images/hardware.png)

*Placa já em mãos para o protótipo: Raspberry Pi Pico 2 W, RP2350, `FCC: 2ABCB-PICO2W` / `IC: 20953-PICO2W`.*

- Raspberry Pi **Pico 2 W** (RP2350), candidato ao gateway.
- **Hub USB alimentado** (sem backfeed) compartilhando o dongle e os controles na mesma porta host.
- **Dongle de autenticação PS5** — só na fase 2 (console bloqueado). Candidato: Besavior P5General (todos os jogos). Alternativa barata: Haute42/COSMOX Booter5 (só luta/corrida).
- Interface USB Host por PIO-USB — a porta do hub. Detalhes de fiação e testes em [docs/hardware.md](docs/hardware.md).
- PS5 **bloqueado**: alvo final; recebe o gateway sem qualquer alteração.
- PS5 **desbloqueado**: bancada de engenharia reversa, com a checagem de autenticação neutralizada.
- Eventual transceptor 2,4 GHz externo, **modelo ainda não escolhido**.
- Ferramentas úteis: multímetro e analisador lógico.

## Arquitetura desejada
```text
Controles USB / dongles 2,4 GHz / Bluetooth
                   |
                   v
        hub USB alimentado (sem backfeed)
          |                  |
          |                  +--> dongle de autenticação
          |                       (fornece a credencial PS5)
          v
      porta host PIO-USB
                   |
                   v
            MultiController
              (Pico 2 W)
         emula controle XInput/HID
                   |
         USB nativo (device)
                   |
                   v
      cabo USB único
                   |
                   v
      PS5 bloqueado (alvo)
```

**Um hub só.** O dongle e os controles compartilham a mesma porta host; o PS5 enxerga apenas um controle. Fiação, lista de material e ordem de testes: [docs/hardware.md](docs/hardware.md).

## Estrutura do código

```text
src/
  core/            estado normalizado do controle — C puro, sem Pico SDK
  adapters/
    input.h        porta: de onde os controles chegam
    output.h       porta: onde os controles são apresentados
    synthetic/     fonte simulada (bancada, onda 1)
    ps5-pico/      saída USB composta que o PS5 enxerga
    bluepad32/     entrada Bluetooth (onda 5)
    pio-usb/       entrada USB host (trilha do hub)
  app/             composition root: init de placa, log e escolha da variante
test/              testes do que está em src/, rodando no host, sem placa
spikes/            experimentos de bancada, descartáveis, um alvo de build cada
```

Regras:

- **`core/` não conhece hardware.** Nada de Pico SDK, USB ou rádio — é o que permite testar no host.
- **`src/` é o código final; `test/` é onde ele é testado.** Enquanto a sonda for experimento, ela vive em `spikes/`; só sobe para `src/` o que sobreviver à medição.
- **`adapters/` tem as portas e as implementações.** Os arquivos `.h` na raiz de `adapters/` são os contratos; cada pasta filha é uma implementação concreta, com alvo de CMake próprio, que depende da porta e do Pico SDK.
- **`app/` é o único lugar que monta as peças** e sabe qual variante está sendo compilada (entrada simulada vs. Bluetooth, N controles).
- **`spikes/` é descartável.** Cada experimento é um firmware inteiro e independente; não entra no build do produto.

A pasta `platform/` foi considerada e descartada: quem toca o Pico SDK são as próprias implementações de adaptador, e uma camada por cima só criaria um segundo lugar para inicializar periférico.

## Saída para o PS5: dongle de autenticação (fase 2)
Esta seção vale apenas para o **console bloqueado**. No V1 (console desbloqueado) a autenticação é neutralizada por payload e o gateway fala direto com o PS5.

O PS5 autentica controles de terceiros e **reautentica a cada ~30 segundos**; sem autenticação válida o controle deixa de responder por volta dos 8 minutos. Em vez de reproduzir a criptografia, o gateway **empresta** a autenticação de um dongle dedicado — o mesmo arranjo usado por fightsticks com GP2040-CE, que exibem `[PS5:AS]` quando o dongle entra em ação.

### Duas classes de dongle

| Classe | Exemplos | Cobertura de jogos | Mecanismo |
|---|---|---|---|
| **PS4 "categorized"** | Haute42/COSMOX Booter5, Mayflash MagicBoots v1.1, GuileKeys P5Boots, Brook Wingman FGC (v1) | Apenas jogos que aceitam controle PS4 licenciado — mais comum em luta e corrida | Autentica como controle PS4 licenciado (chip `NXP7105`) |
| **Emula DualSense** | **Besavior P5General**, Besavior P5Mate Pro | **Todos os jogos de PS5** | Autentica como DualSense, ~1000 Hz |

Conversores da segunda classe — Brook Wingman P5 e FGC2 — resolvem o mesmo problema, mas trocando a arquitetura: o gateway passa a só emular XInput e o conversor faz o resto.

**Candidato do V1: Besavior P5General** (dongle, não conversor). O Booter5 fica como opção barata caso o alvo seja apenas jogo de luta ou corrida.

### Consequências assumidas
- O gateway precisa emular apenas um controle comum, e não um DualSense com sua criptografia.
- **Um dongle atende um controle.** Cada jogador adicional exige mais uma unidade — a solução para vários é física, com vários dongles no hub.
- O suporte a hub múltiplo é **experimental** no que existe hoje de referência, e há relatos de instabilidade.
- **Recursos do DualSense variam por dongle**: a classe PS4 não tem gatilhos adaptativos, giroscópio nem alto-falante; a classe DualSense costuma repassá-los.
- O **Brook Wingman FGC2 não serve** como dongle (arquitetura interna diferente); só o FGC v1 e os listados acima.
- Atualizações de sistema do PS5 podem quebrar o dongle — manutenção permanente do projeto.
- O caminho da classe DualSense depende de **chip clonado**: a credencial existe, mas não é reproduzível por engenharia reversa própria.

## Software livre candidato
- [GP2040-CE](https://github.com/OpenStickCommunity/GP2040-CE): referência do modo *auth passthrough* de PS5 e da integração com dongles; verificar suporte a RP2350 (o projeto nasceu em RP2040).
- [OGX-Mini](https://github.com/wiredopposite/OGX-Mini): base candidata para receber/normalizar gamepads de entrada.
- [Pico-PIO-USB](https://github.com/sekigon-gonnoc/Pico-PIO-USB): implementação USB via PIO, necessária para a porta host do hub.

## Pesquisa e incógnitas do PS5
- **Protocolo do dongle**: o que o gateway precisa enviar/receber na porta USB-A para que a credencial seja aplicada.
- Se o **Besavior P5General aceita um controle custom** como entrada, ou se só funciona com conversores e sticks conhecidos.
- Quantas interfaces USB o Pico 2 W sustenta ao mesmo tempo: host do hub e device para o PS5.
- Se o caminho do dongle é estável o bastante para jogo (latência, reconexão, suspensão do console).
- Quantos dongles cabem atrás de um hub sem comprometer o prazo de reautenticação de ~30 s.
- **Longo prazo**: como Brook e Besavior fazem a mesma coisa — indício de uso de **chips de DualSense clonados**, o que explicaria a viabilidade comercial e não ser alcançável por engenharia reversa pura.
- Na bancada desbloqueada: capturar o handshake para documentar o protocolo, mesmo que a V1 não o implemente.

## Achados de bancada

Medições já feitas. O detalhe e a evidência de cada uma ficam em [docs/hardware.md](docs/hardware.md).

- **O Pico 2 W enumera como gamepad.** Dispositivo USB composto com uma interface HID; o macOS o registra como Game Pad (UsagePage 1 / Usage 5), com o descritor de relatório previsto.
- **`sleep_ms()` no laço principal impede a enumeração no RP2350.** Com pico-sdk 2.3.1 e GCC 14.3, o dispositivo simplesmente não aparece no host; trocando por `busy_wait_us_32()` ele enumera. O caminho de sono do RP2350 (alarme + `wfe`) tem histórico de problema, e a correção citada no SDK não se aplica a esta combinação. **Regra do projeto: nada de `sleep_ms`/`sleep_us` no laço do firmware** até uma versão confirmada.

## Experimentos — fase 1 (console desbloqueado, sem dongle)
1. **Pico como device**: enumerar como controle HID no PC.
2. **Pico como host**: ler um gamepad via PIO-USB.
3. **Host + device simultâneos**: ler o gamepad enquanto enumera no PC.
4. **PS5 desbloqueado aceita o Pico** como controle e responde ao input.
5. **Hub alimentado** com um gamepad, lido pelo Pico e repassado ao PS5 (P1).
6. **Latência e reconexão** ao desconectar/reconectar o controle de origem.
7. **Segundo controle** e enumeração P1/P2.
8. **Dispositivo composto com 4 interfaces HID**: verificar se o PS5 registra 4 jogadores. Se sim, um só Pico atende todos.
9. **Conexão Bluetooth**: parear 4 controles no Pico com Bluepad32 e medir consumo, latência e estabilidade da pilha.

## Experimentos — fase 2 (console bloqueado, com dongle)
1. Confirmar no PS5 **bloqueado** que um controle sem autenticação é rejeitado — medir a janela real.
2. Validar o dongle com um controle já suportado, em jogo que **não** aceita controle PS4 legado.
3. Repetir a fase 1 com o dongle no caminho.
4. Deixar um controle parado enquanto o outro joga por 20 minutos e medir a reautenticação de ~30 s.
5. Verificar quantos dongles o hub suporta sem violar o prazo de reautenticação.

## Quatro jogadores com um Pico
A entrada não é o problema: Bluepad32 recebe até **4 controles Bluetooth** simultâneos no Pico 2 W, com a lista completa (DS4, DualSense, Switch Pro, Xbox). O gargalo é a saída — o PS5 atribui jogador **por dispositivo USB**, e o RP2350 tem um único controlador USB.

**Teste decisivo:** o Pico enumerar como **dispositivo composto com 4 interfaces HID de gamepad**. É barato e define toda a arquitetura.

| Resultado | Consequência |
|---|---|
| O PS5 registra 4 jogadores | **Um Pico resolve o objetivo inteiro** |
| O PS5 registra 1 jogador | São necessários 4 dispositivos USB |

Se falhar, as formas viáveis são:

| Formato | Custo aprox. | Como |
|---|---|---|
| **4 Picos**, um por jogador | R$ 240 | Cada placa faz 1 BT + 1 USB; simples e robusto |
| **1 cérebro + 4 bocas** | R$ 160 | Um Pico recebe os 4 BT e alimenta 4 RP2040 Zero por SPI/UART, cada um virando um joystick |
| 1 Pico + payload no console | R$ 0 | Patchear a camada de input (ScePad); **não transfere** para o console bloqueado |

**Regra do projeto:** nada que dependa de payload entra no produto. Payload é conveniência de bancada.

## Mapeamento mínimo PS5
| Entrada genérica | Saída (controle emulado) |
|---|---|
| Direcional | D-pad |
| Analógico esquerdo | Left Stick |
| Analógico direito | Right Stick |
| Botão principal | Cross |
| Botões secundários | Circle / Square / Triangle |
| Botões de ombro | L1 / R1 |
| Gatilhos | L2 / R2 |
| Cliques dos analógicos | L3 / R3 |
| Menu / pausa | Options / Create |

## Critérios de sucesso do V1
- Um gamepad não PlayStation funciona em um **jogo de PS5** no console **desbloqueado**, **sem dongle**.
- Pipeline estável: enumeração, input e reconexão reproduzíveis.
- Latência adequada para jogar, medida em testes.
- Nenhuma modificação nos controles de origem.
- Base pronta para a fase 2, com o dongle como único acréscimo.

## Evolução posterior
- **Autenticação própria**: substituir o dongle por um caminho de auth embutido no gateway (chip/dongle integrado), eliminando a dependência de acessório externo.
- Dois jogadores simultâneos.
- Alvo **Atari Pac-Man Edition** com os receptores DB9 dos CX40+ Wireless (fase posterior; ver histórico no git).
- Entradas de aproximadamente duas dezenas de controles diferentes.
- Saídas adicionais para Mac e Nintendo Switch 1 via USB.
- Perfis por controle e console, seleção automática e integração ao painel retrô PolyConsole.

## Decisões pendentes
- **Firmware e payload do console desbloqueado** — fixar versão como bancada oficial do projeto.
- Qual dongle adotar na fase 2 e confirmar se um atende só um controle.
- Protocolo entre gateway e dongle: implementar em firmware próprio ou adotar GP2040-CE como base.
- Viabilidade de autenticação própria no futuro — define se o dongle é etapa final ou transitória.
- Capacidade real do Pico 2 W com host USB (PIO) e device simultâneos.
- Quantidade de controles suportada atrás do hub de entrada.
- Estratégia de alimentação e proteção elétrica.

## Licença

[MIT](LICENSE) © 2026 Sandro Bihaiko
