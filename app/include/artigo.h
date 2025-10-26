#ifndef ARTIGO_H
#define ARTIGO_H

#include <array>
#include <cstring>
#include <cstdlib>
#include <iostream>
#include "utils.h"

//tamanho fixo (N + 1 para o terminador nulo '\0')
#define TAM_TITULO 301
#define TAM_SNIPPET 1025
#define TAM_AUTORES 151
#define TAM_ATUALIZACAO 20 //VERIFICAR REALMENTE QUANTOS BYTES POSSUEM ESSE ATRIBUTO
#define BLOCK_SIZE 4096
#define FATOR_BLOCO 2
class Artigo{
    private:
        int id;
        char titulo[TAM_TITULO];
        int ano;
        char autores[TAM_AUTORES];
        int citacoes;
        char atualizacao[TAM_ATUALIZACAO];
        char snippet[TAM_SNIPPET];
    
    public:
    Artigo() = default;

    Artigo(int id, const std::string& titulo, int ano,
        const std::string& autores, int citacoes,
        const std::string& atualizacao, const std::string& snippet);

    /** getters e setters */
    int getId() const { return id; }
    int getAno() const { return ano; }
    int getCitacoes() const { return citacoes; }
    std::string getTitulo() const { return std::string(titulo); }
    std::string getAutores() const { return std::string(autores); }
    std::string getAtualizacao() const { return std::string(atualizacao); }
    std::string getSnippet() const { return std::string(snippet); }
    void setId(int v);
    void setAno(int v);
    void setCitacoes(int v);
    void setTitulo(const std::string& v);
    void setAutores(const std::string& v);
    void setAtualizacao(const std::string& v);
        
    void setSnippet(const std::string& v);
};

// --- ESTRUTURA FÍSICA (O BLOCO DE HASHING) ---
struct BlocoDeDados{
    //1. Metadados (Cabeçalho do Bloco)

    // Quantos registros estão em uso (ex: 0, 1 ou 2)
    int contador_registros;

    //ID do próximo bloco de overflow ( -1 se for o último)
    long long ponteiro_overflow;

    //2. Área de Dados

    //Array de FATOR_BLOCO "Artigos"
    Artigo registros[FATOR_BLOCO];

};

#endif