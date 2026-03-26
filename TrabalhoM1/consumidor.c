#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>

// Variáveis Globais
int i_PP = 0;
unsigned char *pixels;
int total_global = 0; 
int largura, altura, max_valor; // Adicionadas aqui

// Função para as threads
void *inverte_valor_cor(void *arg) {
    while (i_PP < total_global) {
        // Cálculo simples de inversão
        pixels[i_PP] = (unsigned char)(255 - pixels[i_PP]);
        i_PP++;
    }
    return NULL;
}

int main() {
    char *myfifo = "/tmp/myfifo";
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

    // --- PASSO 3: LER OS PIXELS (LOOP GARANTIDO) ---
    int lido_agora = 0;
    int total_lido = 0;

    while (total_lido < total_global) {
        lido_agora = read(fd, pixels + total_lido, total_global - total_lido);
        if (lido_agora <= 0) break; 
        total_lido += lido_agora;
    }
    printf("Total lido da FIFO: %d bytes.\n", total_lido);

    // --- PASSO 4: PROCESSAR ---
    if (total_lido > 0) {
        printf("Iniciando processamento...\n");
        
        // Se quiser usar threads de verdade, descomente aqui:
        /*
        pthread_t thread1;
        pthread_create(&thread1, NULL, inverte_valor_cor, NULL);
        pthread_join(thread1, NULL);
        */
        
        inverte_valor_cor(NULL); // Chamada direta por enquanto

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
    }

    close(fd);
    free(pixels);
    return 0;
}