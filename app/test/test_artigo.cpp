#include "../include/logger.h"
#include "../include/artigo.h"

int main(){
    Logger log_sys;
    log_sys.info("--- Teste da classe Artigo ---");
    log_sys.debug("Iniciando teste com a classe Artigo");

    Artigo art(
        1,
        "titulo teste", 
        2025,
        "dré", 
        37, 
        "2025-10-14 10:30:00", 
        "testando tamanho do snippet sla");

    art.setSnippet("testando tamanho do snippet 2 sla");

    log_sys.info("Verificando dados:");
    log_sys.info("  ID: " + std::to_string(art.getId()));
    log_sys.info("  Título: " + art.getTitulo()); 
    log_sys.info("  Autores: " + art.getAutores()); 
    log_sys.info("  Ano: " + std::to_string(art.getAno()));
    log_sys.info("  Atualizacao: " + art.getAtualizacao()); 
    log_sys.info("  Snippet: " + art.getSnippet()); 

    log_sys.debug("teste de copia e criacao de objeto artigo concluido");
    return 0;
}