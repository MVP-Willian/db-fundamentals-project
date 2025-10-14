#include <iostream>
#include "../include/logger.h"
#include "../include/artigo.h"

int main(){
    Logger log_sys;
    Artigo art(1,"titulo teste", 2025,"dré", 0, "14/10/2025", "blabla");
    std::cout << "--- Teste do artigo ---" << std::endl;

    log_sys.info("tentando imprimir o negocio do artigo");
    log_sys.info("Título: " + art.getTitulo());
    log_sys.info("Ano: " + std::to_string(art.getAno()));
    log_sys.debug("Cabouuu ebaaa");

    return 0;
}