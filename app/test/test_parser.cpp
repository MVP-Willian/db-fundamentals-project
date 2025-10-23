#include "../include/logger.h"
#include "../include/parser.h"
#include "../include/artigo.h"

int main(){
    Logger log_sys;
    log_sys.info("--- Teste do Parser com CSV ---");
    std::string caminho = "data/artigo.csv";
    std::string linha;
    std::ifstream arq(caminho);
    int cont = 0;

    if(!arq.is_open()){
        log_sys.error("O arquivo " +caminho +" não abre");
        return 1;
    }

    while(std::getline(arq, linha) && cont < 5){
        cont++;
        Artigo art = parseArtigo(linha);
        log_sys.info("Verificando dados:");
        log_sys.info("  ID: " + std::to_string(art.getId()));
        log_sys.info("  Título: " + art.getTitulo()); 
        log_sys.info("  Autores: " + art.getAutores()); 
        log_sys.info("  Ano: " + std::to_string(art.getAno()));
        log_sys.info("  Atualizacao: " + art.getAtualizacao()); 
        log_sys.info("  Snippet: " + art.getSnippet()); 
    }
    arq.close();
    log_sys.debug("teste de parsing pro objeto artigo concluido");
    return 0;
}