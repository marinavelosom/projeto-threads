/*2. Crie um programa em C que utilize o algoritmo de ordenação bubble sort de maneira concorrente.
O programa deve dividir um array em N partes, e cada thread deve ordenar uma parte. Após todas as
threads completarem a ordenação de suas respectivas partes, uma thread final deve mesclar todos os
segmentos para formar o array ordenado completo. Utilize barriers para sincronizar a conclusão das
ordenações parciais antes de começar a mesclagem.*/

#define _XOPEN_SOURCE 600 // Necessário para a função pthread_barrier_t
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

#define N 4 // Número de threads
pthread_barrier_t barrier;

// Definição da duas struct que serão úteis para passar os argumentos para as threads
typedef struct
{
    int *array; // array a ser ordenado
    int start;  // início da seção a ser ordenada
    int end;    // fim da seção a ser ordenada
    int taskid; // id da thread
} ThreadData;

typedef struct
{
    int *array;
    int Tam;
    int Nthreads;
} MergeData;

// Implementação das funções
void bubble_sort(int *array, int start, int end, int taskid);
void *thread_sort(void *arg);
void merge(int *array, int size, int parts);
void *thread_merge(void *args);

int main()
{
    int size;
    printf("Digite o tamanho do array: ");
    scanf("%d", &size);
    int array[size];
    srand(time(NULL));
    for (int i = 0; i < size; i++) // fiz o preenchimento do array de forma aleatória para testar o algoritmo, porém poderia ser feito de forma manual.
    {
        array[i] = rand() % 100;
    }
    printf("Array desordenado: ");
    for (int i = 0; i < size; i++) // mostrando o array desordenado
    {
        printf("%d ", array[i]);
    }
    printf("\n");
    pthread_t threads[N];
    ThreadData thread_data[N];

    pthread_barrier_init(&barrier, NULL, N + 1); // N + 1 para contar com a thread main

    int part_size = size / N;
    int remainder = size % N;

    for (int i = 0; i < N; i++)
    {
        thread_data[i].array = array;
        thread_data[i].start = i * part_size + (i < remainder ? i : remainder);
        thread_data[i].end = (i == N - 1) ? size : (i + 1) * part_size + (i < remainder ? i + 1 : remainder);
        thread_data[i].taskid = i;
        pthread_create(&threads[i], NULL, thread_sort, &thread_data[i]); // como é uma estrutra de dados não necessita de casting
    }

    pthread_barrier_wait(&barrier); // esperando todas as threads operárias terminarem para que a thread main possa continuar
    printf("Juncao das secoes ordenadas porem array ainda nao mesclado:");
    for (int i = 0; i < size; i++)
    {
        printf("%d ", array[i]);
    }

    printf("\n");

    MergeData merge_data;
    merge_data.array = array;
    merge_data.Tam = size;
    merge_data.Nthreads = N;
    pthread_t merge_thread;
    pthread_create(&merge_thread, NULL, thread_merge, &merge_data); // como é uma estrutura de dados não necessita de casting
    pthread_join(merge_thread, NULL);                               // esperando a thread de merge terminar
    printf("Array ordenado por meio da thread final merge_thread:");
    for (int i = 0; i < size; i++) // mostrando o array ordenado
    {
        printf("%d ", array[i]);
    }
    printf("\n");

    for (int i = 0; i < N; i++)
    {
        pthread_join(threads[i], NULL);
    }

    pthread_barrier_destroy(&barrier);

    return 0;
}

void bubble_sort(int *array, int start, int end, int taskid)
{
    for (int i = start; i < end - 1; i++) // bubble sort da seção
    {
        for (int j = start; j < end - 1 - (i - start); j++)
        {
            if (array[j] > array[j + 1])
            {
                int temp = array[j];
                array[j] = array[j + 1];
                array[j + 1] = temp;
            }
        }
    }
    printf("Thread %d ordenou: ", taskid); // mostrando a thread que ordenou
    for (int i = start; i < end; i++)      // mostrando a seçao ordenada
    {
        printf("%d ", array[i]);
    }
    printf("\n");
}

void *thread_sort(void *arg)
{
    ThreadData *data = (ThreadData *)arg;
    bubble_sort(data->array, data->start, data->end, data->taskid);
    pthread_barrier_wait(&barrier); // esperando todas as threads operárias terminarem
    return NULL;
}

void merge(int *array, int size, int parts)
{
    int *temp = (int *)malloc(size * sizeof(int));     // vetor temporário
    int *indices = (int *)malloc(parts * sizeof(int)); //  guarda os índices de início de cada parte
    int part_size = size / parts;                      // tamanho de cada parte
    int remainder = size % parts;                      // para lidar com tamanhos de vetor que não é multiplo do meu número de threads

    // Initialize the starting index of each part
    for (int i = 0; i < parts; i++)
    {
        indices[i] = i * part_size + (i < remainder ? i : remainder); // qual o início de cada parte?
    }

    for (int i = 0; i < size; i++)
    {
        int min_index = -1;          // atribuido -1 para evitar possíveis erros
        int min_value = __INT_MAX__; // garantir que o primeiro valor seja o menor

        for (int j = 0; j < parts; j++) // merge entre as partes
        {
            int start = j * part_size + (j < remainder ? j : remainder);
            int end = start + part_size + (j < remainder ? 1 : 0);

            if (indices[j] < end && indices[j] < size && array[indices[j]] < min_value)
            {
                min_value = array[indices[j]];
                min_index = j;
            }
        }

        if (min_index != -1)
        {
            temp[i] = array[indices[min_index]];
            indices[min_index]++;
        }
    }

    // copiando os elementos do vetor temporário para o original
    for (int i = 0; i < size; i++)
    {
        array[i] = temp[i];
    }

    free(temp);
    free(indices);
}

void *thread_merge(void *args)
{
    MergeData *mergedata = (MergeData *)args;

    merge(mergedata->array, mergedata->Tam, mergedata->Nthreads);

    return NULL;
}
