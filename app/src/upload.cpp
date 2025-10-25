#include "artigo.h"
#include "parser.h"
#include "bplus.h"
#include <fstream>
#include <iostream>
#include <filesystem>

// Índice Primário: Chave=int, Dado=long (offset)
const int ORDEM_FOLHA_PRIMARIO = (BLOCK_SIZE - sizeof(CabecalhoPagina) - sizeof(int)) / (sizeof(int) + sizeof(long));
const int ORDEM_INTERNA_PRIMARIO = (BLOCK_SIZE - sizeof(CabecalhoPagina)) / (sizeof(int) + sizeof(int)) - 1;


int main(int argc, char**argv){
    Logger log_sys;
    std::filesystem::remove("/data/db/bplus_primario.idx");
    DiskManager diskPrimario("/data/db/bplus_primario.idx", log_sys);

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

    BPlusTree<int, long, ORDEM_INTERNA_PRIMARIO, ORDEM_FOLHA_PRIMARIO> arvorePrimaria(diskPrimario, log_sys); // chave: ID, dado: offset
    std::string linha;
    int linhaNum = 0;
     // contadores de campos ausentes / inválidos
    int semTitulo = 0;
    int anoInvalido = 0;
    int semAutores = 0;
    int citacoesInvalidas = 0;
    int truncados = 0;
    int semId = 0;
    std::string juntaLinhas;
    log_sys.iniciarTimer();
    while(std::getline(arq,linha)){
        juntaLinhas += linha;
        auto campos = splitCSV(juntaLinhas);
            if (campos.size() < 7) {
                // ainda falta coisa nas outras linhas
                juntaLinhas += "\n";
                continue;
            }

        linhaNum++;
        try{
            Artigo art = parseArtigo(juntaLinhas);
            juntaLinhas.clear();
            bool dadosIncompletos = false;
            std::string motivos;

            if(art.getId() <= 0){
                dadosIncompletos = true;
                motivos += "Id Vazio; ";
                semId++;
            }
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
            if (std::strlen(art.getTitulo().c_str()) >= TAM_TITULO - 1){
                log_sys.warn("Título truncado na linha " + std::to_string(linhaNum));
                truncados++;
            }

            if (dadosIncompletos){
                log_sys.warn("Possível problema no artigo de ID " +std::to_string(art.getId()) + ": " + motivos);
                log_sys.warn(linha);
            }

            long offsetFake = 12345 + art.getId();
            arvorePrimaria.inserirNoBuffer(art.getId(), offsetFake);
            log_sys.debug("Linha: " +std::to_string(linhaNum) +"Parseada com sucesso");
            log_sys.debug("ID [" +std::to_string(art.getId()) +"] Titulo: " +art.getTitulo());
        }catch (const std::exception& e){
            log_sys.error("Erro no parsing da linha " +std::to_string(linhaNum) +":" +e.what());
        }
    }
    arvorePrimaria.flushBuffer();
    log_sys.info("Upload finalizado");
    log_sys.finalizarTimer("Upload + indexação");
    
    // resumo dos problemas
    log_sys.info("Resumo da qualidade dos dados:");
    log_sys.info("Linhas processadas: " + std::to_string(linhaNum));
    log_sys.info("Títulos vazios: " + std::to_string(semTitulo));
    log_sys.info("Anos inválidos: " + std::to_string(anoInvalido));
    log_sys.info("Autores vazios: " + std::to_string(semAutores));
    log_sys.info("Citações negativas: " + std::to_string(citacoesInvalidas));
    log_sys.info("Títulos truncados: " + std::to_string(truncados));
    log_sys.info("ID vazio/inválido: " + std::to_string(semId));
    arq.close();
    return 0;
}