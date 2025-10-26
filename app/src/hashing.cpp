// hashing.cpp

#include "hashing.h"
#include "utils.h"      // N_BUCKETS, hash_function
#include "artigo.h"     // BlocoDeDados, BLOCK_SIZE, FATOR_BLOCO
#include "diskManager.h" // DiskManager
#include "logger.h"      // Logger
#include <iostream>
#include <cstring>      // Para memset, se necessário

// --- IMPLEMENTAÇÃO DA PRÉ-ALOCAÇÃO DA TABELA HASH ---

/**
 * @brief Inicializa o arquivo de dados de Hashing com N_BUCKETS blocos vazios.
 * @param dm O gerenciador de disco do arquivo de dados.
 * @param log O logger para mensagens.
 */
void pre_aloca_hashing(DiskManager& dm, Logger& log) {
    // Bloco 0 é reservado. O Hashing começa do Bloco 1.
    if (dm.getTotalBlocks() > 1) { 
        log.info("Arquivo de Hashing já existe (Tamanho: " + std::to_string(dm.getTotalBlocks()) + " blocos). Pulando pré-alocação.");
        return;
    }
    
    log.info("Iniciando pré-alocação da Tabela Hash com " + std::to_string(N_BUCKETS) + " baldes...");
    
    // 1. Cria um buffer de 4096 bytes zerado na memória
    char buffer_vazio[BLOCK_SIZE] = {0};
    
    // 2. Interpreta esses bytes como a struct BlocoDeDados
    BlocoDeDados* bloco_vazio = reinterpret_cast<BlocoDeDados*>(buffer_vazio);
    
    // 3. Configura o TEMPLATE de bloco vazio (Metadados)
    bloco_vazio->contador_registros = 0;
    // CRUCIAL: O ponteiro de overflow é -1 para indicar o fim da cadeia.
    bloco_vazio->ponteiro_overflow = -1; 
    
    // 4. Escreve N_BUCKETS blocos vazios no disco, começando do Bloco 1.
    for (long long i = 1; i <= N_BUCKETS; i++) {
        // O DiskManager.writeBlock usa 'int' para o id_block, cast é seguro aqui
        // já que N_BUCKETS (1.1M) cabe em int.
        dm.writeBlock(i, buffer_vazio);
    }
    
    log.info("Pré-alocação concluída. O arquivo tem agora " + std::to_string(dm.getTotalBlocks()) + " blocos.");
}


// --- IMPLEMENTAÇÃO DA INSERÇÃO COM TRATAMENTO DE OVERFLOW ---

/**
 * @brief Insere um Artigo no arquivo de dados organizado por Hashing Estático,
 * tratando colisões e overflows.
 * @param dm O gerenciador de disco do arquivo de dados.
 * @param artigo O registro Artigo a ser inserido.
 * @param log O logger para mensagens.
 * @return long long O ID do bloco (Bloco ID) onde o artigo foi gravado.
 */
long long insere_no_hash(DiskManager& dm, Artigo& artigo, Logger& log) {
    
    // 1. Calcular o balde primário (hash_function deve retornar um ID >= 1)
    long long balde_id_atual = hash_function(artigo.getId());
    char buffer[BLOCK_SIZE];
    
    while (true) {
        // 2. Lê o bloco atual do disco (1 acesso a disco)
        // O DiskManager.readBlock usa 'int' para o id_block
        dm.readBlock(balde_id_atual, buffer);
        BlocoDeDados* bloco = reinterpret_cast<BlocoDeDados*>(buffer);

        // 3. Caso 1: Bloco tem espaço (Inserção sem overflow)
        if (bloco->contador_registros < FATOR_BLOCO) {
            bloco->registros[bloco->contador_registros] = artigo;
            bloco->contador_registros++;
            dm.writeBlock(balde_id_atual, buffer);
            
            log.debug("Artigo ID " + std::to_string(artigo.getId()) + " inserido no balde " + std::to_string(balde_id_atual));
            return balde_id_atual; // RETORNA O ID DO BLOCO
        }
        
        // 4. Caso 2: Bloco está cheio, mas já tem ponteiro de overflow
        if (bloco->ponteiro_overflow != -1) {
            // Segue a cadeia de overflow
            long long proximo_bloco = bloco->ponteiro_overflow;
            log.debug("Colisão para ID " + std::to_string(artigo.getId()) + ". Seguindo para overflow no bloco " + std::to_string(proximo_bloco));
            balde_id_atual = proximo_bloco;
            continue; // Repete o loop para tentar o próximo bloco da cadeia
        }

        // 5. Caso 3: Bloco está cheio, e É o fim da cadeia (CRIAR NOVO BLOCO)
        
        // O próximo ID de bloco livre é o tamanho atual do arquivo (TOTAL BLOCKS)
        long long novo_bloco_id = dm.getTotalBlocks(); 
        
        // (A) Atualiza o bloco atual para apontar para o novo
        bloco->ponteiro_overflow = novo_bloco_id;
        dm.writeBlock(balde_id_atual, buffer); // Salva o novo ponteiro
        
        // (B) Cria o novo bloco de overflow na memória
        char buffer_novo[BLOCK_SIZE] = {0};
        BlocoDeDados* bloco_novo = reinterpret_cast<BlocoDeDados*>(buffer_novo);
        
        // Inicializa o novo bloco de overflow com o registro atual
        bloco_novo->contador_registros = 1; 
        bloco_novo->registros[0] = artigo;
        bloco_novo->ponteiro_overflow = -1; // É o novo fim da cadeia

        // (C) Escreve o novo bloco no disco
        dm.writeBlock(novo_bloco_id, buffer_novo);
        
        log.warn("Overflow para ID " + std::to_string(artigo.getId()) + ". Novo bloco criado e inserido em ID " + std::to_string(novo_bloco_id));
        return novo_bloco_id; // RETORNA O ID DO NOVO BLOCO DE OVERFLOW
    }
}