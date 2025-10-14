#ifndef ARTIGO_H
#define ARTIGO_H

#include <string>

class Artigo{
    /**
     tem que ajustar os strings para arrays fixos depois,
     olhar como funciona os arrays de caracteres em cpp
    */
    private:
        int id;
        int ano;
        int citacoes;
        std::string titulo;
        std::string autores;
        std::string atualizacao; //mudar pra data/hora ao inves de texto
        std::string snippet;
    
    public:
    Artigo() = default;

    /* Lista de inicialização pro construtor, visto em 
    https://www.learncpp.com/cpp-tutorial/constructor-member-initializer-lists/*/
    Artigo(int id, const std::string& titulo, int ano,
        const std::string& autores, int citacoes,
        const std::string& atualizacao, const std::string& snippet):
        id(id),
        titulo(titulo),
        ano(ano),
        autores(autores),
        citacoes(citacoes),
        atualizacao(atualizacao),
        snippet(snippet) {}

    /** getters e setters */
    int getId() const { return id; }
    int getAno() const { return ano; }
    int getCitacoes() const { return citacoes; }
    std::string getTitulo() const { return titulo; }
    std::string getAutores() const { return autores; }
    std::string getAtualizacao() const { return atualizacao; }
    std::string getSnippet() const { return snippet; }
    void setId(int v) { id = v; }
    void setAno(int v) { ano = v; }
    void setCitacoes(int v) { citacoes = v; }
    void setTitulo(const std::string& v) { titulo = v; }
    void setAutores(const std::string& v) { autores = v; }
    void setAtualizacao(const std::string& v) { atualizacao = v; }
    void setSnippet(const std::string& v) { snippet = v; }
};

#endif