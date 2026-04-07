#include "config.h" // Seu arquivo de structs e funcoes
#include <sys/time.h>

#define NUM_TASKS 8
#define NUM_THREADS 4

// Controle de concorrência
sem_t semaforo;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
int prox_tarefa = 0;

// Dados compartilhados para processamento
PGM g_imagem;
Task tarefa[NUM_TASKS];
int g_mode;         // NEGATIVO (0) ou SLICE (1)
int g_t1, g_t2;     // Limites inferior e superior para fatiamento

// Protótipos das funções das threads
void aplicar_negativo(void* arg);
void aplicar_fatiamento(void* arg);

int main(int argc, char* argv[]) {
    // Verificação básica de argumentos
    if(argc < 2) {
        printf("Uso Negativo: 0 [num_threads]\n");
        printf("Uso Fatiamento:  1 <t1> <t2> [num_threads]\n");
        return 1;
    }

    Header cabecalho;
    int num_threads;
    char nome[50]; 

    const char* path = FIFO_PATH; 
    int modo = atoi(argv[1]); 

    // Configuração baseada nos argumentos de linha de comando
    if(modo == NEGATIVO) {
        g_mode = NEGATIVO;
        if(argc >= 3) { // Ajustado para pegar o argumento correto (argv[2])
            num_threads = atoi(argv[2]);
        } else {
            num_threads = NUM_THREADS;
        }
    } else if(modo == SLICE) {
        g_mode = SLICE;
        if(argc < 4) {
            printf("Erro: Fatiamento exige os parametros t1 e t2.\n");
            return 1;
        }
        g_t1 = atoi(argv[2]); // Ajustado indices dos argumentos
        g_t2 = atoi(argv[3]);
        if(argc >= 5) {
            num_threads = atoi(argv[4]);
        } else {
            num_threads = NUM_THREADS;
        }
    } else {
        printf("Modo invalido.\n");
        return 1;
    }

    printf("Consumidor: Preparando %d threads.\n", num_threads);

    // Abre a FIFO apenas para leitura (trava aqui ate o Produtor abrir para escrita)
    int fd = open(path, O_RDONLY);
    if(fd == -1){
        perror("Erro ao abrir a FIFO");
        return 1;
    }
    
    printf("Consumidor: Recebendo dados...\n");

    // Lê o cabeçalho
    if(read(fd, &cabecalho, sizeof(Header)) <= 0) {
        perror("Erro ao ler cabecalho");
        close(fd);
        return 1;
    }
    
    g_imagem.w = cabecalho.w;
    g_imagem.h = cabecalho.h;
    g_imagem.maxv = cabecalho.maxv;
    printf("Imagem recebida do cabeçalho - Altura: %d | Largura: %d | Maxv: %d\n", g_imagem.h, g_imagem.w, g_imagem.maxv);

    // Aloca a memória e lê os pixels
    g_imagem.data = (unsigned char*)malloc(g_imagem.w * g_imagem.h * sizeof(unsigned char));
    size_t tamanho_esperado = g_imagem.w * g_imagem.h;
    size_t tamanho_lido = 0;
    
    while(tamanho_lido < tamanho_esperado) {
        size_t n = read(fd, g_imagem.data + tamanho_lido, tamanho_esperado - tamanho_lido);
        if (n <= 0) break; // Evita loop infinito se a conexão cair
        tamanho_lido += n;
    }
    printf("Consumidor: %ld bytes lidos com sucesso.\n", tamanho_lido);
    close(fd); 

    // --- Início do preparo para as Threads ---
    sem_init(&semaforo, 0, num_threads); 

    // Divide a imagem em tarefas (blocos de linhas horizontais)
    int row_por_tarefa = g_imagem.h / NUM_TASKS;
    int sobrou = g_imagem.h % NUM_TASKS; 
    int row_atual = 0;
    
    // A primeira tarefa absorve o resto das divisões se não for exata
    tarefa[0].row_start = row_atual;
    tarefa[0].row_end = row_por_tarefa + sobrou; 
    row_atual = tarefa[0].row_end;
    
    for(int i = 1; i < NUM_TASKS; i++){
        tarefa[i].row_start = row_atual;
        tarefa[i].row_end = row_atual + row_por_tarefa;
        row_atual = tarefa[i].row_end;
    }

    // Cria as Threads
    pthread_t thread[num_threads]; 
    int IDs_threads[num_threads]; 
    
    struct timeval start, end; // Usando timeval agora
    
    printf("Iniciando processamento paralelo...\n");
    gettimeofday(&start, NULL); // INICIA O CRONÔMETRO AQUI

    for(int i = 0; i < num_threads; i++){
        IDs_threads[i] = i + 1;
        if(g_mode == NEGATIVO){
            pthread_create(&thread[i], NULL, (void *)aplicar_negativo, &IDs_threads[i]);
        } else {
            pthread_create(&thread[i], NULL, (void *)aplicar_fatiamento, &IDs_threads[i]);
        }
    }

    // Aguarda todas as threads finalizarem
    for(int i = 0; i < num_threads; i++){
        pthread_join(thread[i], NULL);
    }

    gettimeofday(&end, NULL); // PARA O CRONÔMETRO AQUI
    
    // Calcula o tempo total em segundos (usando microsegundos agora)
    double tempo_gasto = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1000000.0;
    printf("Processamento finalizado em %f segundos.\n", tempo_gasto);

    sem_destroy(&semaforo);
    pthread_mutex_destroy(&mutex);

    // --- Salvando o arquivo de saída (Log de Tempos) ---
    FILE *log_file = fopen("tempos_execucao.txt", "a"); // "a" faz append (adiciona no final do arquivo sem apagar o anterior)
    if(log_file != NULL){
        fprintf(log_file, "Filtro: %s | Threads: %d | Tempo: %f segundos\n", 
                (g_mode == NEGATIVO) ? "NEGATIVO" : "SLICE", num_threads, tempo_gasto);
        fclose(log_file);
    } else {
        printf("Aviso: Nao foi possivel salvar o log de tempos no arquivo.\n");
    }

    // --- Salvando o arquivo de saída (Imagem) ---
    // Verifica se o arquivo já existe para não sobrescrever (saida1.pgm, saida2.pgm...)
    int contador = 1, verifica;
    do {
        snprintf(nome, 50, "saida%d.pgm", contador);
        verifica = 0;
        FILE* teste = fopen(nome, "rb");
        if(teste != NULL){
            fclose(teste);
            contador++;
            verifica = 1;
        }
    } while(verifica == 1);

    write_PGM(nome, &g_imagem);
    printf("Sucesso! Imagem salva como '%s'.\n", nome);

    free(g_imagem.data);
    return 0;
}

