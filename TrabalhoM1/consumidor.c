#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include "fifo_config.h"
#include <pthread.h>

// Variáveis Globais
pthread_mutex_t mutex_cor = PTHREAD_MUTEX_INITIALIZER; // protege i_pixel_count entre threads
int i_pixel_count = 0; // índice do próximo pixel a ser processado
unsigned char *pixels; // buffer onde os pixels lidos da FIFO serão armazenados
int total_global = 0; // quantidade total de pixels = largura * altura
int largura, altura, max_valor; // informações do cabeçalho PGM

// Função para as threads
void *inverte_valor_cor(void *arg) {
    while (1) {
        // pega o próximo índice disponível de pixel com proteção de mutex.
        // Assim, duas threads não processam o mesmo pixel.
        pthread_mutex_lock(&mutex_cor);
        if (i_pixel_count >= total_global) {
            pthread_mutex_unlock(&mutex_cor);
            return NULL;
        }
        int index = i_pixel_count++;
        pthread_mutex_unlock(&mutex_cor);

        // inverte o valor do pixel no buffer já preenchido:
        // o byte está em pixels[index] e é substituído por 255 - valor.
        pixels[index] = (unsigned char)(255 - pixels[index]);
    }
}

int main() {
    char *myfifo = fifo_path;
    int fd;
    
    printf("Consumidor: Aguardando produtor abrir a FIFO...\n");
    fd = open(myfifo, O_RDONLY); 
    if (fd == -1) {
        perror("Erro ao abrir a FIFO");
        return 1;
    }

    // --- PASSO 1: LER O CABEÇALHO DA FIFO ---
    // O produtor envia largura, altura e max_valor primeiro
    read(fd, &largura, sizeof(int));
    read(fd, &altura, sizeof(int));
    read(fd, &max_valor, sizeof(int));
    
    total_global = largura * altura;
    printf("Configuração recebida: %dx%d, Total de pixels: %d\n", largura, altura, total_global);

    // --- PASSO 2: ALOCAR MEMÓRIA EXATA ---
    pixels = (unsigned char *)malloc(total_global);
    if (pixels == NULL) {
        perror("Erro ao alocar memória");
        return 1;
    }

    // --- PASSO 3: LER OS PIXELS ---
    // O read() copia bytes diretamente para o buffer "pixels".
    // A cada iteração ele escreve a partir de pixels + bytes_lidos,
    // garantindo que os dados sejam colocados em sequência no buffer.
    int bytes_lidos = 0;
    while (bytes_lidos < total_global) {
        ssize_t n = read(fd, pixels + bytes_lidos, total_global - bytes_lidos);
        if (n <= 0) {
            perror("Erro ao ler pixels");
            close(fd);
            free(pixels);
            return 1;
        }
        bytes_lidos += n;
    }

    // dividido em 4 threads o processo de inverter o valor do pixel
    pthread_t Tid1, Tid2, Tid3, Tid4;
        pthread_create(&Tid1, NULL, inverte_valor_cor, NULL);
        pthread_create(&Tid2, NULL, inverte_valor_cor, NULL);
        pthread_create(&Tid3, NULL, inverte_valor_cor, NULL);
        pthread_create(&Tid4, NULL, inverte_valor_cor, NULL);
        pthread_join(Tid1, NULL);
        pthread_join(Tid2, NULL);
        pthread_join(Tid3, NULL);
        pthread_join(Tid4, NULL);
        
        

        // --- PASSO 5: SALVAR ARQUIVO PGM VÁLIDO ---
        FILE *out = fopen("saida_final.pgm", "wb");
        if (out) {
            // O segredo está aqui: o cabeçalho P5
            fprintf(out, "P5\n%d %d\n%d\n", largura, altura, max_valor);
            fwrite(pixels, 1, total_global, out);
            fclose(out);
            printf("Sucesso! Imagem 'saida_final.pgm' gerada.\n");
        } else {
            perror("Erro ao criar arquivo de saída");
        }


    close(fd);
    free(pixels);
    return 0;
}