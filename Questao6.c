/*6. Uma versão brasileira do Linux pretende implementar um novo algoritmo de escalonamento para threads em CPUs multicore (múltiplos núcleos).
Para isso, deve seguir os requisitos abaixo:

Uma constante N que representa a quantidade de núcleos do sistema computacional. Consequentemente, N representará a quantidade máximas de threads em execução;

lista_pronto que representará uma fila das execuções pendentes das threads;

Uma thread escalonador. Esta deverá pegar as threads da lista_pronto, e gerenciar a execução nos N núcleos.
Assuma que o código das threads são representadas por uma função qualquer que termina (ex: não tem laço infinito).
Se não houver thread para ser executada na lista_pronto, a thread escalonador dorme. Pelo menos uma thread na lista_pronto,
faz com que o escalonador acorde e coloque a nova thread  pra executar. Se por um acaso N threads estejam executando e existem threads na lista_pronto,
somente quando uma thread concluir a execução, uma nova thread será executada.

Atenção: a thread escalonador é interna do sistema operacional e escondida do usuário.

A implementação não poderá ter espera ocupada. A estrutura e funcionamento ficarão a critério da equipe, desde que siga os requisitos acima.
Por exemplo, pode-se criar uma função agendar para adicionar uma thread a lista_pronto e acordar o escalonador quando necessário.
Você deverá utilizar variáveis de condição para evitar a espera ocupada.
Lembre-se que essas variáveis precisam ser utilizadas em conjunto com mutexes.
Mutexes deverão ser utilizados de forma refinada, no sentido que um recurso não deverá travar outro recurso independente.
*/
#define _XOPEN_SOURCE 600 // necessário para poder utilizar barreiras
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h> // biblioteca padrão do unix (usei para simular o trabalho das threads por meio do sleep)
// Caso você queira trocar o  número de núcleos, basta alterar o valor de N. Para trocar o número de threads que serão agendadas,
//  basta alterar o valor de NUM_THREADS

// Naturalmente, é esperado que o número de núcleos seja menor que o número de threads agendadas, para que haja a necessidade de escalonamento.
// Caso contrário, o escalonador não terá trabalho a fazer.

#define N 3            // Número de núcleos (número máximo de threads em execução)
                       // aqui você pode alterar o número de núcleos para testar o código.
#define NUM_THREADS 14 // Número de threads a serem agendadas
                       // aqui você pode alterar o número de threads que serão agendadas
typedef struct ThreadNode
{
    int ThreadID;          // para saber qual thread está trabalhando
    void (*function)(int); // Ponteiro para a função da thread
    struct ThreadNode *next;
} ThreadNode;

ThreadNode *lista_pronto = NULL;                         // Lista de threads prontas para execução
pthread_mutex_t lista_mutex = PTHREAD_MUTEX_INITIALIZER; // mutex para garantir a segurança da lista
pthread_cond_t lista_cond = PTHREAD_COND_INITIALIZER;

int running_threads = 0;                                   // Contador de threads em execução
pthread_mutex_t running_mutex = PTHREAD_MUTEX_INITIALIZER; // tive que usar 2 mutex para garantir que não trave recursos independentes
pthread_cond_t running_cond = PTHREAD_COND_INITIALIZER;

// Prototipo da funções
void exemplo_funcao(int ThreadID);                 // Função fictícia
void *thread_function(void *arg);                  // manda a thread fazer a função fictícia
void *escalonador(void *arg);                      // escalona as threads
void agendar(void (*function)(int), int ThreadID); // adiciona a thread na lista_pronto FIFO

pthread_barrier_t barrier; // Barreira para sincronização das threads no final

