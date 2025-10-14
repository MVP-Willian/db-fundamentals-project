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
class Artigo{
    /**
     tem que ajustar os strings para arrays fixos depois,
     olhar como funciona os arrays de caracteres em cpp
    */
    private:
        int id;
        char titulo[TAM_TITULO];
        int ano;
        char autores[TAM_AUTORES];
        int citacoes;
        std::tm atualizacao;
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
    std::string getAtualizacao() const { return tmParaString(atualizacao); }
    std::string getSnippet() const { return std::string(snippet); }
    void setId(int v);
    void setAno(int v);
    void setCitacoes(int v);
    void setTitulo(const std::string& v);
    void setAutores(const std::string& v);
    void setAtualizacao(const std::string& v) { atualizacao = stringParaTm(v); }
    void setSnippet(const std::string& v);
};

#endif