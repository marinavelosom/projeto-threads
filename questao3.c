#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

//disparidade grande entre contas e clientes para demonstrar exclusão mútua
#define NUM_CLIENTES 200
#define NUM_CONTAS 20

pthread_mutex_t mutexes[NUM_CONTAS]; //para cada conta um mutex

typedef struct{
    int id;
    float saldo;
}Conta;

Conta contas[NUM_CONTAS];

//struct que é parametro da funcao executada pela thread banco, nela tem informacao suficiente para acessar o struct conta
typedef struct{
    int acao; // 0 deposito, 1 saque 2 consulta saldo
    int id_conta;
    int id_cliente;
}Cliente;

Cliente clientes[NUM_CLIENTES];


//thread chamada pela thread banco para fazer depósito
void* fazDeposito(void* id_cliente){
    int* id = (int *)id_cliente;
    int valor_deposito = (rand() % 1000) + 500;

    pthread_mutex_lock(&mutexes[clientes[*id].id_conta]);
    float saldoAntigo = contas[clientes[*id].id_conta].saldo;
    contas[clientes[*id].id_conta].saldo += valor_deposito;
    printf("Cliente %d atendido, Saldo novo: %f, Saldo antigo: %f, depósito de %dReais\n", *id, contas[clientes[*id].id_conta].saldo, saldoAntigo, valor_deposito);
    pthread_mutex_unlock(&mutexes[clientes[*id].id_conta]);

    pthread_exit(NULL);
}

//thread chamada pela thread banco para fazer depósito
void* fazSaque(void * id_cliente){
    int* id = (int*)id_cliente;
    int valor_saque = (rand() % 1000) + 500;

    pthread_mutex_lock(&mutexes[clientes[*id].id_conta]);
    if(valor_saque <= contas[clientes[*id].id_conta].saldo){
        float saldoAntigo = contas[clientes[*id].id_conta].saldo;
        contas[clientes[*id].id_conta].saldo -= valor_saque;
        printf("Cliente %d atendido, Saldo novo: %f, Saldo antigo: %f, depósito de %dReais\n", *id, contas[clientes[*id].id_conta].saldo, saldoAntigo, valor_saque);
        pthread_mutex_unlock(&mutexes[clientes[*id].id_conta]);
    }else{
        printf("Valor insuficiente, saque do cliente %d cancelado\n", *id);
    }
    pthread_mutex_unlock(&mutexes[clientes[*id].id_conta]);

    pthread_exit(NULL);
}

//thread chamada pela thread banco para fazer depósito
void* checaSaldo(void * id_cliente){
    int* id = (int*)id_cliente;

    //mutex ate para printar o saldo pois pode ocorrer de outra thread estar atualizando o saldo no mesmo momento
    pthread_mutex_lock(&mutexes[clientes[*id].id_conta]);
    printf("Saldo da conta %d do cliente %d: %f\n", clientes[*id].id_conta, clientes[*id].id_cliente, contas[clientes[*id].id_conta].saldo);
    pthread_mutex_unlock(&mutexes[clientes[*id].id_conta]);

    pthread_exit(NULL);
}

//thread principal que cria outras threads
void* acaoBanco(void * stru){

    Cliente * clientes = (Cliente *) stru;

    pthread_t clientesBanco[NUM_CLIENTES];

    //checa qual acao vai ser feita por cada cliente e chama thread correspondente
    for(int i = 0; i< NUM_CLIENTES; i++){
        if(clientes[i].acao == 0){
            int x;
            printf("Atendendo cliente %d, depósito\n", i);
            x = pthread_create(&clientesBanco[i], NULL, fazDeposito, (void *) &clientes[i].id_cliente);
            if (x){         
                printf("ERRO; código de retorno é %d\n", x);         
                exit(-1);      
            }

        }
        else if(clientes[i].acao == 1){
            int y;
            printf("Atendendo cliente %d, saque\n", i);
            y = pthread_create(&clientesBanco[i], NULL, fazSaque, (void *) &clientes[i].id_cliente);
            if (y){         
                printf("ERRO; código de retorno é %d\n", y);         
                exit(-1);      
            }
        }
        else if(clientes[i].acao == 2){
            int z;
            printf("Atendendo cliente %d, saque\n", i);
            z = pthread_create(&clientesBanco[i], NULL, checaSaldo, (void *) &clientes[i].id_cliente);
            if (z){         
                printf("ERRO; código de retorno é %d\n", z);         
                exit(-1);      
            }
        }
    }

    for(int j = 0; j <NUM_CLIENTES; j++){
        pthread_join(clientesBanco[j], NULL);
    }

    pthread_exit(NULL);
}

int main (){

    // garantir números aleatórios a cada compilação
    srand(time(NULL));

    for(int m = 0; m < NUM_CONTAS; m++){
        pthread_mutex_init(&mutexes[m], NULL);
    }

    //gera contas aleatórias
    for(int i = 0; i < NUM_CONTAS; i++){
        contas[i].id = i;
        contas[i].saldo = rand() % 5000;
    }

    //associa a cada cliente uma conta e uma acao a ser feita
    for(int j = 0; j < NUM_CLIENTES; j++){
        clientes[j].id_conta = rand() % NUM_CONTAS;
        clientes[j].acao = rand() % 3;
        clientes[j].id_cliente = j;
    }

    //apenas uma thread sai da main e de lá são chamadas outras para cada ação
    pthread_t banco;     

    int a;
    a = pthread_create(&banco, NULL, acaoBanco, (void *) clientes);      
    if (a){         
        printf("ERRO; código de retorno é %d\n", a);         
        exit(-1);      
    }

    for(int n = 0; n < NUM_CONTAS; n++){
        pthread_mutex_destroy(&mutexes[n]);
    }


    pthread_exit(NULL);
}