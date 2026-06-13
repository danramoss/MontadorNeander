/* Trabalho realizado por:
 * Danilo Nascimento Ramos
 * Leonardo Cordeiro da Silva */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MEM_TAM    256
#define MAX_LINHA  256
#define MAX_TOKEN   64

static unsigned char memoria[MEM_TAM];

typedef struct {
    const char   *mnem;
    unsigned char opcode;
    int           tem_op;   /* 1 = a instrucao possui operando (endereco) */
} Instrucao;

static const Instrucao conjunto[] = {
    { "NOP", 0x00, 0 },
    { "STA", 0x10, 1 },
    { "LDA", 0x20, 1 },
    { "ADD", 0x30, 1 },
    { "OR",  0x40, 1 },
    { "AND", 0x50, 1 },
    { "NOT", 0x60, 0 },
    { "JMP", 0x80, 1 },
    { "JN",  0x90, 1 },
    { "JZ",  0xA0, 1 },
    { "HLT", 0xF0, 0 },
};
static const int N_INSTRUCOES = (int)(sizeof(conjunto) / sizeof(conjunto[0]));

static void para_maiusculas(char *s) {
    for (; *s; ++s) *s = (char)toupper((unsigned char)*s);
}

static void erro(int linha, const char *msg, const char *detalhe) {
    fprintf(stderr, "Erro (linha %d): %s", linha, msg);
    if (detalhe && *detalhe) fprintf(stderr, " '%s'", detalhe);
    fprintf(stderr, "\n");
    exit(EXIT_FAILURE);
}

static const Instrucao *busca_instrucao(const char *mnem) {
    char up[MAX_TOKEN];
    strncpy(up, mnem, MAX_TOKEN - 1);
    up[MAX_TOKEN - 1] = '\0';
    para_maiusculas(up);
    for (int i = 0; i < N_INSTRUCOES; ++i)
        if (strcmp(up, conjunto[i].mnem) == 0) return &conjunto[i];
    return NULL;
}

static int eh_hex(const char *s) {
    if (*s == '\0') return 0;
    for (const char *p = s; *p; ++p)
        if (!isxdigit((unsigned char)*p)) return 0;
    return 1;
}

int main(int argc, char *argv[]) {
    if (argc < 2 || argc > 3) {
        fprintf(stderr, "Uso: %s prog1.asm [prog1.mem]\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char *arq_entrada = argv[1];

    char nome_saida[512];
    if (argc == 3) {
        strncpy(nome_saida, argv[2], sizeof(nome_saida) - 1);
        nome_saida[sizeof(nome_saida) - 1] = '\0';
    } else {
        strncpy(nome_saida, arq_entrada, sizeof(nome_saida) - 5);
        nome_saida[sizeof(nome_saida) - 5] = '\0';
        char *ponto = strrchr(nome_saida, '.');
        if (ponto) *ponto = '\0';
        strcat(nome_saida, ".mem");
    }

    FILE *fe = fopen(arq_entrada, "r");
    if (!fe) {
        fprintf(stderr, "Erro: nao foi possivel abrir '%s'\n", arq_entrada);
        return EXIT_FAILURE;
    }

    memset(memoria, 0, sizeof(memoria));

    char linha[MAX_LINHA];
    int  num_linha = 0;
    int  cl = 0;   /* contador de localizacao (endereco atual) */

    while (fgets(linha, sizeof(linha), fe)) {
        num_linha++;

        char *pc = strchr(linha, ';');   /* remove comentario */
        if (pc) *pc = '\0';

        char *mnem = strtok(linha, " \t\r\n");
        if (!mnem) continue;
        char *oper = strtok(NULL, " \t\r\n");
        char *resto = strtok(NULL, " \t\r\n");
        if (resto) erro(num_linha, "tokens em excesso na linha", resto);

        const Instrucao *ins = busca_instrucao(mnem);
        if (!ins) erro(num_linha, "instrucao desconhecida", mnem);

        if (cl >= MEM_TAM)
            erro(num_linha, "programa excede a memoria de 256 bytes", NULL);
        memoria[cl++] = ins->opcode;

        if (ins->tem_op) {
            if (!oper) erro(num_linha, "instrucao exige operando", ins->mnem);
            if (!eh_hex(oper)) erro(num_linha, "operando hexadecimal invalido", oper);
            long end = strtol(oper, NULL, 16);
            if (end < 0 || end >= MEM_TAM)
                erro(num_linha, "endereco fora da faixa 00..FF", oper);
            if (cl >= MEM_TAM)
                erro(num_linha, "programa excede a memoria de 256 bytes", NULL);
            memoria[cl++] = (unsigned char)(end & 0xFF);
        } else if (oper) {
            erro(num_linha, "instrucao nao aceita operando", oper);
        }
    }
    fclose(fe);

    FILE *fs = fopen(nome_saida, "wb");
    if (!fs) {
        fprintf(stderr, "Erro: nao foi possivel criar '%s'\n", nome_saida);
        return EXIT_FAILURE;
    }

    /* cabecalho fixo do arquivo .mem do Neander */
    unsigned char cabecalho[4] = { 0x03, 0x4E, 0x44, 0x52 };
    fwrite(cabecalho, 1, 4, fs);

    /* cada posicao de memoria seguida de um byte 00 */
    for (int i = 0; i < MEM_TAM; ++i) {
        unsigned char par[2] = { memoria[i], 0x00 };
        fwrite(par, 1, 2, fs);
    }
    fclose(fs);

    printf("Montagem concluida com sucesso.\n");
    printf("  Entrada : %s\n", arq_entrada);
    printf("  Saida   : %s  (abra no simulador Neander)\n", nome_saida);
    return EXIT_SUCCESS;
}
