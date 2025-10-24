#include "bplus.h"
#include "logger.h"

int main() {
    Logger logger; // instancia do logger
    BPlusTree<int, std::string> arvore(2); // ordem = 2

    // Inserindo os valores
    std::vector<std::pair<int, std::string>> elementos = {
        {10, "dez"}, {20, "vinte"}, {5, "cinco"}, {6, "seis"}, {12, "doze"}, {30, "trinta"},
        {1, "um"}, {2, "dois"}, {7, "sete"}, {8, "oito"}, {15, "quinze"}, {25, "vinte e cinco"},
        {35, "trinta e cinco"}, {40, "quarenta"}, {45, "quarenta e cinco"}, {50, "cinquenta"}
    };

    for (auto &e : elementos) {
        arvore.inserir(e.first, e.second);
        logger.debug("Inserida chave " + std::to_string(e.first) + " com valor '" + e.second + "'");
    }

    // Teste de busca para todos os valores
    bool sucesso = true;
    for (auto &e : elementos) {
        try {
            std::string valor = arvore.buscar(e.first);
            if (valor != e.second) {
                logger.error("Chave " + std::to_string(e.first) + " retornou '" + valor + "', esperado '" + e.second + "'");
                sucesso = false;
            } else {
                logger.info("Busca da chave " + std::to_string(e.first) + " retornou corretamente '" + valor + "'");
            }
        } catch (const std::exception &ex) {
            logger.error("Chave " + std::to_string(e.first) + " não encontrada! " + ex.what());
            sucesso = false;
        }
    }

    if (sucesso) {
        logger.info("Todas as buscas passaram com sucesso!");
    } else {
        logger.warn("Algumas buscas falharam!");
    }

    logger.info("Estrutura atual da arvore:");
    arvore.imprimir();
    logger.info("Lista de folhas da arvore:");
    arvore.imprimirFolhas();

    BPlusTree<std::string, int> arvore2(2);
    arvore2.inserir("Um título", 1);
    arvore2.inserir("Outro título", 2);
    arvore2.inserir("título bacanudo", 3);
    arvore2.inserir("título daora", 4);
    arvore2.inserir("título paia", 5);
    arvore2.inserir("título meio paia", 6);

    int id = arvore2.buscar("Um título");
    logger.info("Busca da chave 'Um título' retornou " + std::to_string(id));


    logger.info("Estrutura atual da arvore2:");
    arvore2.imprimir();
    
    logger.info("Lista de folhas da arvore2:");
    arvore2.imprimirFolhas();

    return 0;
}
