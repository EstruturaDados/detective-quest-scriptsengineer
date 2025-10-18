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
    char *pista; // pista opcional presente na sala
    struct Sala *esquerda;
    struct Sala *direita;
} Sala;

// strdup_safe() - cópia de string com fallback quando strdup não estiver disponível
static char *strdup_safe(const char *s) {
    if (!s) return NULL;
#if defined(_POSIX_C_SOURCE) || defined(_BSD_SOURCE) || defined(_GNU_SOURCE)
    return strdup(s);
#else
    size_t n = strlen(s) + 1;
    char *p = (char *)malloc(n);
    if (p) memcpy(p, s, n);
    return p;
#endif
}

// criarSala() – cria, de forma dinâmica, uma sala com nome.
// Parâmetros: const char *nome - texto com o nome da sala
// Retorno: ponteiro para Sala recém-alocada (com cópia do nome)
// criarSala() – cria, de forma dinâmica, uma sala com nome e pista opcional.
// Parâmetros: const char *nome - nome da sala
//             const char *pista - texto da pista (pode ser NULL)
// Retorno: ponteiro para Sala recém-alocada (com cópia do nome e da pista)
Sala *criarSala(const char *nome, const char *pista) {
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
    // copia da pista, se fornecida
    if (pista) {
        s->pista = strdup_safe(pista);
        if (!s->pista) {
            fprintf(stderr, "Erro alocando pista para Sala\n");
            free(s->nome);
            free(s);
            exit(EXIT_FAILURE);
        }
    } else {
        s->pista = NULL;
    }
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
    free(raiz->pista);
    free(raiz);
}

// explorarSalas() – permite a navegação do jogador pela árvore.
// O jogador começa em 'raiz' e pode escolher: 'e' para esquerda, 'd' para direita, 's' para sair.
// A função exibe o nome da sala a cada movimento e, ao final, lista as salas visitadas.
// Estrutura para a árvore de pistas (BST)
typedef struct PistaNode {
    char *texto;
    struct PistaNode *esq;
    struct PistaNode *dir;
} PistaNode;

// inserirPista() – insere uma nova pista na BST de forma ordenada.
// Ignora inserção de duplicatas exatas.
PistaNode *inserirPista(PistaNode *raiz, const char *texto) {
    if (!texto) return raiz;
    if (!raiz) {
        PistaNode *n = (PistaNode *)malloc(sizeof(PistaNode));
        if (!n) { fprintf(stderr, "Erro alocando nó de pista\n"); exit(EXIT_FAILURE); }
        n->texto = strdup_safe(texto);
        if (!n->texto) { fprintf(stderr, "Erro alocando texto da pista\n"); exit(EXIT_FAILURE); }
        n->esq = n->dir = NULL;
        return n;
    }
    int cmp = strcmp(texto, raiz->texto);
    if (cmp == 0) return raiz; // já existe
    if (cmp < 0) raiz->esq = inserirPista(raiz->esq, texto);
    else raiz->dir = inserirPista(raiz->dir, texto);
    return raiz;
}

// exibirPistas() – percorre a BST em ordem e imprime as pistas em ordem alfabética.
void exibirPistas(PistaNode *raiz) {
    if (!raiz) return;
    exibirPistas(raiz->esq);
    printf("- %s\n", raiz->texto);
    exibirPistas(raiz->dir);
}

// liberarPistas() – libera memória da BST de pistas.
void liberarPistas(PistaNode *raiz) {
    if (!raiz) return;
    liberarPistas(raiz->esq);
    liberarPistas(raiz->dir);
    free(raiz->texto);
    free(raiz);
}

// --------------------
// Tabela hash simples para mapear pista -> suspeito
// --------------------
typedef struct HashEntry {
    char *chave;      // texto da pista
    char *suspeito;   // nome do suspeito associado
    struct HashEntry *prox;
} HashEntry;

#define HASH_SIZE 101

// função de hash simples: soma de caracteres modulo tamanho
static unsigned int hash_func(const char *s) {
    unsigned int h = 0;
    while (*s) h = h * 31 + (unsigned char)*s++;
    return h % HASH_SIZE;
}

// inserirNaHash() – associa uma pista a um suspeito (substitui se já existir)
void inserirNaHash(HashEntry **tabela, const char *pista, const char *suspeito) {
    unsigned int h = hash_func(pista);
    HashEntry *e = tabela[h];
    while (e) {
        if (strcmp(e->chave, pista) == 0) {
            free(e->suspeito);
            e->suspeito = strdup_safe(suspeito);
            return;
        }
        e = e->prox;
    }
    HashEntry *novo = (HashEntry *)malloc(sizeof(HashEntry));
    novo->chave = strdup_safe(pista);
    novo->suspeito = strdup_safe(suspeito);
    novo->prox = tabela[h];
    tabela[h] = novo;
}

// encontrarSuspeito() – retorna o nome do suspeito associado a uma pista (ou NULL)
const char *encontrarSuspeito(HashEntry **tabela, const char *pista) {
    unsigned int h = hash_func(pista);
    HashEntry *e = tabela[h];
    while (e) {
        if (strcmp(e->chave, pista) == 0) return e->suspeito;
        e = e->prox;
    }
    return NULL;
}

