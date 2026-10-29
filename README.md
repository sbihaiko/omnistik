# Omnistik — O Joystick Universal

Atualizado em 29/10/2026. Síntese das decisões e hipóteses discutidas; não representa hardware ou firmware já validado.

## Objetivo
Criar um gateway que receba controles diferentes (USB, Bluetooth e controles 2,4 GHz com dongle) e os apresente a sistemas de destino. Primeiro alvo: **PlayStation 5**, entregando os controles como joysticks de PS5 por cabo USB, sem abrir ou modificar os controles de origem.

A intenção final é funcionar em um **PS5 bloqueado**. O **PS5 desbloqueado** é o instrumento de bancada: serve para observar e engenharia reversa do protocolo de autenticação de controle, livre da checagem que o console bloqueado impõe.

## Escopo V1
- Primeiro demonstrar **um gamepad de terceiros controlando o PS5**; depois expandir a dois jogadores.
- O gateway se enumera como controle de PS5 (DualSense/DualShock 4) via USB com fio.
- Entrada dos controles físicos por **hub USB alimentado**.
- Saída agregada ao PS5 por **hub USB** — um único cabo até o console.
- PS5 **desbloqueado**: a checagem de autenticação de controle de terceiros precisa estar neutralizada pelo payload/exploit em uso.
- Controles de origem mantidos intactos.

## Hardware considerado

![Placa Raspberry Pi Pico 2 W usada no protótipo](docs/images/hardware.png)

*Placa já em mãos para o protótipo: Raspberry Pi Pico 2 W, RP2350, `FCC: 2ABCB-PICO2W` / `IC: 20953-PICO2W`.*

- Raspberry Pi **Pico 2 W** (RP2350), candidato ao gateway.
- **Hub USB alimentado externamente** no lado de entrada, para gamepads USB e receptores 2,4 GHz.
- **Hub USB** no lado de saída, agregando os controles emulados em um cabo para o PS5.
- Interface USB Host adicional por PIO-USB, a validar na placa/firmware.
- PS5 **desbloqueado**: bancada de engenharia reversa, com a checagem de autenticação neutralizada.
- PS5 **bloqueado**: alvo final; recebe o mesmo gateway sem qualquer alteração.
- Eventual transceptor 2,4 GHz externo, **modelo ainda não escolhido**.
- Ferramentas úteis: multímetro e analisador lógico.

## Arquitetura desejada
```text
Controles USB / dongles 2,4 GHz / Bluetooth
                   |
                   v
        hub USB alimentado (entrada)
                   |
                   v
              MultiController
              (Pico 2 W)
                   |
     emulação de controle PS5 (USB HID)
                   |
          hub USB (saída)
                   |
            cabo único USB
                   |
           PS5 desbloqueado
```

**Importante:** o PS5 autentica controles de terceiros. No console desbloqueado essa checagem pode ser desativada, o que permite desenvolver a emulação; no **bloqueado** ela está ativa e passa a ser o obstáculo real do projeto. Por isso o PS5 desbloqueado é ferramenta de bancada, e não o alvo.

## Software livre candidato
- [OGX-Mini](https://github.com/wiredopposite/OGX-Mini): base candidata para receber/normalizar gamepads e produzir saída de controle PS4/PS5; verificar suporte efetivo ao Pico 2 W, quais modos de saída funcionam sem chip de autenticação, múltiplos jogadores e coexistência com USB Host.
- [Pico-PIO-USB](https://github.com/sekigon-gonnoc/Pico-PIO-USB): implementação USB via PIO a avaliar para entrada Host.

## Pesquisa e incógnitas do PS5
- **Engenharia reversa da autenticação**: como o console valida um controle USB, quais mensagens troca e o que um controle oficial responde — observado no PS5 desbloqueado.
- Se a autenticação pode ser reproduzida por hardware próprio, ou se depende de chave/segredo inacessível no chip Sony.
- Qual payload/exploit do PS5 desbloqueado remove a exigência e em quais versões de firmware, para comparar o comportamento com e sem a checagem.
- Definir se a emulação deve ser **DualSense** ou **DualShock 4** (diferenças de descritor, relatório e recursos aceitos pelo console).
- Verificar se dois controles emulados podem coexistir atrás de um hub USB e como o PS5 os enumera (P1/P2).
- Medir se a latência gateway + hub é aceitável para jogo.

## Experimentos propostos
1. Confirmar no PS5 desbloqueado que um controle USB sem autenticação é aceito.
2. Capturar no PS5 desbloqueado a troca de mensagens de um DualSense oficial, para mapear o protocolo de autenticação.
3. Fazer o Pico 2 W enumerar como DualSense e validar botões/analógicos em um teste de sistema.
4. Receber um gamepad de terceiros via hub USB alimentado e repassá-lo ao PS5 (P1).
5. Medir latência e reconexão ao desconectar/reconectar o controle de origem.
6. Adicionar um segundo controle atrás do hub de saída e verificar a enumeração como P1/P2.
7. Testar conflitos, ordem de enumeração e comportamento após suspensão do console.
8. Repetir o teste no **PS5 bloqueado** e medir o que a checagem de autenticação rejeita.

## Mapeamento mínimo PS5
| Entrada genérica | Saída DualSense |
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
- Um gamepad não PlayStation funciona no PS5 **desbloqueado**.
- Enumeração estável como controle de PS5, sem o console exigir autenticação.
- Protocolo de autenticação do PS5 documentado a partir da bancada desbloqueada.
- Pareamento/reconexão reproduzíveis.
- Latência adequada para jogar, medida em testes.
- Nenhuma modificação no console ou nos controles de origem.

## Evolução posterior
- **Aceitação por PS5 bloqueado**: o gateway passa a se apresentar como controle autêntico, sem depender de desbloqueio.
- Dois jogadores simultâneos.
- Alvo **Atari Pac-Man Edition** com os receptores DB9 dos CX40+ Wireless (fase posterior; ver histórico no git).
- Entradas de aproximadamente duas dezenas de controles diferentes.
- Saídas adicionais para Mac e Nintendo Switch 1 via USB.
- Perfis por controle e console, seleção automática e integração ao painel retrô PolyConsole.

## Decisões pendentes
- Viabilidade de reproduzir a autenticação de controle sem o chip Sony — define se o PS5 bloqueado é alcançável.
- Payload/exploit e versão de firmware do PS5 desbloqueado usado na bancada.
- DualSense ou DualShock 4 como identidade emulada.
- Capacidade real do Pico 2 W com USB Host, emulação USB e (se necessário) Bluetooth simultâneos.
- Quantidade de controles suportada atrás dos hubs.
- Estratégia de alimentação e proteção elétrica.

## Licença

[MIT](LICENSE) © 2026 Sandro Bihaiko
