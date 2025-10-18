#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// Desafio Detective Quest - Implementação do mapa da mansão
// Este arquivo implementa uma árvore binária simples onde cada nó representa uma sala.

// =========================
// Estrutura: Sala (nó da árvore)
// =========================
// Cada sala possui um nome (string alocada dinamicamente) e dois ponteiros
// para as salas à esquerda e à direita.
typedef struct Sala {
    char *nome;
    struct Sala *esquerda;
    struct Sala *direita;
} Sala;

// criarSala() – cria, de forma dinâmica, uma sala com nome.
// Parâmetros: const char *nome - texto com o nome da sala
// Retorno: ponteiro para Sala recém-alocada (com cópia do nome)
Sala *criarSala(const char *nome) {
    Sala *s = (Sala *)malloc(sizeof(Sala));
    if (!s) {
        fprintf(stderr, "Erro de alocação de memória para Sala\n");
        exit(EXIT_FAILURE);
    }
    // usar versão segura de strdup para portabilidade
    extern char *strdup(const char *);
    char *temp_nome = NULL;
#if defined(_POSIX_C_SOURCE) || defined(_BSD_SOURCE) || defined(_GNU_SOURCE)
    temp_nome = strdup(nome);
#else
    // fallback simples quando strdup não está declarado
    size_t _len = strlen(nome) + 1;
    temp_nome = (char *)malloc(_len);
    if (temp_nome) memcpy(temp_nome, nome, _len);
#endif
    s->nome = temp_nome;
    if (!s->nome) {
        fprintf(stderr, "Erro de alocação de memória para nome da Sala\n");
        free(s);
        exit(EXIT_FAILURE);
    }
    s->esquerda = NULL;
    s->direita = NULL;
    return s;
}

// conectarSalas() – conecta um nó pai às salas filhas esquerda e direita.
void conectarSalas(Sala *pai, Sala *esq, Sala *dir) {
    if (!pai) return;
    pai->esquerda = esq;
    pai->direita = dir;
}

// liberarArvore() – libera recursivamente a árvore e os nomes alocados.
void liberarArvore(Sala *raiz) {
    if (!raiz) return;
    liberarArvore(raiz->esquerda);
    liberarArvore(raiz->direita);
    free(raiz->nome);
    free(raiz);
}

// explorarSalas() – permite a navegação do jogador pela árvore.
// O jogador começa em 'raiz' e pode escolher: 'e' para esquerda, 'd' para direita, 's' para sair.
// A função exibe o nome da sala a cada movimento e, ao final, lista as salas visitadas.
void explorarSalas(Sala *raiz) {
    if (!raiz) {
        printf("Mapa vazio. Nada para explorar.\n");
        return;
    }

    // vetor dinâmico para guardar os nomes visitados (apontadores para cópias)
    char **visitados = NULL;
    size_t visit_count = 0;

    Sala *atual = raiz;
    char entrada[64];

    while (atual) {
        printf("\nVocê está em: %s\n", atual->nome);

        // registra visita (fazer cópia do nome para manter histórico independente da liberação)
    char *copia = NULL;
#if defined(_POSIX_C_SOURCE) || defined(_BSD_SOURCE) || defined(_GNU_SOURCE)
    copia = strdup(atual->nome);
#else
    size_t _l = strlen(atual->nome) + 1;
    copia = (char *)malloc(_l);
    if (copia) memcpy(copia, atual->nome, _l);
#endif
        if (!copia) {
            fprintf(stderr, "Erro alocando memória para histórico de visitas\n");
            break;
        }
        char **tmp = realloc(visitados, (visit_count + 1) * sizeof(char *));
        if (!tmp) {
            fprintf(stderr, "Erro alocando memória para histórico de visitas\n");
            free(copia);
            break;
        }
        visitados = tmp;
        visitados[visit_count++] = copia;

        // verifica se é folha
        if (!atual->esquerda && !atual->direita) {
            printf("Você alcançou um cômodo sem mais caminhos (nó-folha). Exploração encerrada.\n");
            break;
        }

        // mostra opções disponíveis
        printf("Escolha uma direção: ");
        if (atual->esquerda) printf("(e) esquerda ");
        if (atual->direita) printf("(d) direita ");
        printf("(s) sair\n");

        // lê entrada do usuário
        if (!fgets(entrada, sizeof(entrada), stdin)) {
            // EOF ou erro
            printf("Entrada encerrada. Saindo...\n");
            break;
        }

        // interpreta primeira letra
        char opc = tolower((unsigned char)entrada[0]);
        if (opc == 's') {
            printf("Saindo da exploração conforme solicitado.\n");
            break;
        } else if (opc == 'e') {
            if (atual->esquerda) {
                atual = atual->esquerda;
            } else {
                printf("Caminho à esquerda indisponível. Escolha outra opção.\n");
            }
        } else if (opc == 'd') {
            if (atual->direita) {
                atual = atual->direita;
            } else {
                printf("Caminho à direita indisponível. Escolha outra opção.\n");
            }
        } else {
            printf("Opção inválida. Use 'e', 'd' ou 's'.\n");
        }
    }

    // exibe histórico de salas visitadas
    if (visit_count > 0) {
        printf("\nSalas visitadas (%zu):\n", visit_count);
        for (size_t i = 0; i < visit_count; ++i) {
            printf("%zu. %s\n", i + 1, visitados[i]);
            free(visitados[i]);
        }
        free(visitados);
    }
}

int main() {

    // 🌱 Nível Novato: Mapa da Mansão com Árvore Binária
    // Montamos a árvore de forma manual (alocação dinâmica com criarSala())
    // Estrutura da mansão (exemplo):
    //                 Hall de Entrada
    //                /               
    //         Biblioteca            Cozinha
    //         /      \              /     
    //     Sótão   Escritório    Jardim  Sala de Jantar

    Sala *hall = criarSala("Hall de Entrada");
    Sala *biblioteca = criarSala("Biblioteca");
    Sala *cozinha = criarSala("Cozinha");
    Sala *sotao = criarSala("Sótão");
    Sala *escritorio = criarSala("Escritório");
    Sala *jardim = criarSala("Jardim");
    Sala *salaJantar = criarSala("Sala de Jantar");

    conectarSalas(hall, biblioteca, cozinha);
    conectarSalas(biblioteca, sotao, escritorio);
    conectarSalas(cozinha, jardim, salaJantar);

    printf("Bem-vindo(a) ao Detective Quest - Exploração da Mansão\n");
    printf("Começando no Hall de Entrada. Navegue com 'e' (esquerda), 'd' (direita) ou 's' (sair).\n");

    explorarSalas(hall);

    // libera memória
    liberarArvore(hall);

    return 0;
}

