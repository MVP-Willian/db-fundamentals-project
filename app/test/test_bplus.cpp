#include "bplus.h"
#include "diskManager.h"
#include "logger.h"
#include <vector>
#include <string>
#include <cstring>
#include <utility>

// --- Arquivos de índice ---
const std::string ARQUIVO_IDX_PRIMARIO = "arvore_primaria.idx";
const std::string ARQUIVO_IDX_SECUNDARIO = "arvore_secundaria.idx";

// --- Definição da Chave do Índice Secundário (Título) ---
const int TAMANHO_TITULO = 301; // alfa 300 + 1 para null terminator
struct ChaveTitulo {
    char titulo[TAMANHO_TITULO];

    // Construtor para facilitar
    ChaveTitulo(const std::string& s = "") {
        strncpy(titulo, s.c_str(), TAMANHO_TITULO - 1);
        titulo[TAMANHO_TITULO - 1] = '\0'; // Garante null termination
    }

    // Sobrecarga de operadores necessária para comparações na B+Tree
    // (A B+Tree interna usará memcmp via keyCompare..., mas isso ajuda)
    bool operator<(const ChaveTitulo& other) const {
        return std::strcmp(titulo, other.titulo) < 0;
    }
    bool operator>=(const ChaveTitulo& other) const {
        return std::strcmp(titulo, other.titulo) >= 0;
    }
    bool operator==(const ChaveTitulo& other) const {
        return std::strcmp(titulo, other.titulo) == 0;
    }
    // Necessário para std::to_string_log na B+Tree
    operator std::string() const { return std::string(titulo); }
};


// --- Cálculo das Ordens (Baseado em bplus.h e BLOCK_SIZE) ---

// Índice Primário: Chave=int, Dado=long (offset)
const int ORDEM_FOLHA_PRIMARIO = (BLOCK_SIZE - sizeof(CabecalhoPagina) - sizeof(int)) / (sizeof(int) + sizeof(long));
const int ORDEM_INTERNA_PRIMARIO = (BLOCK_SIZE - sizeof(CabecalhoPagina)) / (sizeof(int) + sizeof(int)) - 1;

// Índice Secundário: Chave=ChaveTitulo (char[301]), Dado=int (ID)
const int ORDEM_FOLHA_SECUNDARIO = (BLOCK_SIZE - sizeof(CabecalhoPagina) - sizeof(int)) / (sizeof(ChaveTitulo) + sizeof(int));
const int ORDEM_INTERNA_SECUNDARIO = (BLOCK_SIZE - sizeof(CabecalhoPagina)) / (sizeof(ChaveTitulo) + sizeof(int)) - 1;


