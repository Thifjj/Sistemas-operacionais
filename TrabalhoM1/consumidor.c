#include "config.h" // Seu arquivo de structs e funcoes
#include <sys/time.h>

#define NUM_TASKS 256
#define NUM_THREADS 2

// Controle de concorrência
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
int prox_tarefa = 0;

// Dados compartilhados para processamento
PGM imagem;
Task task[NUM_TASKS];
int mode;         // NEGATIVO (0) ou SLICE (1)
int t1, t2;     // Limites inferior e superior para fatiamento

// Protótipos das funções das threads
void *aplicar_negativo(void* arg);
void *aplicar_fatiamento(void* arg);

int main(int argc, char* argv[]) {
    // Verificação básica de argumentos
    if(argc < 2) {
        printf("Uso Negativo: 0 [num_threads]\n");
        printf("Uso Fatiamento:  1 [num_threads] <t1> <t2> \n");
        return 1;
    }

    Header cabecalho;
    int num_threads = NUM_THREADS; // Valor padrão

    const char* path = FIFO_PATH;
    int modo = atoi(argv[1]);

    // Configuração baseada nos argumentos de linha de comando
    if(modo == NEGATIVO) {
        mode = NEGATIVO;
        if(argc >= 3) {
            num_threads = atoi(argv[2]); // num_threads em argv[2]
        }
    } else if(modo == SLICE) {
        mode = SLICE;
        if(argc < 5) {
            printf("Erro: Fatiamento exige Modo, Threads, T1 e T2.\n");
            printf("Uso: 1 [num_threads] <t1> <t2>\n");
            return 1;
        }
        num_threads = atoi(argv[2]); // num_threads em argv[2]
        t1 = atoi(argv[3]);        // t1 em argv[3]
        t2 = atoi(argv[4]);        // t2 em argv[4]
    } else {
        printf("Modo invalido.\n");
        return 1;
    }

    printf("Consumidor: Preparando %d threads.\n", num_threads);

    // Abre a FIFO apenas para leitura
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

    imagem.w = cabecalho.w;
    imagem.h = cabecalho.h;
    imagem.maxv = cabecalho.maxv;

    // Aloca a memória e lê os pixels do FIFO
    imagem.data = (unsigned char*)malloc(imagem.w * imagem.h * sizeof(unsigned char));
    size_t bytes_totais = (size_t)imagem.w * imagem.h;
    size_t bytes_lido = 0;

    while(bytes_lido < bytes_totais) {
        ssize_t n = read(fd, imagem.data + bytes_lido, bytes_totais - bytes_lido);
        if (n <= 0) break;
        bytes_lido += n;
    }
    close(fd);

    // Divide a imagem em tarefas e por linha
    int linhas_task = imagem.h / NUM_TASKS;
    int resto_task = imagem.h % NUM_TASKS;
    int linha_atual = 0;

    for(int i = 0; i < NUM_TASKS; i++){
        task[i].row_start = linha_atual;
        int extra = (i == 0) ? resto_task : 0;
        task[i].row_end = linha_atual + linhas_task + extra;
        linha_atual = task[i].row_end;
    }

    // Cria as Threads
    pthread_t thread[num_threads];
    pthread_attr_t attr; /* set of attributes for the thread */
    int thread_ids[num_threads];

    struct timeval start, end;
    gettimeofday(&start, NULL);
    pthread_attr_init(&attr);

    for(int i = 0; i < num_threads; i++){
        thread_ids[i] = i; // Atribui um ID para cada thread
        if(mode == NEGATIVO){
            pthread_create(&thread[i], &attr, aplicar_negativo, &thread_ids[i]);
        } else {
            pthread_create(&thread[i], &attr, aplicar_fatiamento, &thread_ids[i]);
        }
    }

    for(int i = 0; i < num_threads; i++){
        pthread_join(thread[i], NULL);
    }

    gettimeofday(&end, NULL);
    double tempo_gasto = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1000000.0;

    // Log de Tempos [cite: 18]
    FILE *log_file = fopen("tempos_execucao.txt", "a");
    if(log_file != NULL){
        fprintf(log_file, "Filtro: %s | Threads: %d | Tempo: %f segundos\n",
                (mode == NEGATIVO) ? "NEGATIVO" : "SLICE", num_threads, tempo_gasto);
        fclose(log_file);
    }

    char nome_saida[30];
    // Salva imagem final
    int contador = 1;
    do {
        snprintf(nome_saida, sizeof(nome_saida), "saida%d.pgm", contador++);
    } while(access(nome_saida, F_OK) == 0);

    write_PGM(nome_saida, &imagem);
    printf("Processamento em %f s. Imagem: %s\n", tempo_gasto, nome_saida);

    pthread_mutex_destroy(&mutex);
    free(imagem.data);
    return 0;
}

// Funções de processamento utilizando Mutex
void *aplicar_negativo(void* arg){
    int thread_id = *(int *)arg;
    while(1){
        pthread_mutex_lock(&mutex);
        if(prox_tarefa >= NUM_TASKS){
            pthread_mutex_unlock(&mutex);
            break;
        }
        int tarefa_id = prox_tarefa++;
        pthread_mutex_unlock(&mutex);

        for(int i = task[tarefa_id].row_start; i < task[tarefa_id].row_end; i++){
            for(int j = 0; j < imagem.w; j++){
                int pos = i * imagem.w + j;
                imagem.data[pos] = 255 - imagem.data[pos]; // s = 255 - r
            }
        }
    }
}

void *aplicar_fatiamento(void* arg){
    int thread_id = *(int *)arg;
    while(1){
        pthread_mutex_lock(&mutex);
        if(prox_tarefa >= NUM_TASKS){
            pthread_mutex_unlock(&mutex);
            break;
        }
        int tarefa_id = prox_tarefa++;
        pthread_mutex_unlock(&mutex);

        for(int i = task[tarefa_id].row_start; i < task[tarefa_id].row_end; i++){
            for(int j = 0; j < imagem.w; j++){
                int pos = i * imagem.w + j;
                // Lógica de fatiamento
                if(imagem.data[pos] > t2 || imagem.data[pos] < t1){
                    imagem.data[pos] = 0;
                } else {
                    imagem.data[pos] = 255;
                }
            }
        }
    }
}
