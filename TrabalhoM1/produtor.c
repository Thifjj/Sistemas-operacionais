#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

int main() {
    FILE *arquivo_in, *arquivo_out;
    char tipo[3];
    int largura, altura, max_valor;
    unsigned char *pixels; // Usamos unsigned char porque P5 armazena bytes (0-255)
    char * myfifo = "/tmp/myfifo";
    int fd;

    // Creating the named file(FIFO)
    // mkfifo(<pathname>, <permission>)
    mkfifo(myfifo, 0666);

    // --- 1. LEITURA (FORMATO P5 - BINÁRIO) ---
    arquivo_in = fopen("imagem.pgm", "rb"); // "rb" para leitura binária
    if (!arquivo_in) {
        printf("Erro ao abrir arquivo.\n");
        return 1;
    }

    // Ler cabeçalho (o cabeçalho ainda é texto, mesmo no P5)
    fscanf(arquivo_in, "%s\n", tipo);
    if (tipo[1] != '5') {
        printf("Erro: Este código é apenas para PGM P5.\n");
        fclose(arquivo_in);
        return 1;
    }

    // Pular comentários se existirem
    char c = fgetc(arquivo_in);
    while (c == '#') {
        while (fgetc(arquivo_in) != '\n');
        c = fgetc(arquivo_in);
    }
    ungetc(c, arquivo_in);

    fscanf(arquivo_in, "%d %d\n%d\n", &largura, &altura, &max_valor);

    // Alocação da memória
    int total_pixels = largura * altura;
    pixels = (unsigned char *)malloc(total_pixels * sizeof(unsigned char));

    // LER OS DADOS BINÁRIOS DE UMA VEZ SÓ
    fread(pixels, sizeof(unsigned char), total_pixels, arquivo_in);
    fclose(arquivo_in);

    printf("Imagem P5 lida: %dx%d\n", largura, altura);

    fd = open(myfifo, O_WRONLY);
    if (fd == -1) return 1;

    // ENVIA O CABEÇALHO PRIMEIRO (para o consumidor saber o tamanho)
    write(fd, &largura, sizeof(int));
    write(fd, &altura, sizeof(int));
    write(fd, &max_valor, sizeof(int));

    // ENVIA OS PIXELS
    write(fd, pixels, total_pixels);

    close(fd);
    free(pixels);
    return 0;

    return 0;
}