// Etapa 0 do spike #11: chamar scePadVirtualDeviceAddDevice do próprio
// processo e registrar o que o console responde.
//
// A pergunta é estreita de propósito. O ariel já mediu que um payload não é
// cliente de pad (scePadOpen -> 0x809b0081). Se a criação de dispositivo
// virtual também for recusada daqui, a etapa 1 (anexar ao SceShellUI) deixa
// de ser opcional e passa a ser o único caminho — e é ela que carrega o risco
// de travar o console.

#include <stdint.h>
#include <stdio.h>
#include <string.h>

// --- libScePad -------------------------------------------------------------
// Nomes e assinaturas vêm dos símbolos exportados por libScePad.so; o SDK
// entrega a biblioteca sem cabeçalho.
int scePadInit(void);
int scePadVirtualDeviceAddDevice(void *param, int32_t type);
int scePadVirtualDeviceDeleteDevice(int32_t handle);

// Tipo do controle virtual. O Ghostcontrol usa 3 para DualSense; é um fato
// publicado, não código copiado.
#define VDI_TYPE_DUALSENSE 3

// Parâmetro de criação: nenhum campo foi confirmado neste console. Passamos um
// bloco zerado e deixamos o retorno dizer se o console valida o conteúdo.
typedef struct {
    uint8_t unknown[64];
} vdi_param_t;

static const char *hex(int32_t v) {
    static char buf[16];
    snprintf(buf, sizeof(buf), "0x%08x", (uint32_t)v);
    return buf;
}

int main(void) {
    printf("vdi-probe: etapa 0, chamada crua\n");

    int rc = scePadInit();
    printf("scePadInit                 -> %s\n", hex(rc));

    vdi_param_t param;
    memset(&param, 0, sizeof(param));

    int32_t handle = scePadVirtualDeviceAddDevice(&param, VDI_TYPE_DUALSENSE);
    printf("scePadVirtualDeviceAdd     -> %s\n", hex(handle));

    if (handle >= 0) {
        printf("handle aceito; removendo\n");
        int del = scePadVirtualDeviceDeleteDevice(handle);
        printf("scePadVirtualDeviceDelete  -> %s\n", hex(del));
    } else {
        printf("recusado no próprio processo; etapa 1 (anexo ao SceShellUI) é o próximo passo\n");
    }

    printf("vdi-probe: fim\n");
    return 0;
}
