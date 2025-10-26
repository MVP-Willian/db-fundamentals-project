#include <iostream>
#include <string>
#include <stdexcept>
#include <set>

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

    
    // --- CÓDIGO NOVO: DETECÇÃO DE CICLO ---
    std::set<long long> blocos_visitados;
    // --- FIM DO CÓDIGO NOVO ---
    
    bool erro_leitura = false; 
    bool ciclo_detectado = false;

    while (balde_id_atual != -1) {

        // --- CÓDIGO NOVO: VERIFICAÇÃO DE CICLO ---
        if (blocos_visitados.count(balde_id_atual)) {
            // Se já visitamos este bloco, estamos em um loop
            encontrado = false;
            break; // Sai do while
        }
        blocos_visitados.insert(balde_id_atual);
        // --- FIM DO CÓDIGO NOVO ---
        

        // 4. Ler o bloco atual do disco
        log_sys.debug("Lendo balde ID: " + std::to_string(balde_id_atual));
        blocos_lidos++;
        

        if (!diskHash.readBlock(balde_id_atual, buffer)) {
             log_sys.error("Falha critica ao ler bloco " + std::to_string(balde_id_atual) + " durante busca do ID " + std::to_string(id_busca) + ". Abortando busca.");
             encontrado = false;
             erro_leitura = true; // Marca que parou por erro de leitura
             break; // Sai do while
        }
        
        // (Assumindo que DiskManager lida com falhas de leitura)
        log_sys.debug("Leitura do bloco " + std::to_string(balde_id_atual) + " OK. Varrendo...");
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
        // --- Código de falha (AGORA MAIS INTELIGENTE) ---
        log_sys.info("Blocos lidos nesta busca: " + std::to_string(blocos_lidos));
        
         if (erro_leitura) {
             log_sys.error("Busca FALHOU devido a erro de leitura no disco.");
             std::cout << "ERRO: Falha ao ler o arquivo de dados durante a busca." << std::endl;
        } else {
             // Se não foi erro de ciclo nem erro de leitura, significa que chegou ao fim da cadeia (-1)
             log_sys.warn("Artigo com ID " + std::to_string(id_busca) + " NAO ENCONTRADO (fim da cadeia).");
             std::cout << "Artigo com ID " << id_busca << " NAO ENCONTRADO." << std::endl;
        }
    }

    return 0;
}