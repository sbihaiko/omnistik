# Omnistik — MultiController: especificação e plano do protótipo

Atualizado em 29/10/2026. Síntese das decisões e hipóteses discutidas; não representa hardware ou firmware já validado.

## Objetivo
Criar um gateway que receba controles diferentes (USB, Bluetooth e controles 2,4 GHz com dongle) e os apresente a sistemas de destino. Primeiro alvo: **Atari Pac-Man Edition**, usando os receptores DB9 fornecidos com dois joysticks Atari CX40+ Wireless, sem abrir ou modificar os controles.

## Escopo V1
- Primeiro demonstrar **um controle de terceiros controlando o Atari físico**; depois expandir a dois jogadores.
- Manter os CX40+ originais intactos.
- Preferência: comunicação sem fio com os receptores DB9 originais.
- Alternativa de contingência: saída DB9 elétrica diretamente do gateway, caso a engenharia reversa de rádio não seja viável inicialmente.

## Hardware considerado
- Raspberry Pi **Pico 2 W** (RP2350), escolhido como candidato ao gateway.
- Hub USB **alimentado externamente** para gamepads USB e receptores 2,4 GHz.
- Interface USB Host adicional por PIO-USB, a validar na placa/firmware.
- Dois CX40+ Wireless e seus receptores USB e DB9.
- Eventual transceptor 2,4 GHz externo, **modelo ainda não escolhido**.
- Ferramentas úteis: multímetro e analisador lógico; para estudar RF, equipamento de captura compatível com o protocolo identificado.

## Arquitetura desejada
```text
Controles USB / dongles 2,4 GHz / Bluetooth
                   |
                   v
              MultiController
              (Pico 2 W)
                   |
        tradução de botões P1/P2
                   |
          rádio 2,4 GHz a definir
                   |
           receptores Atari DB9
                   |
             Atari físico
```

**Importante:** o receptor USB do CX40+ e o receptor DB9 são ambos receptores. Não basta conectá-los entre si: para usar o DB9 original, o gateway precisa transmitir um sinal RF compatível. O rádio Wi-Fi/Bluetooth do Pico 2 W não garante suporte ao protocolo proprietário Atari.

## Software livre candidato
- [OGX-Mini](https://github.com/wiredopposite/OGX-Mini): base candidata para receber/normalizar gamepads e produzir saídas USB; verificar suporte efetivo ao Pico 2 W, controles específicos, múltiplos jogadores e modos de saída antes de adotar.
- [Pico-PIO-USB](https://github.com/sekigon-gonnoc/Pico-PIO-USB): implementação USB via PIO a avaliar para entrada Host.

## Pesquisa e incógnitas de rádio
- Os CX40+ usam conexão proprietária anunciada como **2,4 GHz**, com receptores alternativos USB-A e DB9.
- Não foi identificado de forma confiável o chipset RF, a codificação de pacotes, canais, chaves ou procedimento interno de pareamento.
- O comportamento de **dois receptores pareados ao mesmo controle simultaneamente** é uma hipótese a testar, não um fato confirmado.
- Inspecionar receptores pode ajudar a identificar o chipset; **não é necessário abrir os controles**.

## Experimentos propostos
1. Parear CX40+ #1 com receptor USB; confirmar comandos no computador.
2. Parear o mesmo controle com receptor DB9 e testar no Atari.
3. Retornar ao receptor USB sem novo pareamento; verificar se o vínculo persiste.
4. Ligar ambos os receptores e verificar se respondem simultaneamente.
5. Examinar identificação USB (VID/PID, descritores e relatórios HID); isso não revela por si só o protocolo RF.
6. Se necessário, abrir **somente um receptor** e fotografar frente/verso e códigos dos circuitos integrados.
7. Identificar o transceptor e investigar viabilidade de transmitir pacotes compatíveis.
8. Demonstrar um gamepad de terceiros → gateway → receptor DB9 → Atari (P1).
9. Expandir para P2 e testar conflitos, latência e reconexão.

## Mapeamento mínimo Atari
| Entrada genérica | Saída Atari |
|---|---|
| Direcional cima | UP |
| Direcional baixo | DOWN |
| Direcional esquerda | LEFT |
| Direcional direita | RIGHT |
| Botão principal | FIRE |

## Critérios de sucesso do V1
- Um gamepad não Atari funciona no Atari físico.
- Os CX40+ permanecem intactos.
- Pareamento/reconexão reproduzíveis.
- Latência adequada para jogar, medida em testes.
- Sem alterações no Atari quando se usa o receptor DB9 original.

## Evolução posterior
- Dois jogadores simultâneos.
- Entradas de aproximadamente duas dezenas de controles diferentes.
- Saídas adicionais para Mac e Nintendo Switch 1 via USB.
- Perfis por controle e console, seleção automática e integração ao painel retrô PolyConsole.

## Decisões pendentes
- Identificação do chipset e protocolo RF.
- Se o pareamento permite dois receptores ativos ao mesmo tempo.
- Escolha de rádio externo e firmware.
- Capacidade real do Pico 2 W com USB Host, Bluetooth e duas saídas Atari simultâneas.
- Estratégia de alimentação e proteção elétrica.

## Licença

[MIT](LICENSE) © 2026 Sandro Bihaiko
