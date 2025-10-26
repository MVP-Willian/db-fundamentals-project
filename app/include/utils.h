// utils.h
#ifndef UTILS_H
#define UTILS_H

#include <ctime>
#include <iomanip>
#include <sstream>
#include <string>

inline std::tm stringParaTm(const std::string& s) {
    std::tm t = {};
    std::istringstream ss(s);
    ss >> std::get_time(&t, "%Y-%m-%d %H:%M:%S"); // formato do CSV
    return t;
}

inline std::string tmParaString(const std::tm& t) {
    std::ostringstream ss;
    ss << std::put_time(&t, "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

// O N (Primo) calculado para N_Registros=1549146, F_B=2, alpha=0.7
const long long N_BUCKETS = 1106527;

// --- FUNÇÃO HASH ---

/**
 * @brief Função Hash principal (Hashing Estático por Módulo).
 * Mapeia um ID de Artigo para um ID de Bloco (Balde).
 * @param id O ID do artigo.
 * @return O ID do bloco (entre 0 e N_BUCKETS-1).
 */
inline long long hash_function(int id) {
    // 'inline' é bom aqui por ser uma função pequena e muito usada
    return (long long)id % N_BUCKETS + 1;
}

#endif