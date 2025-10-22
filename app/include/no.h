#ifndef NO_H
#define NO_H

#include "RecordID.h"
#include <vector>
#include <iostream>

/**
    O no deve conter uma lista de chaves, ponteiros para os no-filhos
    o ponteiro pro prox no, e um bool que indica se é folha ou nao
*/

enum TipoNo{
    INTERNA,
    FOLHA
};

template <typename T>
class No{
public:
    //vetor de chaves
    std::vector<T> chaves;

    /** referencia o proximo nó (só usa no nó-folha) 
        aponta pro ID da pagina no próximo nó-folha
        -1 pra dizer q é null sla
    */
    int prox_pagina_id = -1;
    int minha_pagina_id;

    /*referencia os filhos (só usado em nó-interno)
    guarda o ID da pagina de cada nó filho no disco
    */
    std::vector<int> filhos;
    /* referencia os dados (só usado no nó-folha), o vetor guarda a localizacao dos
    dados no arquivo de dados
    se T for chave primaria, o vetor vai ter o msm tamanho que chaves
    */
    std::vector<RecordID> ponteiro_dados;

    //tipo do no ne
    TipoNo tipo;

    No(TipoNo t = TipoNo::FOLHA): tipo(t) {}
    ~No(){}

    void print() {
        std::cout << "Pagina " << minha_pagina_id << " (" 
                << (tipo == TipoNo::FOLHA ? "folha" : "interna") << ")\n";
        std::cout << "Chaves: ";
        for (auto c : chaves) std::cout << c << " ";
        std::cout << "\n";
    }

};

#endif