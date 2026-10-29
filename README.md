# Omnistik — O Joystick Universal

Atualizado em 29/10/2026. Síntese das decisões e hipóteses discutidas; não representa hardware ou firmware já validado.

## Objetivo
Criar um gateway que receba controles diferentes (USB, Bluetooth e controles 2,4 GHz com dongle) e os apresente a sistemas de destino. Primeiro alvo: **PlayStation 5**, entregando os controles como joysticks de PS5 por cabo USB, sem abrir ou modificar os controles de origem.

A intenção final é funcionar em um **PS5 bloqueado**. O **PS5 desbloqueado** é o instrumento de bancada: serve para observar o handshake de autenticação, que será delegado a um dongle no caminho de produção.

## Escopo V1
- Primeiro demonstrar **um gamepad de terceiros controlando o PS5**; depois expandir a dois jogadores.
- O gateway **não emula a autenticação**: ele emula um controle comum (XInput/HID) e delega a credencial a um **dongle de autenticação** ligado à sua própria porta USB-A.
- Entrada dos controles físicos e dongle de autenticação por um **único hub USB alimentado**, ligado à porta host do gateway.
- Saída por um único cabo USB até o console.
- Controles de origem mantidos intactos.
- Validar preferencialmente já no **PS5 bloqueado**, que é o alvo real.

## Hardware considerado

![Placa Raspberry Pi Pico 2 W usada no protótipo](docs/images/hardware.png)

*Placa já em mãos para o protótipo: Raspberry Pi Pico 2 W, RP2350, `FCC: 2ABCB-PICO2W` / `IC: 20953-PICO2W`.*

- Raspberry Pi **Pico 2 W** (RP2350), candidato ao gateway.
- **Hub USB alimentado** (sem backfeed) compartilhando o dongle e os controles na mesma porta host.
- **Dongle de autenticação PS5**, ligado ao hub. Candidatos: Haute42/COSMOX Booter5, Mayflash MagicBoots v1.1, GuileKeys P5Boots, Brook Wingman FGC (v1).
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

## Saída para o PS5: dongle de autenticação
O PS5 autentica controles de terceiros e desconecta o que não passa no handshake (janela de cerca de 8 minutos). Em vez de reproduzir a criptografia, o gateway **empresta** a autenticação de um dongle dedicado — o mesmo arranjo já usado por fightsticks com GP2040-CE, que exibem `[PS5:AS]` quando o dongle entra em ação.

Consequências assumidas:
- O gateway passa a precisar emular apenas um controle comum, e não um DualSense com sua criptografia.
- O dongle é o único componente que conhece o protocolo de autenticação; trocá-lo não afeta o gateway.
- **Recursos do DualSense não existem**: sem gatilhos adaptativos, giroscópio ou alto-falante. O jogo aceita o controle; recursos que dependem do hardware oficial simplesmente não aparecem.
- O **Brook Wingman FGC2 não serve** como dongle (arquitetura interna diferente); só o FGC v1 e os dongles listados acima.
- Atualizações de sistema do PS5 podem quebrar o dongle — isso vira manutenção permanente do projeto.

## Software livre candidato
- [GP2040-CE](https://github.com/OpenStickCommunity/GP2040-CE): referência do modo *auth passthrough* de PS5 e da integração com dongles; verificar suporte a RP2350 (o projeto nasceu em RP2040).
- [OGX-Mini](https://github.com/wiredopposite/OGX-Mini): base candidata para receber/normalizar gamepads de entrada.
- [Pico-PIO-USB](https://github.com/sekigon-gonnoc/Pico-PIO-USB): implementação USB via PIO, necessária para as duas portas Host (hub de entrada e dongle).

## Pesquisa e incógnitas do PS5
- **Protocolo do dongle**: o que o gateway precisa enviar/receber na porta USB-A para que a credencial seja aplicada.
- Se **um dongle atende um controle ou vários** — define o custo e a viabilidade de dois jogadores.
- Quantas interfaces USB o Pico 2 W sustenta ao mesmo tempo: host do hub de entrada, host do dongle e device para o PS5.
- Se o caminho do dongle é estável o bastante para jogo (latência, reconexão, suspensão do console).
- **Longo prazo**: como Brook e Besavior fazem a mesma coisa — indício de uso de **chips de DualSense clonados**, o que explicaria a viabilidade comercial e não ser alcançável por engenharia reversa pura.
- Na bancada desbloqueada: capturar o handshake para documentar o protocolo, mesmo que a V1 não o implemente.

## Experimentos propostos
1. Confirmar no PS5 **bloqueado** que um controle sem autenticação é rejeitado (ou desconectado) — medir a janela real.
2. Repetir no PS5 **desbloqueado** e comparar, registrando a diferença de comportamento.
3. Validar um dongle de autenticação com um controle simples já suportado (referência externa), confirmando que entra em jogo nativo de PS5.
4. Fazer o Pico 2 W emular um controle comum e ser aceito pelo PS5 através do dongle.
5. Receber um gamepad de terceiros via hub USB alimentado e repassá-lo ao PS5 (P1).
6. Medir latência e reconexão ao desconectar/reconectar o controle de origem.
7. Adicionar um segundo controle e verificar enumeração P1/P2 e necessidade de um segundo dongle.
8. Testar conflitos, ordem de enumeração e comportamento após suspensão do console.

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
- Um gamepad não PlayStation funciona em um **jogo nativo de PS5** no console **bloqueado**.
- Autenticação resolvida pelo dongle, sem modificação no console.
- Pareamento/reconexão reproduzíveis.
- Latência adequada para jogar, medida em testes.
- Nenhuma modificação nos controles de origem.
- Comportamento na bancada desbloqueada documentado (handshake observado).

## Evolução posterior
- **Autenticação própria**: substituir o dongle por um caminho de auth embutido no gateway (chip/dongle integrado), eliminando a dependência de acessório externo.
- Dois jogadores simultâneos.
- Alvo **Atari Pac-Man Edition** com os receptores DB9 dos CX40+ Wireless (fase posterior; ver histórico no git).
- Entradas de aproximadamente duas dezenas de controles diferentes.
- Saídas adicionais para Mac e Nintendo Switch 1 via USB.
- Perfis por controle e console, seleção automática e integração ao painel retrô PolyConsole.

## Decisões pendentes
- Qual dongle adotar e se um atende mais de um controle.
- Protocolo entre gateway e dongle: implementar em firmware próprio ou adotar GP2040-CE como base.
- Viabilidade de autenticação própria no futuro — define se o dongle é etapa final ou transitória.
- Capacidade real do Pico 2 W com dois hosts USB (PIO) e device simultâneos.
- Quantidade de controles suportada atrás do hub de entrada.
- Estratégia de alimentação e proteção elétrica.

## Licença

[MIT](LICENSE) © 2026 Sandro Bihaiko
