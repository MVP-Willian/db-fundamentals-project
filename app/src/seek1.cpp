#include <iostream>
#include "logger.h"
#include "diskManager.h"
#include "bplus.h"

// Índice Primário: Chave=int, Dado=long (offset)
const int ORDEM_FOLHA_PRIMARIO = (BLOCK_SIZE - sizeof(CabecalhoPagina) - sizeof(int)) / (sizeof(int) + sizeof(long));
const int ORDEM_INTERNA_PRIMARIO = (BLOCK_SIZE - sizeof(CabecalhoPagina)) / (sizeof(int) + sizeof(int)) - 1;

int main(int argc, char** argv) {
    Logger log_sys;

    if(argc < 2){
        log_sys.error("Uso: seek1 <ID>");
        return 1;
    }

    int id = std::stoi(argv[1]);
    log_sys.info("Programa SEEK1 iniciado. Buscando ID: " +std::to_string(id));
    
    DiskManager diskPrimario("/data/db/bplus_primario.idx", log_sys);
    BPlusTree<int, long, ORDEM_INTERNA_PRIMARIO, ORDEM_FOLHA_PRIMARIO> arvore(diskPrimario, log_sys);
    log_sys.iniciarTimer();
    try {
        auto resultado = arvore.buscar(id);
        log_sys.finalizarTimer("Busca em B+ primária");
        log_sys.info("Chave " + std::to_string(id) + " encontrada! Offset: " + std::to_string(resultado.first));
    } catch (const std::exception& e) {
        log_sys.error("Chave " + std::to_string(id) + " não encontrada: " + e.what());
        log_sys.finalizarTimer("Busca em B+ primária");
        return 1;
    }

    return 0;
}