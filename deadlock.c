#include <semaphore.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>

sem_t arquivo_1;
sem_t arquivo_2;

void editor_de_arquivo_A() {
    // Pega arquivo 1
    puts("Usuário A está esperando para acessar arquivo 1");
    sem_wait(&arquivo_1);

    // Processa arquivo 1
    puts("Usuário A editando arquivo 1");
    sleep(rand() % 2 + 1);

    // Pega arquivo 2
    puts("Usuário A está esperando para acessar arquivo 2");
    sem_wait(&arquivo_2);

    // Processa arquivo 2
    puts("Usuário A editando arquivo 2");
    sleep(rand() % 2 + 1);

    // Libera arquivos 1 e 2
    puts("Usuário A liberou arquivo 1");
    sem_post(&arquivo_1);
    puts("Usuário A liberou arquivo 2");
    sem_post(&arquivo_2);
}

void editor_de_arquivo_B() {
    // Pega arquivo 2
    puts("Usuário B está esperando para acessar arquivo 2");
    sem_wait(&arquivo_2);

    // Processa arquivo 2
    puts("Usuário B editando arquivo 2");
    sleep(rand() % 2 + 1);

    // Pega arquivo 1
    puts("Usuário B está esperando para acessar arquivo 1");
    sem_wait(&arquivo_1);

    // Processa arquivo 1
    puts("Usuário B editando arquivo 1");
    sleep(rand() % 2 + 1);

    // Libera arquivos 1 e 2
    puts("Usuário B liberou arquivo 1");
    sem_post(&arquivo_1);
    puts("Usuário B liberou arquivo 2");
    sem_post(&arquivo_2);
}

int main(void) {
    pthread_t thread_A, thread_B;

    sem_init(&arquivo_1, 0, 1);
    sem_init(&arquivo_2, 0, 1);

    pthread_create(&thread_A, NULL, editor_de_arquivo_A, NULL);
    pthread_create(&thread_B, NULL, editor_de_arquivo_B, NULL);

    pthread_join(thread_A, NULL);
    pthread_join(thread_B, NULL);
    sem_destroy(&arquivo_1);
    sem_destroy(&arquivo_2);
    return 0;
}
