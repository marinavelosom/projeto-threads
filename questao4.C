#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#define TAM 9

typedef struct {
    int linha;
    int coluna;
    int (*tab)[TAM];
} Parametros;

typedef struct {
    int indice;
    int status;
    char tipo;
} Resultado;

Resultado resultados[TAM * 3]; // Array para armazenar o resultado de cada thread

void *verificarLinha(void *params) {
    Parametros *dados = (Parametros *) params;
    int linha = dados->linha;
    int verificadorColunas[TAM] = {0};

    for (int coluna = 0; coluna < TAM; coluna++) {
        int numero = dados->tab[linha][coluna];
        if (numero < 1 || numero > 9 || verificadorColunas[numero - 1] == 1) {
            resultados[linha].indice = linha;
            resultados[linha].status = 0;
            resultados[linha].tipo = 'L';
            pthread_exit(NULL);
        }
        verificadorColunas[numero - 1] = 1;
    }
    resultados[linha].indice = linha;
    resultados[linha].status = 1;
    resultados[linha].tipo = 'L';
    pthread_exit(NULL);
}

void *verificarColuna(void *params) {
    Parametros *dados = (Parametros *) params;
    int coluna = dados->coluna;
    int verificadorLinhas[TAM] = {0};

    for (int linha = 0; linha < TAM; linha++) {
        int numero = dados->tab[linha][coluna];
        if (numero < 1 || numero > 9 || verificadorLinhas[numero - 1] == 1) {
            resultados[TAM + coluna].indice = coluna;
            resultados[TAM + coluna].status = 0;
            resultados[TAM + coluna].tipo = 'C';
            pthread_exit(NULL);
        }
        verificadorLinhas[numero - 1] = 1;
    }
    resultados[TAM + coluna].indice = coluna;
    resultados[TAM + coluna].status = 1;
    resultados[TAM + coluna].tipo = 'C';
    pthread_exit(NULL);
}

void *verificarSubgrade(void *params) {
    Parametros *dados = (Parametros *) params;
    int linhaInicial = dados->linha;
    int colunaInicial = dados->coluna;
    int verificadorGrade[TAM] = {0};
    int indice = 2 * TAM + (linhaInicial / 3) * 3 + colunaInicial / 3;

    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            int numero = dados->tab[linhaInicial + i][colunaInicial + j];
            if (numero < 1 || numero > 9 || verificadorGrade[numero - 1] == 1) {
                resultados[indice].indice = indice;
                resultados[indice].status = 0;
                resultados[indice].tipo = 'G';
                pthread_exit(NULL);
            }
            verificadorGrade[numero - 1] = 1;
        }
    }
    resultados[indice].indice = indice;
    resultados[indice].status = 1;
    resultados[indice].tipo = 'G';
    pthread_exit(NULL);
}

int main() {

    int tabuleiro[TAM][TAM];
    char entrada[TAM * TAM + 1]; // String para armazenar a entrada do usuário

    // Receber o tabuleiro do usuário
    printf("Digite o tabuleiro de Sudoku como uma única linha (81 números de 1 a 9, sem espaços):\n");
    scanf("%81s", entrada);


    pthread_t threads[TAM * 3];
    Parametros dados[TAM * 3];
    int indiceThread = 0;

    // Criar threads para verificar linhas
    for (int i = 0; i < TAM; i++) {
        dados[indiceThread].linha = i;
        dados[indiceThread].tab = tab;
        pthread_create(&threads[indiceThread], NULL, verificarLinha, (void *) &dados[indiceThread]);
        indiceThread++;
    }

    // Criar threads para verificar colunas
    for (int i = 0; i < TAM; i++) {
        dados[indiceThread].coluna = i;
        dados[indiceThread].tab = tab;
        pthread_create(&threads[indiceThread], NULL, verificarColuna, (void *) &dados[indiceThread]);
        indiceThread++;
    }

    // Criar threads para verificar subgrades
    for (int i = 0; i < TAM; i += 3) {
        for (int j = 0; j < TAM; j += 3) {
            dados[indiceThread].linha = i;
            dados[indiceThread].coluna = j;
            dados[indiceThread].tab = tab;
            pthread_create(&threads[indiceThread], NULL, verificarSubgrade, (void *) &dados[indiceThread]);
            indiceThread++;
        }
    }

    // Esperar todas as threads terminarem
    for (int i = 0; i < TAM * 3; i++) {
        pthread_join(threads[i], NULL);
    }

    // Verificar os resultados e imprimir erros, se houver
    int tudoCorreto = 1;
    for (int i = 0; i < TAM * 3; i++) {
        if (resultados[i].status == 0) {
            tudoCorreto = 0;
            if (resultados[i].tipo == 'L') {
                printf("Erro na linha %d\n", resultados[i].indice);
            } else if (resultados[i].tipo == 'C') {
                printf("Erro na coluna %d\n", resultados[i].indice);
            } else if (resultados[i].tipo == 'G') {
                printf("Erro na subgrade %d\n", resultados[i].indice -17);
            }
        }
    }

    if (tudoCorreto) {
        printf("A solução do Sudoku está correta.\n");
    } else {
        printf("A solução do Sudoku tem erros.\n");
    }

    return 0;
}
