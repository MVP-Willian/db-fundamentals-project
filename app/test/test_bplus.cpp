#include "bplus.h"
#include "logger.h"

int main() {
    Logger logger;

    // Cria a árvore no arquivo de índice "indice_bplus.dat"
    BplusTree<int> arvore("dados/indice_bplus.dat", logger);

    // só pra ver se o construtor leu/grava os metadados
    logger.info("Teste de construtor da B+Tree finalizado.");

    return 0;
}
