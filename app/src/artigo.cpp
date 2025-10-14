#include "artigo.h"

Artigo::Artigo(int id, const std::string& titulo, int ano,
            const std::string& autores, int citacoes,
            const std::string& atualizacao, const std::string& snippet){
                setId(id);
                setAno(ano);
                setCitacoes(citacoes);
                setTitulo(titulo);
                setAutores(autores);
                setSnippet(snippet);
                setAtualizacao(atualizacao);
            }

void Artigo::setId(int v){
    id = v;
}

void Artigo::setAno(int v){
    ano = v;
}

void Artigo::setCitacoes(int v){
    citacoes = v;
}

void Artigo::setSnippet(const std::string& v){
    std::strncpy(snippet, v.c_str(), TAM_SNIPPET);
    snippet[TAM_SNIPPET-1] = '\0';
}

void Artigo::setAutores(const std::string& v){
    std::strncpy(autores, v.c_str(), TAM_AUTORES);
    autores[TAM_AUTORES-1] = '\0';
}

void Artigo::setTitulo(const std::string& v){
    std::strncpy(titulo, v.c_str(), TAM_TITULO);
    titulo[TAM_TITULO-1] = '\0';
}
