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
    int linhaNum = 0;
     // contadores de campos ausentes / inválidos
    int semTitulo = 0;
    int anoInvalido = 0;
    int semAutores = 0;
    int citacoesInvalidas = 0;
    int truncados = 0;
    log_sys.iniciarTimer();
    while(std::getline(arq,linha)){
        linhaNum++;
        try{
            Artigo art = parseArtigo(linha);
            bool dadosIncompletos = false;
            std::string motivos;

            if (art.getTitulo().empty()) {
                dadosIncompletos = true;
                motivos += "Título vazio; ";
                semTitulo++;
            }
            if (art.getAno() <= 0) {
                dadosIncompletos = true;
                motivos += "Ano inválido; ";
                anoInvalido++;
            }
            if (art.getAutores()[0] == '\0') {
                dadosIncompletos = true;
                motivos += "Autores vazios; ";
                semAutores++;
            }
            if (art.getCitacoes() < 0) {
                dadosIncompletos = true;
                motivos += "Número de citações negativo; ";
                citacoesInvalidas++;
            }

            // checa se algo foi cortado pelo strncpy
            if (std::strlen(art.getTitulo().c_str()) >= TAM_TITULO - 1)
                log_sys.warn("Título truncado na linha " + std::to_string(linhaNum));
                truncados++;

            if (dadosIncompletos){
                log_sys.warn("Possível problema no artigo de ID " +std::to_string(art.getId()) + ": " + motivos);
                log_sys.warn(linha);
            }

            log_sys.debug("Linha: " +std::to_string(linhaNum) +"Parseada com sucesso");
            log_sys.debug("ID [" +std::to_string(art.getId()) +"] Titulo: " +art.getTitulo());
        }catch (const std::exception& e){
            log_sys.error("Erro no parsing da linha " +std::to_string(linhaNum) +":" +e.what());
        }
    }
    log_sys.info("Parsing finalizado");
    log_sys.finalizarTimer("Parsing");
    
    // resumo dos problemas
    log_sys.info("Resumo da qualidade dos dados:");
    log_sys.info("Linhas processadas: " + std::to_string(linhaNum));
    log_sys.info("Títulos vazios: " + std::to_string(semTitulo));
    log_sys.info("Anos inválidos: " + std::to_string(anoInvalido));
    log_sys.info("Autores vazios: " + std::to_string(semAutores));
    log_sys.info("Citações negativas: " + std::to_string(citacoesInvalidas));
    log_sys.info("Títulos truncados: " + std::to_string(truncados));
    arq.close();
    return 0;
}