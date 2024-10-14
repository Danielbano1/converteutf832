#include <stdio.h>
#include "converteutf832.h"

// Define uma união para armazenar 4 bytes de um caractere UTF-32 ou um inteiro
union u {
    unsigned char c[4];
    unsigned int i;
};

/**
 * Função auxiliar que insere bytes de um caractere UTF-8 em uma variável de 32 bits.
 * @param insere Valor inicial do caractere UTF-8.
 * @param arquivo_entrada Ponteiro para o arquivo de entrada.
 * @param arquivo_saida Ponteiro para o arquivo de saída.
 * @param repeticoes Número de bytes a serem processados (0 a 3).
 * @return 0 em caso de sucesso, -1 em caso de erro.
 */
int secundaria8p32(int insere, FILE* arquivo_entrada, FILE* arquivo_saida, int repeticoes) {
    unsigned char caractere;

    for (int i = 0; i < repeticoes; i++) {
        insere = insere << 8; // Desloca bits para a esquerda
        if (fread(&caractere, sizeof(unsigned char), 1, arquivo_entrada) != 1) {
            fprintf(stderr, "Ocorreu um erro de leitura!\n");
            return -1;
        }
        caractere = caractere << 2;  // Manipula o caractere
        insere = insere | caractere;
        insere = insere >> 2;  // Corrige deslocamento de bits
    }

    if (fwrite(&insere, sizeof(unsigned int), 1, arquivo_saida) != 1) {
        fprintf(stderr, "Ocorreu um erro de gravacao!\n");
        return -1;
    } else {
        return 0;
    }
}

/**
 * Converte um arquivo de texto codificado em UTF-8 para UTF-32.
 * @param arquivo_entrada Ponteiro para o arquivo de entrada codificado em UTF-8.
 * @param arquivo_saida Ponteiro para o arquivo de saída que receberá dados em UTF-32.
 * @return 0 em caso de sucesso, -1 em caso de erro.
 */
int convUtf8p32(FILE* arquivo_entrada, FILE* arquivo_saida) {
    const unsigned int BOM = 0x0000feff; // Marca de ordem de byte UTF-32 (little-endian)
    unsigned char caractere;
    unsigned int insere;
    int repeticoes = 0;

    fwrite(&BOM, sizeof(unsigned int), 1, arquivo_saida); // Escreve o BOM no início do arquivo

    while (fread(&caractere, sizeof(unsigned char), 1, arquivo_entrada) == 1) {
        // Determina o número de bytes com base no valor do caractere UTF-8
        if (caractere <= 127)
            repeticoes = 0;
        else if (caractere <= 223) {
            caractere = caractere << 3;
            caractere = caractere >> 3;
            repeticoes = 1;
        } else if (caractere <= 239) {
            caractere = caractere << 4;
            caractere = caractere >> 4;
            repeticoes = 2;
        } else if (caractere <= 247) {
            caractere = caractere << 5;
            caractere = caractere >> 5;
            repeticoes = 3;
        }

        insere = (unsigned int)caractere; // Inicializa o valor a ser inserido

        if (secundaria8p32(insere, arquivo_entrada, arquivo_saida, repeticoes) == -1)
            return -1;
    }

    if (feof(arquivo_entrada))
        return 0;
    else {
        fprintf(stderr, "Ocorreu um erro de leitura!\n");
        return -1;
    }
}

/**
 * Converte um arquivo de texto codificado em UTF-32 para UTF-8.
 * @param arquivo_entrada Ponteiro para o arquivo de entrada codificado em UTF-32.
 * @param arquiva_saida Ponteiro para o arquivo de saída que receberá dados em UTF-8.
 * @return 0 em caso de sucesso, -1 em caso de erro.
 */
