// QUESTÃO 1. 
// Desenvolva um programa que conte a ocorrência de uma palavra específica em X arquivos de texto. 
// O programa deve usar threads, onde cada thread é responsável por buscar a palavra em um arquivo diferente. Use mutexes para atualizar um contador global de forma segura. 
// O programa deve imprimir o total de ocorrências da palavra em todos os arquivos.

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

#define QDT_ARQUIVOS 3
#define TAM_MAX_PALAVRA 100

// definindo palavra 
const char *palavra_procurada = "programa";

// contador de palavras
int total_leituras = 0;

// mutex 
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

// função para contar a leitura da palavra em um arquivo 
void *contar_leituras_palavra(void *arquivo) {
    char *caminho = (char *)arquivo;
    int ocorrencias = 0;

    // abre arquivo
    FILE *file = fopen(caminho, "r");
    if (file == NULL) {
        perror("Erro ao abrir o arquivo");
        pthread_exit(NULL);
    }

    char palavra[TAM_MAX_PALAVRA];
    // ler cada palavra do arquivo
    while (fscanf(file, "%s", palavra) != EOF) {
        // verifica se é a palavra
        if (strcmp(palavra, palavra_procurada) == 0) {
            ocorrencias++;
        }
    }

    // atualiza o contador global com mutex
    pthread_mutex_lock(&mutex);
    total_leituras += ocorrencias;
    pthread_mutex_unlock(&mutex);

    fclose(file);
    pthread_exit(NULL);
}

int main() {
    // arquivos a serem lidos
    char *caminhos_arquivos[QDT_ARQUIVOS] = {"arquivo1.txt", "arquivo2.txt", "arquivo3.txt"};

    pthread_t threads[QDT_ARQUIVOS];

    // cria e inicia uma thread para cada arquivo
    for (int i = 0; i < QDT_ARQUIVOS; i++) {
        if (pthread_create(&threads[i], NULL, contar_leituras_palavra, (void *)caminhos_arquivos[i]) != 0) {
            perror("Erro ao criar a thread");
            exit(EXIT_FAILURE);
        }
    }

    // aguarda todas as threads terminarem
    for (int i = 0; i < QDT_ARQUIVOS; i++) {
        if (pthread_join(threads[i], NULL) != 0) {
            perror("Erro ao esperar a thread");
            exit(EXIT_FAILURE);
        }
    }

    // imprime o total de ocorrências da palavra em todos os arquivos
    printf("Quantidade '%s' encontradas: %d\n", palavra_procurada, total_leituras);

    return 0;
}