int main()
{
    pthread_t escalonador_thread;

    pthread_barrier_init(&barrier, NULL, NUM_THREADS + 1); // +1 para incluir a thread principal

    // Cria a thread escalonador
    pthread_create(&escalonador_thread, NULL, escalonador, NULL);
    pthread_detach(escalonador_thread); // liberar recursos automaticamente

    // Agenda o número de threads definido anteriormente
    for (int i = 0; i < NUM_THREADS; i++) // aqui você pode alterar o número de threads que serão agendadas
    {
        agendar(exemplo_funcao, i);
        sleep(0.1); // Para que o agendamento não seja instantâneo
    }

    // Aguarda todas as threads terminarem
    pthread_barrier_wait(&barrier);
    // Destroi a barreira
    pthread_barrier_destroy(&barrier);
    printf("Todas as threads terminaram a execução.\n");
    return 0;
}
// Função executada pelas threads de trabalho
void *thread_function(void *arg)
{
    ThreadNode *node = (ThreadNode *)arg;
    void (*function)(int) = node->function;
    function(node->ThreadID);

    // Sinaliza que a thread terminou a execução
    pthread_mutex_lock(&running_mutex); // mutex utilizado para evitar a condição de disputa no número de threads em execução
    running_threads--;
    pthread_cond_signal(&running_cond);
    pthread_mutex_unlock(&running_mutex);

    pthread_barrier_wait(&barrier);

    free(node); // Libera a memória do nó
    return NULL;
}

// Função do escalonador
void *escalonador(void *arg)
{
    while (1)
    {
        pthread_mutex_lock(&lista_mutex); // pega o mutex para que possa retirar um elemento da lista_pronto com segurança

        while (lista_pronto == NULL)
        {
            pthread_cond_wait(&lista_cond, &lista_mutex); // Espera até que haja threads na lista_pronto (trabalho feito pelo agendar())
        }

        ThreadNode *node = lista_pronto;
        lista_pronto = lista_pronto->next; // Remove a thread da lista_pronto para execução (retira a primeira)

        pthread_mutex_unlock(&lista_mutex);

        // Espera até que haja um núcleo disponível
        pthread_mutex_lock(&running_mutex);
        while (running_threads >= N)
        {
            pthread_cond_wait(&running_cond, &running_mutex);
        }
        running_threads++; // quando houver um núcleo disponível, incrementa o contador de threads em execução
        pthread_mutex_unlock(&running_mutex);

        // Cria e executa a nova thread
        pthread_t thread;
        pthread_create(&thread, NULL, thread_function, node);
        pthread_detach(thread);
    }
    return NULL;
}

// Função para adicionar uma thread à lista_pronto na ordem fifo
void agendar(void (*function)(int), int ThreadID)
{
    ThreadNode *node = malloc(sizeof(ThreadNode));
    node->function = function; // pega a função que passei como parâmetro na main
    node->next = NULL;
    node->ThreadID = ThreadID;

    pthread_mutex_lock(&lista_mutex); // pega o mutex para acessar a lista e não deixar que o escalonador acesse enquanto a lista é organizada

    // Adiciona a nova thread à lista_pronto
    ThreadNode *tail = lista_pronto; // usarei essa variável para percorrer a lista sem perder a referência do início (que fica salvo em lista_pronto)
    if (tail == NULL)                // não tem nenhum elemento na lista?
    {
        lista_pronto = node; // Adiciona o primeiro elemento
    }
    else // já tem elementos na lista?
    {
        while (tail->next != NULL) // procura o último até encontrar o NULL (típico de lista)
        {
            tail = tail->next;
        }
        tail->next = node; // insere o novo elemento no final para manter o FIFO (first in, first out)
    }

    pthread_cond_signal(&lista_cond);   // Acorda o escalonador, após ter organizado a lista
    pthread_mutex_unlock(&lista_mutex); // libera o mutex para que o escalonador possa acessar a lista
}

// Função Fictícia
void exemplo_funcao(int ThreadID)
{
    printf("Thread %d executando\n", ThreadID);
    sleep(1); // Simula trabalho sendo feito
    printf("Thread %d terminou \n", ThreadID);
}
