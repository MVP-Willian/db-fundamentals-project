#include <iostream>
#include <string>
#include <stdexcept>

#include "logger.h"
#include "diskManager.h"
#include "artigo.h" // Para BlocoDeDados, Artigo, FATOR_BLOCO
#include "utils.h"  // Para hash_function, N_BUCKETS, BLOCK_SIZE

int main(int argc, char**argv) {
    Logger log_sys;

    // --- 1. Validar Argumento ---
    if (argc < 2) {
        log_sys.error("Uso: findrec <ID_DO_ARTIGO>");
        std::cerr << "Uso: findrec <ID_DO_ARTIGO>" << std::endl;
        return 1;
    }

    int id_busca;
    try {
        id_busca = std::stoi(argv[1]);
    } catch (const std::exception& e) {
        log_sys.error("ID invalido: " + std::string(argv[1]));
        return 1;
    }

    log_sys.info("Buscando Artigo com ID: " + std::to_string(id_busca));
    log_sys.iniciarTimer();

    // --- 2. Abrir o Arquivo de Hash ---
    DiskManager diskHash("/data/db/arquivo_hash.dat", log_sys);

    // --- 3. Lógica de Busca no Hash ---
    
    // Calcular o balde primário (USA A MESMA FUNÇÃO DO UPLOAD)
    long long balde_id_atual = hash_function(id_busca);
    char buffer[BLOCK_SIZE];
    
    int blocos_lidos = 0;
    bool encontrado = false;
    Artigo artigo_encontrado;

    while (balde_id_atual != -1) {
        
        // 4. Ler o bloco atual do disco
        log_sys.debug("Lendo balde ID: " + std::to_string(balde_id_atual));
        blocos_lidos++;
        
        // (Assumindo que DiskManager lida com falhas de leitura)
        diskHash.readBlock(balde_id_atual, buffer);
        BlocoDeDados* bloco = reinterpret_cast<BlocoDeDados*>(buffer);

        // 5. Varredura linear dentro do bloco
        for (int i = 0; i < bloco->contador_registros; ++i) {
            if (bloco->registros[i].getId() == id_busca) {
                // 6. Encontrado!
                artigo_encontrado = bloco->registros[i];
                encontrado = true;
                break; // Sai do 'for' loop
            }
        }

        if (encontrado) {
            break; // Sai do 'while' loop
        }

        // 7. Não foi encontrado neste bloco. Seguir a cadeia de overflow.
        log_sys.debug("ID nao encontrado no bloco. Verificando overflow...");
        balde_id_atual = bloco->ponteiro_overflow; 
    }

    // --- 8. Apresentar os resultados ---
    log_sys.finalizarTimer("Busca no hash");
    
    if (encontrado) {
        log_sys.info("Artigo ENCONTRADO!");
        log_sys.info("Blocos lidos nesta busca: " + std::to_string(blocos_lidos));
        
        // Imprime os detalhes do artigo no console
        std::cout << "----------------------------------------" << std::endl;
        std::cout << "ID: " << artigo_encontrado.getId() << std::endl;
        std::cout << "Titulo: " << artigo_encontrado.getTitulo() << std::endl;
        std::cout << "Ano: " << artigo_encontrado.getAno() << std::endl;
        std::cout << "Autores: " << artigo_encontrado.getAutores() << std::endl;
        std::cout << "Citacoes: " << artigo_encontrado.getCitacoes() << std::endl;
        std::cout << "Atualizacao: " << artigo_encontrado.getAtualizacao() << std::endl;
        std::cout << "Snippet: " << artigo_encontrado.getSnippet() << std::endl;
        std::cout << "----------------------------------------" << std::endl;
    } else {
        log_sys.warn("Artigo com ID " + std::to_string(id_busca) + " NAO ENCONTRADO.");
        log_sys.info("Blocos lidos nesta busca: " + std::to_string(blocos_lidos));
        std::cout << "Artigo com ID " << id_busca << " NAO ENCONTRADO." << std::endl;
    }

    return 0;
}