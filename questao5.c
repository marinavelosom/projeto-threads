#define _XOPEN_SOURCE 600

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#define M 5 //lado da matriz A e consequentemente o numero de Xi
#define Iteracoes 10

//variáveis globais para serem manipuladas em todas as funções
int matrizA[M][M];
int matrizB[M][1];
float matrizX[M][Iteracoes];

// cria barreira que vai fazer os nucleos esperarem uns aos outros
pthread_barrier_t barreira;

// cada nucleo sabe quantas threads vai executar e quais são elas(seus respectivos Xi)
typedef struct {
    int *xAssociados;
    int quantidade;
}processoNucleo;

//preenche matriz A com numeros aleatórios, se for da diagonal principal(faz divisão) impede de ser 0
void preencheMatrizes(int tamanho){
    for(int i = 0; i < tamanho; i++){
        for(int j = 0; j < tamanho; j++){
            matrizA[i][j] = (rand() % 20) - 10;
            if(matrizA[i][j] == 0 & i == j) matrizA[i][j] = (rand() % 10) + 1;
        }
        matrizB[i][0] = (rand() % 40) - 20;
    }
}

//os Xi são salvos em uma matriz onde a coluna 0 tem os Xi° a proxima tem os Xi¹ etc 
float calculaProximoX(int Xi, int Xj){

    float X;
    X = (matrizB[Xi][0])/matrizA[Xi][Xi];

    for(int colunaMatrizA = 0; colunaMatrizA < M; colunaMatrizA ++){

        if(colunaMatrizA != Xi) X -= (matrizA[Xi][colunaMatrizA]*matrizX[Xi][colunaMatrizA - 1])/matrizA[Xi][Xi];

    }

    return X;
}

/*Funcao dos nucleos, cada um chama a funcao calculaProximo dependendo de quantos X tiver associado
e salva na proxima coluna da matrizX o valor X^(K + 1)*/
void *ativaNucleo(void * informacaoX){
    processoNucleo * dados = (processoNucleo *) informacaoX;

    for(int i = 1; i < Iteracoes; i++){
        for(int j = 0; j < dados->quantidade; j++){

            printf("Calculando X%d, na %dª iteracao\n", dados->xAssociados[j] + 1, i);
            matrizX[dados->xAssociados[j]][i] = calculaProximoX(dados->xAssociados[j],i);
            printf("Fim do calculo de X%d, na %dª iteracao, x é %.2f\n", dados->xAssociados[j] + 1, i, matrizX[dados->xAssociados[j]][i]);

        }

        pthread_barrier_wait(&barreira); //espera as outras threads
    }

    pthread_exit(NULL);
}


int main(){

    srand(time(NULL));

    // enche matrizes de numeros aleatórios
    preencheMatrizes(M);

    //inicializa threads e barreira
    int N;
    scanf("%d", &N);
    pthread_barrier_init(&barreira, NULL, N);
    pthread_t threadNucleos[N];

    /*cada struct armazena quantos X cada nucleo(thread) fica responsável
    além de quantos X sao*/
    processoNucleo nucleo[N];

    int inteiro = M/N;// divide os x igualmente
    int resto = M % N;// o resto


    // faz a alocação exata da quantidade de x para cada nucleo
    for(int a = 0; a < N; a++){
        nucleo[a].xAssociados = (int *)malloc( inteiro * sizeof(int));
        nucleo[a].quantidade = inteiro;
    }
    for(int b = 0; b < resto; b++){//alguns recebem x a mais
        nucleo[b].xAssociados = realloc(nucleo[b].xAssociados, inteiro * sizeof(int) + 1);
        nucleo[b].quantidade++;
    }

    // associa cada nucleo a quais X
    int aux = 0;
    for(int c = 0; c < N; c++){
        for(int n = 0; n < nucleo[c].quantidade; n++){
            nucleo[c].xAssociados[n] = aux;
            aux++;
        }
    }

    //seta os valores de x⁰(coluna 0 de matrizX) em 1
    for(int x = 0; x < M; x++){
        matrizX[x][0] = 1;
    }

    //ativa os nucleos
    for(int i = 0; i < N; i++){
        int x = pthread_create(&threadNucleos[i], NULL, ativaNucleo, (void *) &nucleo[i]);
        if (x){         
            printf("ERRO; código de retorno é %d\n", x);         
            exit(-1);
        }
    }

    //espera os nucleos terminarem
    for(int z = 0; z < N; z++){
        pthread_join(threadNucleos[z], NULL);
    }

    pthread_barrier_destroy(&barreira);


    //libera memória usada
    for(int d = 0; d < resto; d++){
        free(nucleo[d].xAssociados);
    }

    pthread_exit(NULL);
}