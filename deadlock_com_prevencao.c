#include <semaphore.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>

// Limite de 5 segundos para ser considerado deadlock
#define DEADLOCK_TIMEOUT 5

typedef struct {
    char nome[50];
    sem_t *semaforo;
} recurso;

typedef struct {
    recurso recurso;
    int foi_liberado;
    time_t tempo;
} recurso_status;

typedef struct {
    int qtd_recursos;
    recurso *recursos;
} detector_arg;

void *detectar_deadlock(void *arg) {
    const int qtd_recursos = ((detector_arg *) arg)->qtd_recursos;
    const recurso *recursos = ((detector_arg *) arg)->recursos;

    // Criar status de recursos
    recurso_status *recursos_status = malloc(sizeof(recurso_status) * qtd_recursos);
    for (int i = 0; i < qtd_recursos; i++) {
        recursos_status[i].foi_liberado = 0;
        recursos_status[i].tempo = time(NULL);
        recursos_status[i].recurso = recursos[i];
    }

    while (1) {
        for (int i = 0; i < qtd_recursos; i++) {
            recurso_status *status = &recursos_status[i];
            sem_getvalue(status->recurso.semaforo, &status->foi_liberado);
            if (status->foi_liberado) {
                status->tempo = time(NULL);
            } else {
                if (time(NULL) - status->tempo > DEADLOCK_TIMEOUT
                ) {
                    sem_post(status->recurso.semaforo);
                    // Reseta temporizador de todos recursos
                    for (int j = 0; j < qtd_recursos; j++) {
                        recursos_status[j].tempo = time(NULL);
                    }
                    printf("%s em deadlock. Liberando %s\n", status->recurso.nome, status->recurso.nome);
                }
            }
        }
    }
}


typedef struct {
    char usuario[50];
    recurso recursos[2];
} editor_arg;

void editor_de_arquivo(void *arg) {
    char *usuario = ((editor_arg *) arg)->usuario;
    recurso *recursos = ((editor_arg *) arg)->recursos;
    // Pega arquivo 1
    printf("Usuário %s está esperando para acessar %s\n", usuario, recursos[0].nome);
    sem_wait(recursos[0].semaforo);

    // Processa arquivo 1
    printf("Usuário %s editando %s\n", usuario, recursos[0].nome);
    sleep(rand() % 2 + 1);

    // Pega arquivo 2
    printf("Usuário %s está esperando para acessar %s\n", usuario, recursos[1].nome);
    sem_wait(recursos[1].semaforo);

    // Processa arquivo 2
    printf("Usuário %s editando %s\n", usuario, recursos[1].nome);
    sleep(rand() % 2 + 1);

    // Libera arquivos 1 e 2
    printf("Usuário %s liberou %s\n", usuario, recursos[0].nome);
    sem_post(recursos[0].semaforo);
    printf("Usuário %s liberou %s\n", usuario, recursos[1].nome);
    sem_post(recursos[1].semaforo);
}

int main(void) {
    sem_t arquivo_1, arquivo_2;
    pthread_t thread_A, thread_B, detector_deadlock;

    sem_init(&arquivo_1, 0, 1);
    sem_init(&arquivo_2, 0, 1);

    const recurso recurso_1 = {.nome = "Arquivo 1", .semaforo = &arquivo_1};
    const recurso recurso_2 = {.nome = "Arquivo 2", .semaforo = &arquivo_2};

    // Passar os recursos na mesma ordem para todas as threads evita que elas caiam em deadlock
    recurso recursos[2];
    recursos[0] = recurso_1;
    recursos[1] = recurso_2;

    pthread_create(&thread_A, NULL, editor_de_arquivo, &(editor_arg){
                       .usuario = "A",
                       .recursos = recursos // ← Os recursos são passados na mesma ordem
                   });
    pthread_create(&thread_B, NULL, editor_de_arquivo, &(editor_arg){
                       .usuario = "B",
                       .recursos = recursos // ← Os recursos são passados na mesma ordem
                   });

    pthread_create(&detector_deadlock, NULL, detectar_deadlock,
                   &(detector_arg){.qtd_recursos = 2, .recursos = recursos});

    pthread_join(thread_A, NULL);
    pthread_join(thread_B, NULL);
    pthread_cancel(detector_deadlock);
    sem_destroy(&arquivo_1);
    sem_destroy(&arquivo_2);
    return 0;
}
