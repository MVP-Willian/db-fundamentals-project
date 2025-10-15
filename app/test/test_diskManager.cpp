#include "../include/logger.h"
#include "../include/artigo.h"
#include "../include/diskManager.h"
#include <vector>
#include <cstring> // std::memset
#include <sstream> // std::ostringstream

int main() {
    Logger log_sys;
    log_sys.info("--- Teste da classe DiskManager ---");
    log_sys.debug("Iniciando teste com DiskManager");

    std::string arquivo_teste = "/data/db/teste_disco.bin";
    DiskManager disk(arquivo_teste, log_sys);

    // cria vários artigos de teste
    std::vector<Artigo> artigos = {
        Artigo(1, "Titulo 1", 2025, "dré", 37, "2025-10-14 10:30:00", "Snippet 1"),
        Artigo(2, "Titulo 2", 2024, "Alice", 12, "2025-09-10 14:00:00", "Snippet 2"),
        Artigo(3, "Titulo 3", 2023, "Bob", 5, "2025-08-20 09:15:00", "Snippet 3"),
        Artigo(4, "Titulo 4", 2022, "Carol", 20, "2025-07-05 18:45:00", "Snippet 4")
    };

    char buffer[BLOCK_SIZE];

    // escreve cada artigo em blocos consecutivos
    for (size_t i = 0; i < artigos.size(); ++i) {
        std::memset(buffer, 0, BLOCK_SIZE);

        std::ostringstream oss;
        oss << artigos[i].getId() << ";"
            << artigos[i].getTitulo() << ";"
            << artigos[i].getAno() << ";"
            << artigos[i].getAutores() << ";"
            << artigos[i].getCitacoes() << ";"
            << artigos[i].getAtualizacao() << ";"
            << artigos[i].getSnippet();

        std::string dados = oss.str();
        std::memcpy(buffer, dados.c_str(), std::min(dados.size(), size_t(BLOCK_SIZE)));

        log_sys.debug("Escrevendo bloco " + std::to_string(i) + " com os dados do artigo");
        disk.writeBlock(i, buffer);
    }

    // leitura e impressão dos blocos
    for (size_t i = 0; i < artigos.size(); ++i) {
        std::memset(buffer, 0, BLOCK_SIZE);

        log_sys.debug("Lendo bloco " + std::to_string(i));
        disk.readBlock(i, buffer);

        log_sys.info("Conteúdo lido do bloco " + std::to_string(i) + ":");
        log_sys.info(buffer);
    }

    int blocos_lidos = disk.getAndResetBlocksRead();
    log_sys.info("Blocos lidos: " + std::to_string(blocos_lidos));

    log_sys.debug("Teste do DiskManager concluído");
    return 0;
}
