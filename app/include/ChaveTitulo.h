#ifndef CHAVE_TITULO_H
#define CHAVE_TITULO_H

#include <cstring> // para strcmp, strcpy, strncpy
#include <string>

// Define o tamanho máximo (o mesmo que você usava antes)
const int TAM_TITULO_CHAVE = 301; 

struct ChaveTitulo {
    char titulo[TAM_TITULO_CHAVE];

    // Construtor padrão (importante)
    ChaveTitulo() {
        titulo[0] = '\0';
    }

    // Construtor a partir de std::string (trunca se necessário)
    ChaveTitulo(const std::string& s) {
        std::strncpy(titulo, s.c_str(), TAM_TITULO_CHAVE - 1);
        titulo[TAM_TITULO_CHAVE - 1] = '\0'; // Garante terminação nula
    }

    // --- Operadores que faltavam no char[] ---

    // 1. Comparação (para std::sort e B+Tree)
    bool operator<(const ChaveTitulo& other) const {
        return std::strcmp(titulo, other.titulo) < 0;
    }

    bool operator==(const ChaveTitulo& other) const {
        return std::strcmp(titulo, other.titulo) == 0;
    }

    bool operator>=(const ChaveTitulo& other) const {
        return std::strcmp(titulo, other.titulo) >= 0;
    }

    // 2. Atribuição (para std::vector, std::sort)
    ChaveTitulo& operator=(const ChaveTitulo& other) {
        if (this != &other) {
            std::strcpy(titulo, other.titulo);
        }
        return *this;
    }

    // 3. Construtor de Cópia (para std::vector)
    ChaveTitulo(const ChaveTitulo& other) {
        std::strcpy(titulo, other.titulo);
    }

    // Função de ajuda para converter para string (para logs)
    std::string toString() const {
        return std::string(titulo);
    }
};

#endif