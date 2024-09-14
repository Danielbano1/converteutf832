#include<stdio.h>
#include"converteutf832.h"


int main() {
	int erro;
	// 8p32
	FILE* arquivo_entrada = fopen("texto.txt", "rb");
	FILE* arquivo_saida = fopen("8p32.txt", "wb");

	erro = convUtf8p32(arquivo_entrada, arquivo_saida);
	printf("%d\n", erro);

	fclose(arquivo_entrada);
	fclose(arquivo_saida);

	// 32p8
	FILE* arquivo_entrada2 = fopen("8p32.txt", "rb");
	FILE* arquivo_saida2 = fopen("32p8.txt", "wb");

	erro = convUtf8p32(arquivo_entrada2, arquivo_saida2);
	printf("%d\n", erro);

	fclose(arquivo_entrada2);
	fclose(arquivo_saida2);

	return 0;
}
