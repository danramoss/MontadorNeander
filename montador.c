/* ============================================================================
 *  montador.c  -  Montador (Assembler) para o processador NEANDER
 * ----------------------------------------------------------------------------
 *  Le um arquivo-fonte em assembly do Neander (ex.: prog1.asm) e gera um
 *  arquivo de codigo de maquina (ex.: prog1.mem) que e aberto diretamente
 *  no simulador Neander.
 *
 *  ----- Regras do trabalho -----
 *  Cabecalho: o arquivo binario sempre comeca com os 4 bytes  03 4E 44 52
 *  Traducao : cada instrucao vira o seu opcode e, para cada byte gerado
 *             (opcode OU endereco), e acrescentado mais um byte 00.
 *             Exemplo:  LDA 80   ->   20 00 80 00
 *
 *  ----- Numeros em HEXADECIMAL -----
 *  Os operandos sao escritos em hexadecimal (como no enunciado: "LDA 80"
 *  significa o endereco 80h).
 *
 *  ----- Conjunto de instrucoes -----
 *    Assembly  Opcode   Operando?
 *    NOP       0x00     nao
 *    STA end   0x10     sim
 *    LDA end   0x20     sim
 *    ADD end   0x30     sim
 *    OR  end   0x40     sim
 *    AND end   0x50     sim
 *    NOT       0x60     nao
 *    JMP end   0x80     sim
 *    JN  end   0x90     sim
 *    JZ  end   0xA0     sim
 *    HLT       0xF0     nao
 *
 *  Uma instrucao por linha. Tudo apos ';' e tratado como comentario.
 *
 *  Uso:
 *    ./montador.out prog1.asm [prog1.mem]
 *    (se a saida for omitida, troca a extensao do fonte por .mem)
 *
 *  Compilacao:
 *    gcc -std=c11 -Wall -Wextra -o montador.out montador.c
 * ==========================================================================*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MEM_TAM    256   /* posicoes de memoria do Neander (0..255) */
#define MAX_LINHA  256   /* tamanho maximo de uma linha do fonte    */
#define MAX_TOKEN   64   /* tamanho maximo de um token              */

/* Memoria de saida: um byte por posicao do Neander. */
static unsigned char memoria[MEM_TAM];

/* --------------------------------------------------------------------------
 *  Descricao de uma instrucao do Neander
 * ------------------------------------------------------------------------*/
typedef struct {
    const char   *mnem;     /* mnemonico                      */
    unsigned char opcode;   /* codigo da operacao             */
    int           tem_op;   /* 1 = possui operando (endereco) */
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

/* --------------------------------------------------------------------------
 *  Utilitarios
 * ------------------------------------------------------------------------*/

/* Converte string para maiusculas no proprio buffer. */
static void para_maiusculas(char *s) {
    for (; *s; ++s) *s = (char)toupper((unsigned char)*s);
}

/* Encerra o programa com uma mensagem de erro de montagem. */
static void erro(int linha, const char *msg, const char *detalhe) {
    fprintf(stderr, "Erro (linha %d): %s", linha, msg);
    if (detalhe && *detalhe) fprintf(stderr, " '%s'", detalhe);
    fprintf(stderr, "\n");
    exit(EXIT_FAILURE);
}

/* Procura uma instrucao pelo mnemonico (case-insensitive). NULL se nao achar. */
static const Instrucao *busca_instrucao(const char *mnem) {
    char up[MAX_TOKEN];
    strncpy(up, mnem, MAX_TOKEN - 1);
    up[MAX_TOKEN - 1] = '\0';
    para_maiusculas(up);
    for (int i = 0; i < N_INSTRUCOES; ++i)
        if (strcmp(up, conjunto[i].mnem) == 0) return &conjunto[i];
    return NULL;
}

/* Verifica se o token e um numero hexadecimal valido. */
static int eh_hex(const char *s) {
    if (*s == '\0') return 0;
    for (const char *p = s; *p; ++p)
        if (!isxdigit((unsigned char)*p)) return 0;
    return 1;
}

/* ==========================================================================
 *  PROGRAMA PRINCIPAL
 * ========================================================================*/
int main(int argc, char *argv[]) {
    if (argc < 2 || argc > 3) {
        fprintf(stderr, "Uso: %s prog1.asm [prog1.mem]\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char *arq_entrada = argv[1];

    /* Nome de saida: argv[2] ou troca a extensao do fonte por .mem */
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

    /* ----------------------------------------------------------------------
     *  Leitura e traducao: uma instrucao por linha, montada sequencialmente
     *  a partir do endereco 0.
     * --------------------------------------------------------------------*/
    char linha[MAX_LINHA];
    int  num_linha = 0;   /* contador de linhas (para mensagens de erro) */
    int  cl = 0;          /* contador de localizacao (endereco atual)    */

    while (fgets(linha, sizeof(linha), fe)) {
        num_linha++;

        /* remove o comentario: tudo a partir de ';' */
        char *pc = strchr(linha, ';');
        if (pc) *pc = '\0';

        /* separa os tokens (mnemonico e operando) */
        char *mnem = strtok(linha, " \t\r\n");
        if (!mnem) continue;                 /* linha em branco/comentario */
        char *oper = strtok(NULL, " \t\r\n");
        char *resto = strtok(NULL, " \t\r\n");
        if (resto) erro(num_linha, "tokens em excesso na linha", resto);

        /* identifica a instrucao */
        const Instrucao *ins = busca_instrucao(mnem);
        if (!ins) erro(num_linha, "instrucao desconhecida", mnem);

        /* grava o opcode */
        if (cl >= MEM_TAM)
            erro(num_linha, "programa excede a memoria de 256 bytes", NULL);
        memoria[cl++] = ins->opcode;

        /* grava o operando (endereco), se a instrucao exigir */
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

    /* ----------------------------------------------------------------------
     *  Gravacao do arquivo .mem
     *  - Cabecalho de 4 bytes:  03 4E 44 52
     *  - Em seguida, para cada posicao de memoria, grava o valor seguido de
     *    um byte 00 (regra: "acrescentar mais um byte 00").
     *    Assim, LDA 80 em MEM[0..1] = 20 80  vira  20 00 80 00.
     * --------------------------------------------------------------------*/
    FILE *fs = fopen(nome_saida, "wb");
    if (!fs) {
        fprintf(stderr, "Erro: nao foi possivel criar '%s'\n", nome_saida);
        return EXIT_FAILURE;
    }

    unsigned char cabecalho[4] = { 0x03, 0x4E, 0x44, 0x52 };
    fwrite(cabecalho, 1, 4, fs);

    for (int i = 0; i < MEM_TAM; ++i) {
        unsigned char par[2];
        par[0] = memoria[i];   /* valor da posicao     */
        par[1] = 0x00;         /* byte 00 acrescentado */
        fwrite(par, 1, 2, fs);
    }
    fclose(fs);

    printf("Montagem concluida com sucesso.\n");
    printf("  Entrada : %s\n", arq_entrada);
    printf("  Saida   : %s  (abra no simulador Neander)\n", nome_saida);
    return EXIT_SUCCESS;
}