// liberarHash() – libera toda a tabela hash
void liberarHash(HashEntry **tabela) {
    for (int i = 0; i < HASH_SIZE; ++i) {
        HashEntry *e = tabela[i];
        while (e) {
            HashEntry *next = e->prox;
            free(e->chave);
            free(e->suspeito);
            free(e);
            e = next;
        }
        tabela[i] = NULL;
    }
}

// contarOcorrenciasSuspeito() – conta quantas das pistas na BST apontam para o suspeito dado
int contarOcorrenciasSuspeito(PistaNode *raiz, HashEntry **tabela, const char *suspeito) {
    if (!raiz) return 0;
    int count = 0;
    const char *s = encontrarSuspeito(tabela, raiz->texto);
    if (s && strcmp(s, suspeito) == 0) count = 1;
    return count + contarOcorrenciasSuspeito(raiz->esq, tabela, suspeito) + contarOcorrenciasSuspeito(raiz->dir, tabela, suspeito);
}

// explorarSalasComPistas() – permite a navegação do jogador pela árvore e coleta pistas.
// Ao visitar uma sala com pista, insere a pista na BST (apenas uma vez).
// Retorno: ponteiro para a raiz da BST de pistas coletadas (deve ser liberada pelo chamador)
PistaNode *explorarSalas(Sala *raiz) {
    if (!raiz) {
        printf("Mapa vazio. Nada para explorar.\n");
        return NULL;
    }
    // Para esta versão, vamos coletar pistas em uma BST enquanto navegamos.
    PistaNode *arvorePistas = NULL;
    // vetor dinâmico para guardar os nomes visitados (apontadores para cópias)
    char **visitados = NULL;
    size_t visit_count = 0;

    Sala *atual = raiz;
    char entrada[64];

    while (1) {
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

        // se há pista na sala, adiciona à árvore de pistas e limpa a pista da sala
        if (atual->pista) {
            printf("Você encontrou uma pista: %s\n", atual->pista);
            arvorePistas = inserirPista(arvorePistas, atual->pista);
            // evita recolher a mesma pista novamente
            free(atual->pista);
            atual->pista = NULL;
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

    // exibe pistas coletadas em ordem alfabética
    printf("\nPistas coletadas (ordem alfabética):\n");
    if (!arvorePistas) {
        printf("(nenhuma pista coletada)\n");
    } else {
        exibirPistas(arvorePistas);
    }
    // Retorna a árvore de pistas para o chamador analisar (ex: julgamento)
    return arvorePistas;
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

    Sala *hall = criarSala("Hall de Entrada", NULL);
    Sala *biblioteca = criarSala("Biblioteca", "Página rasgada com anotações");
    Sala *cozinha = criarSala("Cozinha", "Pegada molhada perto da pia");
    Sala *sotao = criarSala("Sótão", NULL);
    Sala *escritorio = criarSala("Escritório", "caneta com monograma antigo");
    Sala *jardim = criarSala("Jardim", "fio de lã azul preso a um galho");
    Sala *salaJantar = criarSala("Sala de Jantar", NULL);

    conectarSalas(hall, biblioteca, cozinha);
    conectarSalas(biblioteca, sotao, escritorio);
    conectarSalas(cozinha, jardim, salaJantar);

    printf("Bem-vindo(a) ao Detective Quest - Exploração da Mansão\n");
    printf("Começando no Hall de Entrada. Navegue com 'e' (esquerda), 'd' (direita) ou 's' (sair).\n");

    // Inicializa tabela hash e preenche associações pista -> suspeito
    HashEntry *tabela[HASH_SIZE];
    for (int i = 0; i < HASH_SIZE; ++i) tabela[i] = NULL;

    // Associações definidas manualmente (exemplo)
    inserirNaHash(tabela, "Página rasgada com anotações", "Sr. Black");
    inserirNaHash(tabela, "Pegada molhada perto da pia", "Sra. White");
    inserirNaHash(tabela, "caneta com monograma antigo", "Sr. Black");
    inserirNaHash(tabela, "fio de lã azul preso a um galho", "Sr. Green");

    // Exploração: retorna árvore de pistas coletadas
    PistaNode *pistasColetadas = explorarSalas(hall);

    // fase de acusação
    if (pistasColetadas) {
        char acusado[128];
        printf("\nQuem você acusa? Digite o nome do suspeito: ");
        if (fgets(acusado, sizeof(acusado), stdin)) {
            // remover newline
            acusado[strcspn(acusado, "\n")] = '\0';
            if (strlen(acusado) == 0) {
                printf("Nenhum suspeito informado. Encerrando.\n");
            } else {
                int ocorrencias = contarOcorrenciasSuspeito(pistasColetadas, tabela, acusado);
                if (ocorrencias >= 2) {
                    printf("Acusação aceita: %s. Foram encontradas %d pista(s) que o ligam ao crime.\n", acusado, ocorrencias);
                } else {
                    printf("Acusação rejeitada: %s. Apenas %d pista(s) ligam esse suspeito ao crime (são necessárias pelo menos 2).\n", acusado, ocorrencias);
                }
            }
        } else {
            printf("Entrada encerrada antes da acusação.\n");
        }
    } else {
        printf("Nenhuma pista coletada. Não há base para uma acusação.\n");
    }

    // liberar recursos
    liberarPistas(pistasColetadas);
    liberarHash(tabela);
    liberarArvore(hall);

    return 0;
}

