#include "../include/logger.h"

void simularTrabalho(Logger& log_sys, int segundos){
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
    log_sys.iniciarTimer();
    simularTrabalho(log_sys, 2);
    log_sys.finalizarTimer("Simulacao de busca sla");

    //teste dos blocos lidos
    log_sys.logBlocosLidos("Simula busca no bloco", 5);
    log_sys.debug("Cabouuu ebaaa");

    return 0;
}