int main() {
    Logger logger;
    logger.info("Teste da B+Tree On-Disk");

    // Remove arquivos de índice antigos (para começar do zero)
    std::remove(ARQUIVO_IDX_PRIMARIO.c_str());
    std::remove(ARQUIVO_IDX_SECUNDARIO.c_str());

    // --- Teste 1: Índice Primário <int, long> ---
    logger.info("\nTestando Índice Primário <int, long>");
    logger.info("Ordem Interna: " + std::to_string(ORDEM_INTERNA_PRIMARIO));
    logger.info("Ordem Folha: " + std::to_string(ORDEM_FOLHA_PRIMARIO));
    { // Bloco para garantir que dm1 seja destruído e feche o arquivo
        DiskManager dm1(ARQUIVO_IDX_PRIMARIO, logger);
        BPlusTree<int, long, ORDEM_INTERNA_PRIMARIO, ORDEM_FOLHA_PRIMARIO> arvore1(dm1, logger);

        std::vector<std::pair<int, long>> elementos = {
            {10, 1000}, {20, 2000}, {5, 500}, {6, 600}, {12, 1200}, {30, 3000},
            {1, 100}, {2, 200}, {7, 700}, {8, 800}, {15, 1500}, {25, 2500},
            {35, 3500}, {40, 4000}, {45, 4500}, {50, 5000}
        };

        logger.info("Inserindo elementos...");
        for (auto &e : elementos) {
            arvore1.inserirNoBuffer(e.first, e.second);
        }
        arvore1.flushBuffer();

        logger.info("Testando buscas...");
        bool sucesso = true;
        for (auto &e : elementos) {
            try {
                std::pair<long, int> resultado = arvore1.buscar(e.first);
                long valor = resultado.first;
                int blocosLidos = resultado.second;

                if (valor != e.second) {
                    logger.error("Chave " + std::to_string(e.first) + " retornou offset " + std::to_string(valor) +
                                 ", esperado " + std::to_string(e.second) + ". Blocos lidos: " + std::to_string(blocosLidos));
                    sucesso = false;
                } else {
                    logger.debug("Busca OK: " + std::to_string(e.first) + " -> " + std::to_string(valor));
                }
            } catch (const std::exception &ex) {
                logger.error("Chave " + std::to_string(e.first) + " não encontrada! " + ex.what());
                sucesso = false;
            }
        }

        if (sucesso)
            logger.info("Todas as buscas no Índice Primário passaram!");
        else
            logger.warn("Algumas buscas no Índice Primário falharam!");

        logger.debug("Número total de blocos no arquivo de índice primário: " + std::to_string(dm1.getTotalBlocks()));
    }

    //Teste 2: Índice Secundário <ChaveTitulo, int>
    logger.info("\n--- Testando Índice Secundário <ChaveTitulo, int> ---");
    logger.info("Ordem Interna: " + std::to_string(ORDEM_INTERNA_SECUNDARIO));
    logger.info("Ordem Folha: " + std::to_string(ORDEM_FOLHA_SECUNDARIO));
    {
        DiskManager dm2(ARQUIVO_IDX_SECUNDARIO, logger);
        BPlusTree<ChaveTitulo, int, ORDEM_INTERNA_SECUNDARIO, ORDEM_FOLHA_SECUNDARIO> arvore2(dm2, logger, false);

        std::vector<std::pair<std::string, int>> elementos_str = {
            {"Um título", 1111}, {"Outro título", 2222}, {"título bacanudo", 3333},
            {"título daora", 4444}, {"título paia", 5555}, {"título meio paia", 6666},
            {"Artigo Z", 7777}, {"Artigo A", 8888}, {"AAA", 8888}, {"AAA", 7777}, {"AAA", 6666}
            , {"D", 8888}, {"E", 8888}, {"F", 8888}, {"G", 8888}, {"H", 8888}
        };

        logger.info("Inserindo elementos...");
       for (auto &e : elementos_str) {
            arvore2.inserirNoBuffer(ChaveTitulo(e.first), e.second);
        }
        arvore2.flushBuffer();

        logger.info("Testando buscas...");
        bool sucesso_str = true;
        for (auto &e : elementos_str) {
            try {
                auto [valores, blocosLidos] = arvore2.buscarTodos(ChaveTitulo(e.first));

                if (valores.empty()) {
                    logger.error("Chave '" + e.first + "' não encontrada! Blocos lidos: " + std::to_string(blocosLidos));
                    sucesso_str = false;
                } else {
                    // Loga todos os valores encontrados para essa chave
                    std::string listaValores;
                    for (auto v : valores) listaValores += std::to_string(v) + " ";
                    logger.debug("Chave '" + e.first + "' encontrada com " + std::to_string(valores.size()) +
                                " registro(s). Valores: [" + listaValores + "]");
                }
            } catch (const std::exception &ex) {
                logger.error("Erro ao buscar chave '" + e.first + "': " + std::string(ex.what()));
                sucesso_str = false;
            }
        }

        //busca com duplicatas (titulo=AAA)
        logger.info("Testando busca com duplicatas na b+ indexação secundária...");
        try{
            auto [resultados, blocosLidos] = arvore2.buscarTodos(ChaveTitulo("AAA"));
            logger.info("Busca por 'AAA' retornou " +std::to_string(resultados.size()) 
            +" resultados. blocos lidos: " +std::to_string(blocosLidos));

            for(auto id: resultados)
                logger.debug(" -> " +std::to_string(id));

            if(resultados.empty()) {
                logger.error("Nenhum resultado retornado para a chave 'AAA'");
                sucesso_str = false;
            }
        } catch (const std::exception &ex){
            logger.error("Falha na busca de duplicatas 'AAA': " +std::string(ex.what()));
            sucesso_str = false;
        }

        // Teste com chave inexistente
        logger.info("\nTestando busca de chave inexistente (usando buscarTodos)...");
        try {
            ChaveTitulo chaveInexistente("Titulo Que Nao Existe");
            auto [resultados, blocosLidos] = arvore2.buscarTodos(chaveInexistente);

            if (resultados.empty()) {
                logger.info("OK: Busca por chave inexistente retornou vetor vazio como esperado.");
            } else {
                logger.error("ERRO: Busca por chave inexistente retornou " +
                             std::to_string(resultados.size()) + " resultados!");
                for (auto id : resultados)
                    logger.debug(" -> " + std::to_string(id));
                sucesso_str = false;
            }
        } catch (const std::exception &ex) {
            logger.error("Falha inesperada ao buscar chave inexistente: " + std::string(ex.what()));
            sucesso_str = false;
        }

        if (sucesso_str)
            logger.info("Todas as buscas no Índice Secundário passaram!");
        else
            logger.warn("Algumas buscas no Índice Secundário falharam!");

        logger.debug("Número total de blocos no arquivo de índice secundário: " +
                    std::to_string(dm2.getTotalBlocks()));
    }

    logger.debug("\nTeste da B+Tree On-Disk concluído.");

    return 0;
}