#include <iostream>
#include "logger.h"
#include "diskManager.h"
#include "bplus.h"
#include "artigo.h"

int main(int argc, char** argv) {
    Logger log_sys;

    if(argc < 2){
        log_sys.error("Uso: seek1 <ID>");
        return 1;
    }

    int id = std::stoi(argv[1]);
    log_sys.info("Programa SEEK1 iniciado. Buscando ID: " +std::to_string(id));
    
    DiskManager diskPrimario("/data/db/bplus_primario.idx", log_sys);
    BPlusTree<int, long> arvore{diskPrimario, log_sys};
    log_sys.iniciarTimer();
    try {
        auto resultado = arvore.buscar(id);
        long idBloco = resultado.first;
        log_sys.info("Artigo com ID " + std::to_string(id) + " encontrada!");
        DiskManager diskHash("/data/db/arquivo_hash.dat", log_sys);
        char buffer[BLOCK_SIZE];

        long deslocamento = idBloco % BLOCK_SIZE;

        if(!diskHash.readBlock(idBloco, buffer)){
            log_sys.error("Falha ao ler registro no bloco " +std::to_string(idBloco));
            return 1;
        }

        BlocoDeDados* bloco = reinterpret_cast<BlocoDeDados*>(buffer);
        int indiceArtigo = deslocamento / sizeof(Artigo);
        if (indiceArtigo < 0 || indiceArtigo >= bloco->contador_registros) {
            log_sys.error("bloco inválido: deslocamento não corresponde a um artigo válido.");
            return 1;
        }
        Artigo art = bloco->registros[indiceArtigo];
        
        std::cout << "----------------------------------------" << std::endl;
        std::cout << "ID: " << art.getId() << std::endl;
        std::cout << "Titulo: " << art.getTitulo() << std::endl;
        std::cout << "Ano: " << art.getAno() << std::endl;
        std::cout << "Autores: " << art.getAutores() << std::endl;
        std::cout << "Citacoes: " << art.getCitacoes() << std::endl;
        std::cout << "Atualizacao: " << art.getAtualizacao() << std::endl;
        std::cout << "Snippet: " << art.getSnippet() << std::endl;
        std::cout << "----------------------------------------" << std::endl;

        log_sys.finalizarTimer("Busca em B+ primária");
    } catch (const std::exception& e) {
        log_sys.error("Chave " + std::to_string(id) + " não encontrada: " + e.what());
        log_sys.finalizarTimer("Busca em B+ primária");
        return 1;
    }

    return 0;
}