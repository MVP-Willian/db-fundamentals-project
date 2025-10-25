#include "bplus.h"
#include "logger.h"
#include <vector>
#include <string>

// Arquivos de índice (constantes globais)
const std::string ARQUIVO_IDX_INT_STR = "arvore_int_string.idx";
const std::string ARQUIVO_IDX_STR_INT = "arvore_string_int.idx";

int main() {
    Logger logger;

    logger.info("--- Teste da b+ ---");

    logger.info("Criando b+ <int, string> (ordem = 2)");
    BPlusTree<int, std::string> arvore1(2);

    std::vector<std::pair<int, std::string>> elementos = {
        {10, "dez"}, {20, "vinte"}, {5, "cinco"}, {6, "seis"}, {12, "doze"}, {30, "trinta"},
        {1, "um"}, {2, "dois"}, {7, "sete"}, {8, "oito"}, {15, "quinze"}, {25, "vinte e cinco"},
        {35, "trinta e cinco"}, {40, "quarenta"}, {45, "quarenta e cinco"}, {50, "cinquenta"}
    };

    for (auto &e : elementos) {
        arvore1.inserir(e.first, e.second);
        logger.debug("Inserida chave " + std::to_string(e.first) + " -> '" + e.second + "'");
    }

    logger.info("Estrutura atual da b+:");
    arvore1.imprimir();

    logger.info("Lista de folhas da b+:");
    arvore1.imprimirFolhas();

    logger.info("Salvando b+ em '" + ARQUIVO_IDX_INT_STR + "'...");
    arvore1.salvar(ARQUIVO_IDX_INT_STR);

    logger.info("Carregando b+ novamente...");
    BPlusTree<int, std::string> arvore1_recarregada(2);
    arvore1_recarregada.carregar(ARQUIVO_IDX_INT_STR);

    logger.info("Estrutura da b+ recarregada:");
    arvore1_recarregada.imprimir();

    bool sucesso = true;
    for (auto &e : elementos) {
        try {
            std::string valor = arvore1_recarregada.buscar(e.first);
            if (valor != e.second) {
                logger.error("Chave " + std::to_string(e.first) + " retornou '" + valor + "', esperado '" + e.second + "'");
                sucesso = false;
            } else {
                logger.info("Busca OK: " + std::to_string(e.first) + " -> '" + valor + "'");
            }
        } catch (const std::exception &ex) {
            logger.error("Chave " + std::to_string(e.first) + " não encontrada! " + ex.what());
            sucesso = false;
        }
    }

    if (sucesso)
        logger.info("Todas as buscas na b+ passaram com sucesso!");
    else
        logger.warn("Algumas buscas falharam");

    logger.info("\nCriando b+ <string, int> (ordem = 2)");
    BPlusTree<std::string, int> arvore2(2);

    arvore2.inserir("Um título", 1111);
    arvore2.inserir("Outro título", 2222);
    arvore2.inserir("título bacanudo", 3333);
    arvore2.inserir("título daora", 4444);
    arvore2.inserir("título paia", 5555);
    arvore2.inserir("título meio paia", 6666);

    logger.info("Estrutura atual da b+:");
    arvore2.imprimir();
    arvore2.imprimirFolhas();

    logger.info("Salvando b+ em '" + ARQUIVO_IDX_STR_INT + "'...");
    arvore2.salvar(ARQUIVO_IDX_STR_INT);

    logger.info("Carregando b+ novamente...");
    BPlusTree<std::string, int> arvore2_recarregada(2);
    arvore2_recarregada.carregar(ARQUIVO_IDX_STR_INT);

    logger.info("Estrutura da b+ recarregada:");
    arvore2_recarregada.imprimir();
    arvore2_recarregada.imprimirFolhas();

    try {
        int id = arvore2_recarregada.buscar("título bacanudo");
        logger.info("Busca por 'título bacanudo' retornou ID " + std::to_string(id));
    } catch (const std::exception &ex) {
        logger.error(std::string("Erro ao buscar 'título bacanudo': ") + ex.what());
    }

    logger.debug("Teste da b+ concluído");

    return 0;
}