// Função executada pelas threads no modo Negativo
void aplicar_negativo(void* arg){
    int thread_id = *(int *)arg;
    
    while(1){
        sem_wait(&semaforo); 

        pthread_mutex_lock(&mutex);
        if(prox_tarefa >= NUM_TASKS){ 
            pthread_mutex_unlock(&mutex);
            sem_post(&semaforo);
            break; // Fim das tarefas, encerra a thread
        }
        int tarefa_id = prox_tarefa++;
        pthread_mutex_unlock(&mutex);

        // Processa o bloco de linhas designado para esta tarefa
        for(int i = tarefa[tarefa_id].row_start; i < tarefa[tarefa_id].row_end; i++){
            for(int j = 0; j < g_imagem.w; j++){
                int pos = i * g_imagem.w + j;
                g_imagem.data[pos] = 255 - g_imagem.data[pos];
            }
        }
        
        sem_post(&semaforo); 
    }
}

// Função executada pelas threads no modo Fatiamento
void aplicar_fatiamento(void* arg){
    int thread_id = *(int *)arg;
    
    while(1){
        sem_wait(&semaforo); 

        pthread_mutex_lock(&mutex);
        if(prox_tarefa >= NUM_TASKS){ 
            pthread_mutex_unlock(&mutex);
            sem_post(&semaforo);
            break; // Fim das tarefas, encerra a thread
        }
        int tarefa_id = prox_tarefa++;
        pthread_mutex_unlock(&mutex);

        // Processa o bloco de linhas designado para esta tarefa
        for(int i = tarefa[tarefa_id].row_start; i < tarefa[tarefa_id].row_end; i++){
            for(int j = 0; j < g_imagem.w; j++){
                int pos = i * g_imagem.w + j;
                if(g_imagem.data[pos] > g_t2 || g_imagem.data[pos] < g_t1){
                    g_imagem.data[pos] = 0;
                } else {
                    g_imagem.data[pos] = 255;
                }
            }
        }
        
        sem_post(&semaforo); 
    }
}