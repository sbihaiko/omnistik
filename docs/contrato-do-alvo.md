# Contrato do alvo: o que o PS5 desbloqueado aceita

Levantamento da issue #3 da spec #1. Registra o que já foi **medido** no console da bancada e o que ainda depende de ensaio. Atualizado em 29/10/2026.

## Console da bancada

| Item | Valor |
|---|---|
| Modelo | CFI-2014 |
| Firmware | 11.20 (único alvo do projeto; não há suporte a outra versão) |
| Entrada do exploit | **WebKit customizado**: app `PINK00001` com o WebKit Autoloader installer v0.7.1 — sha256 `48a1e68959d00b98…` |
| Natureza | exploit **tethered**: refeito a cada boot, sem persistência |
| Kernel | `kstuff-lite` 1.09 — sha256 `ec5212794dc6e44e…` |
| Mounter | ShadowMountPlus 1.6beta16 — sha256 `a35246fb3bb6042b…` |
| Transferência | ftpsrv v0.21.1 — sha256 `7d4b31c83eae4e05…` |
| Shell do agente | Ariel `ariel-ps5.elf`, build de `dev` em `32159d4` |
| Ordem de carga | `autoload.txt` em `/mnt/usb0/ps5_autoloader/`: `kstuff.elf` → 2 s → `shadowmountplus.elf` → 1 s → `ftpsrv.elf` → `ariel.elf` |

