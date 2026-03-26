/*	
*	Ilustra a criacao de threads e uso de mutex
*	Compilar com:	gcc -lpthread -o pthreads-tela pthreads-tela.c
*	ou
*			gcc -o pthreads-tela pthreads-tela.c -lpthread
*
*/

#include	<pthread.h>
#include	<stdio.h>
#include	<stdlib.h>
#include	<unistd.h>

int valor_total = 0;




void *thread_imprime_tela(void *param){
    printf("O valor total e de: %d \n",valor_total);
    pthread_exit(0);
}

void *thread_soma_constante(void *param){
    valor_total += 5;
    pthread_exit(0);
}

void *thread_le_teclado(void *param){
    printf("Digite o numero: ");
    scanf("%d", &valor_total);
    pthread_exit(0);
}

int main( int argc, char *argv[]) {

        pthread_t t1, t2, t3;
        pthread_attr_t attr;
        pthread_attr_init(&attr);

    pthread_create(&t2, &attr, thread_le_teclado, NULL);
    pthread_join( t2, NULL);

    pthread_create(&t1, &attr, thread_soma_constante, NULL);
    pthread_join( t1, NULL);

    pthread_create(&t3, &attr, thread_imprime_tela, NULL);
    pthread_join( t3, NULL);
}