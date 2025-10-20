#include "artigo.h"
#include "parser.h"
#include <fstream>
#include <iostream>

int main(int argc, char**argv){
    Logger log_sys;

    if(argc < 2){
        log_sys.error("Uso do programa: upload <arquivo.csv");
        return 1;
    }

    std::string caminho = argv[1];
    std::ifstream arq(caminho);
    if(!arq.is_open()){
        log_sys.error("Não foi possível abrir o arquivo: " +caminho);
        return 1;
    }

    std::string linha;
    int linhaNum;
    log_sys.iniciarTimer();
    while(std::getline(arq,linha)){
        linhaNum++;
        try{
            Artigo art = parseArtigo(linha);
            log_sys.debug("Linha: " +std::to_string(linhaNum) +"Parseada com sucesso");
            log_sys.debug("ID [" +std::to_string(art.getId()) +"] Titulo: " +art.getTitulo());
        }catch (const std::exception& e){
            log_sys.error("Erro no parsing da linha " +std::to_string(linhaNum) +":" +e.what());
        }
    }
    log_sys.info("Parsing finalizado");
    log_sys.finalizarTimer("Parsing");
    arq.close();
    return 0;
}