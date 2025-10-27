#ifndef HASHING_H
#define HASHING_H

// Inclua o que for necessário para a função funcionar
#include "artigo.h"     // Para struct Artigo e BlocoDeDados
#include "diskManager.h" // Para a classe DiskManager
#include "logger.h"      // Para a classe Logger

/**
 * @brief Insere um Artigo no arquivo de dados organizado por Hashing Estático.
 * * @param dm O gerenciador de disco do arquivo de dados.
 * @param artigo O registro Artigo a ser inserido.
 * @param log O logger para mensagens de debug/warn/error.
 * @return long long O ID do bloco (Bloco ID) onde o artigo foi gravado.
 */
long long insere_no_hash(DiskManager& dm, Artigo& artigo, Logger& log);

// Você pode colocar a função de pré-alocação aqui também, se quiser
void pre_aloca_hashing(DiskManager& dm, Logger& log);

#endif // HASHING_H