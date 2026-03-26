/*	
*	Ilustra a criacao de threads e uso de mutex
*	Compilar com:	gcc -lpthread -o pthreads-tela pthreads-tela.c
*	ou
*			gcc -o pthreads-tela pthreads-tela.c -lpthread
*

    Faça uma aplicação que tenha um vetor de 10 valores, 
    gerados randomicamente ou com entrada do usuário. 
    Com o vetor preenchido, eles devem gerar uma soma e um produto 
    (resultado de uma multiplicação). 
    Você deve usar pelo menos duas threads para cada operação 
    (soma e multiplicação) e utilizar os dados no vetor original.
*/

#include	<pthread.h>
#include	<stdio.h>
#include	<stdlib.h>
#include	<unistd.h>

int valor_total[10] = {1,2,3,4,5,6,7,8,9,10};
pthread_mutex_t mutex_sum = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_mult = PTHREAD_MUTEX_INITIALIZER;

int sum = 0;
int mult = 1;
int i_mult = 0;
int i_sum = 0;

void *thread_imprime_tela(void *param) {
    printf("Resultado soma: %d\n",          sum);
    printf("Resultado multiplicacao: %d\n", mult);
    pthread_exit(NULL);
}

void *thread_soma_vetorial(void *param){
    while(1){
        pthread_mutex_lock(&mutex_sum);
        if(i_sum < 10){
            sum += valor_total[i_sum];
            i_sum++;
            pthread_mutex_unlock(&mutex_sum); 
        }
        else{
            pthread_mutex_unlock(&mutex_sum);
            pthread_exit(0);
        }
    }
}


void *thread_produto_vetorial(void *param){
        while(1){
        pthread_mutex_lock(&mutex_mult);
        if(i_mult < 10){
            mult *= valor_total[i_mult];
            i_mult++;
            pthread_mutex_unlock(&mutex_mult); 
        }
        else{
            pthread_mutex_unlock(&mutex_mult);
            pthread_exit(0);
        }
    }
}

int main( int argc, char *argv[]) {

    pthread_t soma_1, soma_2, produto_1, produto_2, leitura;
    pthread_attr_t attr;
    pthread_attr_init(&attr);

    pthread_create(&soma_1, &attr, thread_soma_vetorial, NULL);
    

    pthread_create(&soma_2, &attr, thread_soma_vetorial, NULL);


    pthread_create(&produto_1, &attr, thread_produto_vetorial, NULL);
    

    pthread_create(&produto_2, &attr, thread_produto_vetorial, NULL);


    

    pthread_join( soma_1, NULL);
    pthread_join( soma_2, NULL);
    pthread_join( produto_1, NULL);
    pthread_join( produto_2, NULL);
    printf("soma , multiplicacao: %d, %d", sum, mult);
}