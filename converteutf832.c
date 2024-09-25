#include <stdio.h>
#include"converteutf832.h"

union u {
	unsigned char c[4];
	unsigned int i;
};

int secundaria8p32(int insere, FILE* arquivo_entrada, FILE* arquivo_saida, int repeticoes) {
	unsigned char caractere;
	for (int i = 0; i < repeticoes; i++) {
		insere = insere << 8;
		if (fread(&caractere, sizeof(unsigned char), 1, arquivo_entrada) != 1) {
			fprintf(stderr, "Ocorreu um erro de leitura!\n");
			return -1;
		}
		caractere = caractere << 2;
		insere = insere | caractere;
		insere = insere >> 2;
	}
	if (fwrite(&insere, sizeof(unsigned int), 1, arquivo_saida) != 1) {
		fprintf(stderr, "Ocorreu um erro de gravacao!\n");
		return -1;
	}
	else
		return 0;
}

int convUtf8p32(FILE* arquivo_entrada, FILE* arquivo_saida) {
	const unsigned int BOM = 0x0000feff;
	unsigned char caractere;
	unsigned int insere;
	int repeticoes = 0;

	fwrite(&BOM, sizeof(unsigned int), 1, arquivo_saida);

	while (fread(&caractere, sizeof(unsigned char), 1, arquivo_entrada) == 1) {
		if (caractere <= 127)
			repeticoes = 0;

		else if (caractere <= 223) {
			caractere = caractere << 3;
			caractere = caractere >> 3;
			repeticoes = 1;
		}

		else if (caractere <= 239) {
			caractere = caractere << 4;
			caractere = caractere >> 4;
			repeticoes = 2;
		}

		else if (caractere <= 247) {
			caractere = caractere << 5;
			caractere = caractere >> 5;
			repeticoes = 3;
		}

		insere = (unsigned int)caractere;
		
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




int convUtf32p8(FILE* arquivo_entrada, FILE* arquiva_saida) {
	unsigned int BOM;
	union u inteiro;
	unsigned char vchar[4];
	unsigned char temp;
	int repeticoes = 0;



	if (fread(&BOM, sizeof(unsigned int), 1, arquivo_entrada) != 1) {
		fprintf(stderr, "Ocorreu um erro de leitura!\n");
		return -1;
	}



	while (fread(vchar, sizeof(unsigned char), 4, arquivo_entrada) == 4) {
		// Estruturar dado
		if (BOM == 0xfffe0000) {    // big-endian
			printf("big-endian");
			for (int i = 0; i < 4; i++)
				inteiro.c[i] = vchar[3 - i];
		}
		else if (BOM == 0xfeff) {   // little-endian     
			printf("little-endian");					
			for (int i = 0; i < 4; i++)					
				inteiro.c[i] = vchar[i];
		}
		else {
			fprintf(stderr, "BOM inválido ou ausente!\n");
			return -1;
		}

		// Identificar tipo
		if (inteiro.i <= 0x7f) {
			if (fwrite(&inteiro.c[0], sizeof(unsigned char), 1, arquiva_saida) != 1) {
				fprintf(stderr, "Ocorreu um erro de gravacao!\n");
				return -1;
			}

			repeticoes = 0;
		}

		else if (inteiro.i <= 0x7ff) {
			inteiro.c[1] = inteiro.c[1] << 2;
			temp = inteiro.c[0] >> 6;
			inteiro.c[1] = inteiro.c[1] | temp;
			inteiro.c[1] = inteiro.c[1] | 0xc0;
			if (fwrite(&inteiro.c[1], sizeof(unsigned char), 1, arquiva_saida) != 1) {
				fprintf(stderr, "Ocorreu um erro de gravacao!\n");
				return -1;
			}

			repeticoes = 1;
		}

		else if (inteiro.i <= 0xffff) {
			inteiro.c[2] = inteiro.c[1] >> 4;
			inteiro.c[2] = inteiro.c[2] | 0xe0;
			if (fwrite(&inteiro.c[2], sizeof(unsigned char), 1, arquiva_saida) != 1) {
				fprintf(stderr, "Ocorreu um erro de gravacao!\n");
				return -1;
			}

			repeticoes = 2;
		}

		else if (inteiro.i <= 0x10ffff) {
			// primeiro byte
			inteiro.c[3] = inteiro.c[2] >> 2;
			inteiro.c[3] = inteiro.c[3] | 0xf0;
			if (fwrite(&inteiro.c[3], sizeof(unsigned char), 1, arquiva_saida) != 1) {
				fprintf(stderr, "Ocorreu um erro de gravacao!\n");
				return -1;
			}

			// segundo byte
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
		printf("%0x\n", inteiro.i);
	}
	if (feof(arquivo_entrada))
		return 0;
	else {
		fprintf(stderr, "Ocorreu um erro de leitura!\n");
		return -1;
	}
}
