#include "artigo.h"
#include "parser.h"
#include "bplus.h"
#include "hashing.h"
#include "utils.h"
#include <fstream>
#include <iostream>
// (Todos os includes de thread/mutex/queue foram removidos)

// Índice Primário: Chave=int, Dado=long (offset)
const int ORDEM_FOLHA_PRIMARIO = (BLOCK_SIZE - sizeof(CabecalhoPagina) - sizeof(int)) / (sizeof(int) + sizeof(long));
const int ORDEM_INTERNA_PRIMARIO = (BLOCK_SIZE - sizeof(CabecalhoPagina)) / (sizeof(int) + sizeof(int)) - 1;


// --- FUNÇÃO MAIN (SERIAL) ---
int main(int argc, char**argv){
    Logger log_sys;
    
    // --- INICIALIZA OS DOIS DISK MANAGERS ---
    DiskManager diskPrimario("/data/db/bplus_primario.idx", log_sys);
    DiskManager diskHash("/data/db/arquivo_hash.dat", log_sys); // DiskManager para o arquivo hash

    // --- NOVA SEÇÃO DE INICIALIZAÇÃO ---
    // (Assumindo que NUM_BALDES_PRIMARIOS está definido em algum header)
    const long long TOTAL_BLOCOS_NECESSARIOS = N_BUCKETS + 1;
    if (diskHash.getTotalBlocks() < TOTAL_BLOCOS_NECESSARIOS) { 
        log_sys.warn("Arquivo de hash 'arquivo_hash.dat' incompleto ou ausente.");
        log_sys.warn("Inicializando " + std::to_string(TOTAL_BLOCOS_NECESSARIOS) + " blocos vazios...");
        
        char buffer_vazio[BLOCK_SIZE] = {0}; 
        for (long long i = 0; i < TOTAL_BLOCOS_NECESSARIOS; ++i) { 
            diskHash.writeBlock(i, buffer_vazio);
            if (i % 100000 == 0) { 
                 log_sys.debug("... blocos escritos: " + std::to_string(i));
            }
        }
        log_sys.info("Arquivo de hash inicializado com " + std::to_string(diskHash.getTotalBlocks()) + " blocos.");
    }
    // --- FIM DA SEÇÃO DE INICIALIZAÇÃO ---
    
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
     // contadores
    int semTitulo = 0;
    int anoInvilido = 0;
    int semAutores = 0;
    int citacoesInvalidas = 0;
    int truncados = 0;
    int semId = 0;
    std::string juntaLinhas;
    log_sys.iniciarTimer();
    
    // --- LOOP DE PROCESSAMENTO SERIAL ---
    while(std::getline(arq,linha)){
        juntaLinhas += linha;
        auto campos = splitCSV(juntaLinhas);
            if (campos.size() < 7) {
                juntaLinhas += "\n";
                continue;
            }

        linhaNum++;
        try{
            // 1. Parse (Trabalho de CPU)
            Artigo art = parseArtigo(juntaLinhas);
            juntaLinhas.clear();
            bool dadosIncompletos = false;
            std::string motivos;

            // ... (Toda a sua lógica de validação) ...
            if(art.getId() <= 0){ dadosIncompletos = true; motivos += "Id Vazio; "; semId++; }
            if (art.getTitulo().empty()) { dadosIncompletos = true; motivos += "Título vazio; "; semTitulo++; }
            if (art.getAno() <= 0) { dadosIncompletos = true; motivos += "Ano inválido; "; anoInvilido++; }
            if (art.getAutores()[0] == '\0') { dadosIncompletos = true; motivos += "Autores vazios; "; semAutores++; }
            if (art.getCitacoes() < 0) { dadosIncompletos = true; motivos += "Número de citações negativo; "; citacoesInvalidas++; }

            if (std::strlen(art.getTitulo().c_str()) >= TAM_TITULO - 1){
                log_sys.warn("Título truncado na linha " + std::to_string(linhaNum));
                truncados++;
            }

            if (dadosIncompletos){
                log_sys.warn("Possível problema no artigo de ID " +std::to_string(art.getId()) + ": " + motivos);
                log_sys.warn(linha);
            }

            // --- MUDANÇA PRINCIPAL ---
            // Removemos o offset FAKE
            
            // 2. Insere no Hash (Trabalho de Disco - I/O)
            //    A CPU ficará esperando o disco terminar aqui.
            long long offset_real = insere_no_hash(diskHash, art, log_sys);

            // 3. Insere na B+ Tree (Trabalho de CPU/Memória)
            //    Usa o offset_real retornado pelo passo 2.
            arvorePrimaria.inserirNoBuffer(art.getId(), offset_real);
            // --- FIM DA MUDANÇA ---


            log_sys.debug("Linha: " +std::to_string(linhaNum) +" Parseada e Inserida com sucesso");
            log_sys.debug("ID [" +std::to_string(art.getId()) +"] Titulo: " +art.getTitulo());

        } catch (const std::exception& e){
            log_sys.error("Erro no parsing da linha " +std::to_string(linhaNum) +":" +e.what());
        }
    }
    
    // --- FINALIZAÇÃO ---
    // (Tudo como estava antes, o que está correto)
    
    log_sys.info("Parsing do CSV finalizado. Fazendo flush do buffer da B+ Tree...");
    arvorePrimaria.flushBuffer(); // Salva o que restou no buffer da B+ Tree
    
    log_sys.info("Upload finalizado");
    log_sys.finalizarTimer("Upload + indexação (serial)");
    
    // resumo dos problemas
    log_sys.info("Resumo da qualidade dos dados:");
    log_sys.info("Linhas processadas: " + std::to_string(linhaNum));
    log_sys.info("Títulos vazios: " + std::to_string(semTitulo));
    log_sys.info("Anos inválidos: " + std::to_string(anoInvilido));
    log_sys.info("Autores vazios: " + std::to_string(semAutores));
    log_sys.info("Citações negativas: " + std::to_string(citacoesInvalidas));
    log_sys.info("Títulos truncados: " + std::to_string(truncados));
    log_sys.info("ID vazio/inválido: " + std::to_string(semId));
    arq.close();
    return 0;
}