#include <iostream>
#include "logger.h"
#include "diskManager.h"
#include "bplus.h"
#include "artigo.h"
#include "ChaveTitulo.h"
#include <string>
#include "hashing.h"

int main(int argc, char** argv) {
    Logger log_sys;

    if(argc < 2){
        log_sys.error("Uso: seek2 <TITULO>");
        return 1;
    }

    std::string tituloStr = argv[1];
    log_sys.info("Programa SEEK2 iniciado. Buscando artigos com o titulo: " +tituloStr);
    
    DiskManager diskPrimario("/data/db/bplus_secundario.idx", log_sys);
    BPlusTree<ChaveTitulo, long> arvore{diskPrimario, log_sys, false};
    log_sys.iniciarTimer();
    try {
        ChaveTitulo chaveBusca(tituloStr);
        auto resultado = arvore.buscarTodos(chaveBusca);
        const auto& idsBlocos = resultado.first;
        int blocosLidos = resultado.second;
        log_sys.info(std::string("Artigos com titulo ") + tituloStr + " encontrada:" + std::to_string(idsBlocos.size()));
        DiskManager diskHash("/data/db/arquivo_hash.dat", log_sys);
        char buffer[BLOCK_SIZE];

        for(long idBloco: idsBlocos){
            if(!diskHash.readBlock(idBloco, buffer)){
                log_sys.error("Falha ao ler registro no bloco " +std::to_string(idBloco));
                continue;
            }

            BlocoDeDados* bloco = reinterpret_cast<BlocoDeDados*>(buffer);
            for(int i = 0; i < bloco->contador_registros; i++){
                Artigo art = bloco->registros[i];
                if(std::string(art.getTitulo()) == tituloStr){
                    std::cout << "----------------------------------------" << std::endl;
                    std::cout << "ID: " << art.getId() << std::endl;
                    std::cout << "Titulo: " << art.getTitulo() << std::endl;
                    std::cout << "Ano: " << art.getAno() << std::endl;
                    std::cout << "Autores: " << art.getAutores() << std::endl;
                    std::cout << "Citacoes: " << art.getCitacoes() << std::endl;
                    std::cout << "Atualizacao: " << art.getAtualizacao() << std::endl;
                    std::cout << "Snippet: " << art.getSnippet() << std::endl;
                    std::cout << "----------------------------------------" << std::endl;
                }
            }
        }
        log_sys.finalizarTimer("Busca em B+ secundária");
    } catch (const std::exception& e) {
        log_sys.error(std::string("Chave ") +tituloStr + " não encontrada: " + e.what());
        log_sys.finalizarTimer("Busca em B+ secundária");
        return 1;
    }

    return 0;
}