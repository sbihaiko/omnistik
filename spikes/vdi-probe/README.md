# vdi-probe — controle virtual a partir de um payload

Spike da issue #11. Responde a uma pergunta só: **um payload consegue criar um controle virtual no PS5?**

O caminho USB direto está medido e descartado ([contrato do alvo](../../docs/contrato-do-alvo.md)): o console enumera o Pico e não o torna jogador, porque a autenticação (`0xF0`–`0xF5`) é protocolo do dispositivo. O `PoorDS4`/Ghostcontrol contorna isso criando um DualSense **virtual** dentro do console com `scePadVirtualDeviceAddDevice` e injetando input a 60 Hz — o dispositivo virtual nasce depois da autenticação, então não precisa dela.

## Etapas

| Etapa | O que roda | Pergunta | Custo |
|---|---|---|---|
| **0 — chamada crua** | este binário chama `scePadVirtualDeviceAddDevice` no próprio processo e imprime o retorno | o console recusa do jeito que recusa o `scePadOpen` (`0x809b0081`)? | baixo |
| **1 — anexar ao SceShellUI** | ptrace no `SceShellUI`, resolução de `libScePad`, chamada no contexto do shell | a chamada passa a funcionar no processo certo? | médio, **risco de travar o console** |
| **2 — handle e vínculo** | ler o `DEVICE_ADDED` do klog, atar o controle virtual ao usuário do primeiro plano | o jogador aparece? | alto |
| **3 — input** | injetar `ScePadData` a 60 Hz e ver a tela reagir | o input chega ao jogo? | alto |
| **4 — ler o Pico** | trocar o gerador sintético pelos relatórios do nosso HID | fecha o gateway | alto |

Cada etapa só começa se a anterior der resultado interpretável. A etapa 0 é a que este esboço entrega.

## Riscos que precisam estar escritos antes da etapa 1

- **Anexar ao `SceShellUI` é a mesma classe de operação que travou o console no spike de controle do `ariel`**: lá o `ptrace` no título em primeiro plano congelou o sistema. Aqui o alvo é o shell, que é mais crítico ainda. Um travamento no shell derruba a cadeia inteira e exige reboot físico.
- **O console já perdeu o armazenamento interno quatro vezes.** `sync` antes de qualquer ensaio é obrigatório, e o procedimento de desligamento seguro está em `ariel/docs/console-setup.md`.
- **A API não foi exercitada a partir de um payload neste console.** O símbolo existe em `libScePad.so`; o comportamento não foi medido.

## Licença

O `PoorDS4`/Ghostcontrol é **GPL-3.0-or-later** e este projeto é MIT. Nada de código dele é copiado aqui: o que se aproveita são fatos publicados — os nomes das funções, o `type=3`, a ideia de pegar o handle no evento `DEVICE_ADDED` do klog e o vínculo ao usuário via MBus no shell. A implementação do anexo precisa ser escrita do zero.

## Build

```sh
export PS5_PAYLOAD_SDK=~/.local/share/ps5-payload-sdk
make
```

O `make deploy` envia para `elfldr` na 9021 do `PS5_HOST`.
