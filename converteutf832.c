#include<stdio.h>

int secundaria8p32(FILE* arquivo_entrada, FILE* arquivo_saida, int repeticoes);

int convUtf8p32(FILE* arquivo_entrada, FILE* arquivo_saida) {
	unsigned char caracter;
	const unsigned int BOM = 0xfffe0000;
	unsigned int insere;
	int repeticoes;

	// Escreve BOM
	fwrite(&BOM, sizeof(unsigned int), 1, arquivo_saida);

	while (fread(&caracter, sizeof(unsigned char), 1, arquivo_entrada == 1) {

		if (caracter <= 127)
			repeticoes = 0;

		else if (caracter <= 223) {
			caracter = caracter << 3;
			caracter = caracter >> 3;
			repeticoes = 1;
		}

		else if (caracter <= 239) {
			caracter = caracter << 4;
			caracter = caracter >> 4;
			repeticoes = 2;
		}

		else if (caracter <= 247) {
			caracter = caracter << 5;
			caracter = caracter >> 5;
			repeticoes = 3;
		}

		fwrite(&caracter, sizeof(unsigned char), 1, arquivo_saida);

		if (repeticoes > 0)
			if (secundaria8p32(arquivo_entrada, arquivo_saida, repeticoes) == 0)
				return 0; //erro na gravacao
	}

}