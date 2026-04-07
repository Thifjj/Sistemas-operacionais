#include "config.h" // Substitua pelo nome real do seu arquivo de cabeçalho, se for diferente

int main(int argc, char* argv[]) {
    // Verificação simples para garantir que os argumentos foram passados

    PGM imagem;
    Header cabecalho;

    const char* path = FIFO_PATH;         // Caminho da FIFO (ex: /tmp/fifo)
    const char* nome_arquivo = IMAGE_PATH; // Nome da imagem PGM

    // Cria a named pipe (FIFO) com permissões de leitura e escrita
    mkfifo(path, 0666);

    // Lê e salva a imagem na struct PGM usando a função do seu cabeçalho
    printf("Produtor: Lendo a imagem %s...\n", nome_arquivo);
    if (read_PGM(nome_arquivo, &imagem) != 0) {//escreve a imagem na struct PGM
        printf("Erro ao processar a imagem. Encerrando.\n");
        return 1;
    }

    // Prepara o header com os metadados para enviar ao consumidor (Worker)
    cabecalho.w = imagem.w;
    cabecalho.h = imagem.h;
    cabecalho.maxv = imagem.maxv;
    cabecalho.mode = NEGATIVO;  // Define o modo inicial (pode ser alterado depois)
    cabecalho.t1 = 0;
    cabecalho.t2 = 0;

    // Abre a named pipe (A execução vai pausar aqui até o Consumidor abrir para leitura)
    printf("Produtor: Esperando pelo Consumidor (Worker) abrir a FIFO...\n");
    int fd = open(path, O_WRONLY);
    if (fd == -1) {
        perror("Erro ao abrir a FIFO para escrita");
        free(imagem.data);
        return 1;
    }

    printf("Produtor: Consumidor conectado. Enviando cabecalho e pixels...\n");

    // Envia a struct do cabeçalho primeiro para o consumidor saber as dimensões
    write(fd, &cabecalho, sizeof(Header));

    // Envia os pixels em um loop para garantir que nada se perca no buffer
    size_t tamanho_esperado = imagem.w * imagem.h;
    size_t tamanho_enviado = 0;
    
    while(tamanho_enviado < tamanho_esperado) {
        // write retorna a quantidade de bytes que realmente conseguiu escrever nesta iteração
        size_t n = write(fd, imagem.data + tamanho_enviado, tamanho_esperado - tamanho_enviado);
        
        if (n == -1) {
            perror("Erro na escrita dos dados");
            break;
        }
        tamanho_enviado += n;
    }

    printf("Produtor: %zu bytes de dados enviados com sucesso.\n", tamanho_enviado);

    // Limpeza da memória alocada dinamicamente pelo read_PGM e fechamento da FIFO
    close(fd);
    free(imagem.data);

    return 0;
}