Fonte: projeto irmão [`ariel`](https://github.com/sbihaiko/ariel), `docs/ps5-scenarios.md` (inventário de 28/10/2026) e `docs/console-setup.md`, no mesmo console.

**Não há etaHEN neste console.** A cadeia é o **WebKit customizado** (app `PINK00001`) + kstuff-lite + ShadowMountPlus + ftpsrv + Ariel. O exploit entra pela app instalada, não pelo navegador do sistema apontando DNS para um servidor público. Isso importa para a #11: um spike de controle virtual precisa ser um ELF próprio do ps5-payload-sdk, carregado por `elfldr` na porta 9021 — não um plugin de etaHEN.

**Risco operacional registrado no `ariel`:** esse console já perdeu o armazenamento interno quatro vezes, todas com carga de escrita em andamento, e travou no estágio 2 do autoloader (o próprio exploit de kernel) em três tentativas seguidas. Um spike que trave o console derruba a cadeia inteira e exige reboot manual, então todo ensaio do Omnistik precisa ser curto, com log fora do console e sem escrita paralela.


## Três coisas separadas

A pergunta "o console aceita o Pico?" mistura três checagens que falham com códigos diferentes:

| Checagem | O que é | Onde é decidida |
|---|---|---|
| **Autenticação** | o controle prova que é um DualSense legítimo | desafio de ~30 s; sem resposta o controle morre em ~8 min |
| **Reconhecimento do dispositivo** | o console aceita o perfil USB declarado | enumeração e binding de driver |
| **Vínculo controle ↔ usuário** | o controle é atribuído a um jogador | conta logada no console + slot de jogador |

Desbloquear a autenticação **não** implica aceitar um gamepad HID genérico nem atribuir jogador. Cada uma das três precisa da sua própria evidência.

## Perfil USB aceito: bloqueio experimental declarado

Não existe fonte pública que diga qual perfil de gamepad o PS5 11.20 desbloqueado aceita — nem HID genérico, nem DS4, nem DualSense. A documentação da cena descreve o desafio de autenticação e os modos de saída com dongle, mas não o que o kernel faz depois que `kstuff` remove a checagem.

Declarado como **bloqueio experimental**: qualquer resultado negativo das ondas 2 e 3 vale apenas para a combinação exata de firmware + cadeia de payload + descritores testada, e não como prova de que o console recusa o dispositivo. O perfil só será conhecido ligando.

## O que já foi medido: quem decide o que vira controle

O klog do console mostra um serviço dedicado a isso, o `SceHidConfigService`:

```
[SceHidConfigService] DownloadFile : [RevokeList]
[SceHidConfigService] SERVER_URL={https://envelope2.np.dl.playstation.net/envelope2/hid_config/np/v00/hid_config.env}
[SceHidConfigService] PERIODIC_CHECK_INTERVAL_SEC={420}
[SceHidConfigService] ... Get: HCS_ISOK::0x8041013d
```

Ou seja: o mapeamento dispositivo → controle é mediado por **configuração baixada da Sony** (`hid_config.env`) com uma **RevokeList**, reconsultada a cada 420 s, mais o que vier embutido no firmware. Neste console o download falha (`0x8041013d`), então ele opera com a configuração local.

Isso muda a pergunta: não é "o descritor HID está certo", é "este dispositivo está na configuração que o console considera aceitável". Um `VID/PID` inventado (`0xCAFE/0x4001`) nunca vai estar — e a autenticação vem por cima.

## O que já foi medido: o console enumera o dispositivo

Ligado na porta USB do console (11.20, kstuff carregado), o Pico foi aceito no nível USB nas duas variantes testadas:

```
ugen1.3: <Omnistik, Omnistik Probe, OMNI-PROBE>(VID=0xcafe PID=0x4001 depth=1 port=2 speed=FS) at usbus1
usb_hid0: <Omnistik Omnistik Probe, class 0/0, rev 2.00/1.00, addr 5> on usbus1
```

Repetido com endereço novo (addr 6) e com N=4, sempre com o mesmo desfecho: driver HID anexado, desconexão limpa, **nenhuma linha de pad no klog, nenhum jogador atribuído, nenhuma reação na tela**.

Isso corrige a hipótese original da spec: a variável não é "o console aceita o dispositivo" — ele aceita. O que falta é a camada entre o driver HID e o jogador, que é onde a autenticação e a atribuição acontecem.

## O que já foi medido: payload não é cliente de pad

No console da bancada, a partir de um payload:

| Chamada | Resultado | Código |
|---|---|---|
| `scePadOpen` no payload | recusa | `0x809b0081` (`USER_NOT_LOGIN`) |
| `scePadOpen` após elevar credenciais de usuário | recusa | `0x80920008` (`DEVICE_NO_HANDLE`) |
| `scePadGetHandle` no payload, no `SceShellCore` e no título para o usuário 1 | recusa | `0x80920008` |
| `scePadReadState` com handle válido dentro do título | recusa | `0x80920003` |

O handle de pad é um índice do cliente de pad **daquele processo**, não um objeto global: copiar o inteiro entre processos não carrega a capacidade. Autenticar com `authid` diferente (`0x4801000000000013` e o `0x3800000000010003` do GhostControl) dá o mesmo resultado, e `np-fake-signin` também não muda nada.

Consequência para o Omnistik: **um payload não consegue ler o controle real**. Escrever input é outra pergunta.

## O que não foi testado

Nunca rodou neste console, e por isso não é resultado:

- `scePadVirtualDeviceAddDevice` e o resto da API `scePadVirtualDevice*`;
- port type 3;
- índices de pad diferentes de 0;
- os setters de contexto do PoorDS4 (`scePadSetLoginUserNumber`, `scePadSetUserNumber`, `scePadSetProcessFocus`).

É exatamente o que a issue #11 mede. A referência externa (`PoorDS4` / GhostControl) injeta em `SceShellUI` e cria um dispositivo virtual — produz input sintético em vez de ler o real, com `MAX_SLOTS 4`.

## O que falta para liberar o ensaio no console

1. Saber se o console aceita o **dispositivo USB** do Pico: só se mede ligando (ondas 2 e 3 da spec).
2. Saber se ele aceita **controle virtual**: só se mede com um ELF de bancada próprio, enviado à porta 9021 (#11).
3. Ter um jogo com N jogadores locais comprovados, para não confundir limite do jogo com limite do USB.