int convUtf32p8(FILE* arquivo_entrada, FILE* arquiva_saida) {
    unsigned int BOM;
    union u inteiro;
    unsigned char vchar[4];
    unsigned char temp;
    int repeticoes = 0;

    // Verifica e lê a marca de ordem de byte (BOM)
    if (fread(&BOM, sizeof(unsigned int), 1, arquivo_entrada) != 1) {
        fprintf(stderr, "Ocorreu um erro de leitura!\n");
        return -1;
    }

    while (fread(vchar, sizeof(unsigned char), 4, arquivo_entrada) == 4) {
        // Verifica se é big-endian ou little-endian e ajusta a ordem dos bytes
        if (BOM == 0xfffe0000) {    // big-endian
            for (int i = 0; i < 4; i++)
                inteiro.c[i] = vchar[3 - i];
        } else if (BOM == 0xfeff) { // little-endian
            for (int i = 0; i < 4; i++)
                inteiro.c[i] = vchar[i];
        } else {
            fprintf(stderr, "BOM inválido ou ausente!\n");
            return -1;
        }

        // Converte o valor UTF-32 de volta para UTF-8
        if (inteiro.i <= 0x7f) {
            if (fwrite(&inteiro.c[0], sizeof(unsigned char), 1, arquiva_saida) != 1) {
                fprintf(stderr, "Ocorreu um erro de gravacao!\n");
                return -1;
            }
            repeticoes = 0;
        } else if (inteiro.i <= 0x7ff) {
            inteiro.c[1] = inteiro.c[1] << 2;
            temp = inteiro.c[0] >> 6;
            inteiro.c[1] = inteiro.c[1] | temp;
            inteiro.c[1] = inteiro.c[1] | 0xc0;
            if (fwrite(&inteiro.c[1], sizeof(unsigned char), 1, arquiva_saida) != 1) {
                fprintf(stderr, "Ocorreu um erro de gravacao!\n");
                return -1;
            }
            repeticoes = 1;
        } else if (inteiro.i <= 0xffff) {
            inteiro.c[2] = inteiro.c[1] >> 4;
            inteiro.c[2] = inteiro.c[2] | 0xe0;
            if (fwrite(&inteiro.c[2], sizeof(unsigned char), 1, arquiva_saida) != 1) {
                fprintf(stderr, "Ocorreu um erro de gravacao!\n");
                return -1;
            }
            repeticoes = 2;
        } else if (inteiro.i <= 0x10ffff) {
            // Converte caracteres UTF-32 para UTF-8 (3 ou 4 bytes)
            inteiro.c[3] = inteiro.c[2] >> 2;
            inteiro.c[3] = inteiro.c[3] | 0xf0;
            if (fwrite(&inteiro.c[3], sizeof(unsigned char), 1, arquiva_saida) != 1) {
                fprintf(stderr, "Ocorreu um erro de gravacao!\n");
                return -1;
            }

            // Converte os bytes restantes
            inteiro.c[2] = inteiro.c[2] << 6;
            inteiro.c[2] = inteiro.c[2] >> 2;
            temp = inteiro.c[1] >> 4;
            inteiro.c[2] = inteiro.c[2] | temp;
            inteiro.c[2] = inteiro.c[2] | 0x80;
            if (fwrite(&inteiro.c[2], sizeof(unsigned char), 1, arquiva_saida) != 1) {
                fprintf(stderr, "Ocorreu um erro de gravacao!\n");
                return -1;
            }
            repeticoes = 2;
        }

        // Processa o último byte se necessário
        if (repeticoes == 2) {
            inteiro.c[1] = inteiro.c[1] << 4;
            inteiro.c[1] = inteiro.c[1] >> 2;
            temp = inteiro.c[0] >> 6;
            inteiro.c[1] = inteiro.c[1] | temp;
            inteiro.c[1] = inteiro.c[1] | 0x80;
            if (fwrite(&inteiro.c[1], sizeof(unsigned char), 1, arquiva_saida) != 1) {
                fprintf(stderr, "Ocorreu um erro de gravacao!\n");
                return -1;
            }
        }

        if (repeticoes >= 1) {
            inteiro.c[0] = inteiro.c[0] & 0x3f;
            inteiro.c[0] = inteiro.c[0] | 0x80;
            if (fwrite(&inteiro.c[0], sizeof(unsigned char), 1, arquiva_saida) != 1) {
                fprintf(stderr, "Ocorreu um erro de gravacao!\n");
                return -1;
            }
        }
    }

    if (feof(arquivo_entrada))
        return 0;
    else {
        fprintf(stderr, "Ocorreu um erro de leitura!\n");
        return -1;
    }
}
