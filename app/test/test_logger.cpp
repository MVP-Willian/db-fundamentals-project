#include <iostream>
#include <unistd.h>
#include "../include/logger.h"

void simular_trabalho(Logger& log_sys, int segundos){
    log_sys.debug("Simulando trabalho de " +std::to_string(segundos) +"s...");
    sleep(segundos);
    log_sys.debug("Trabalho finalizado :D");
}

int main(){
    Logger log_sys;
    std::cout << "--- Teste do Logger ---" << std::endl;

    log_sys.info("O nivel de log configurado (lido da ENV) está ativo");
    log_sys.warn("Aviso muito importante");
    log_sys.error("Erro muito errado ai chefe");

    //teste do tempo
    log_sys.iniciar_timer();
    simular_trabalho(log_sys, 2);
    log_sys.finalizar_timer("Simulacao de busca sla");

    //teste dos blocos lidos
    log_sys.log_blocos_lidos("Simula busca no bloco", 5);
    log_sys.debug("Cabouuu ebaaa");

    return 0;
}