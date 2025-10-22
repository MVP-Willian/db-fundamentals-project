#ifndef BPLUS_H
#define BPLUS_H

#include "no.h"
#include "logger.h"
#include "diskManager.h"
#include <string>
#include <iostream>
#include <cstring> //lib pro buffer

//ordem maxima da arvore
//é pra ser definida com base no tamanho do bloco do disco (provavelmente 4kb sla)
#define ORDEM 4
#define TAM_BLOCO BLOCK_SIZE

template <typename T>
class BplusTree{
private:
    std::string nome_arquivo_indice;
    int id_pagina_raiz;
    int prox_pagina_livre; // contador pro ID de pagina usado no proximo nó
    Logger& log;
    DiskManager disk; //vai tratar I/O no disco

    //leitura de um nó do disco pra memoria (arquivo de indice da b+tree)
    No<T> lerNo(int id_pagina){
        /**
            abrir o arquivo
            procurar posição certa (id_pagina * tam_bloco)
            ler os bytes
            converte em objeto No<T>
            retorna nó alocado
        */
        log.debug("Lendo nó da página com ID " +std::to_string(id_pagina));
        //codigo aq
    }

    //salva um nó da memoria pro disco (tbm arquivo de indice da b+tree)
    void salvarNo(No<T>* no){
        /**
            pega o no e passa pra um buffer de bytes
            abre o arquivo
            procura a posição certa pra colocar (no->minha_pagina_id  tam_bloco)
            escreve o buffer no disco (usar diskManager)
        */
        log.debug("Salvando nó da página com ID " +std::to_string(no->minha_pagina_id));
    }

    //busca o no folha onde a chave deve ser buscada/inserida
    No<T>* buscarNoFolha(const T& chave);
    /** Função recursiva que insere no nó e lida com o split (divisão). */
    // Retorna a chave promovida (se houver split) ou um valor especial (se não houver).
    T inserirRecursivo(No<T>* no_atual, const T& chave, const RecordID& ref_dado);

public:
    
    BplusTree(const std::string& nome_arquivo, Logger& log):
    nome_arquivo_indice(nome_arquivo), id_pagina_raiz(-1), 
    prox_pagina_livre(1), log(log), disk(nome_arquivo, log)
    {
        //bloco 0 tem metadados pra btree
        char meta[TAM_BLOCO];
        disk.readBlock(0, meta);

        if(disk.getTotalBlocks() <= 1){
            log.info("Inicializando nova B+tree no arquivo: " +nome_arquivo);
            id_pagina_raiz = 1;
            prox_pagina_livre = 2;
            No<T> raiz(TipoNo::FOLHA);
            raiz.minha_pagina_id = id_pagina_raiz;
            raiz.prox_pagina_id = -1;

            salvarNo(&raiz);

            //maracutaia dos dados do buffer que vai pro disco (metadados)
            std::memset(meta, 0, TAM_BLOCO);
            std::memcpy(meta, &id_pagina_raiz, sizeof(int));
            std::memcpy(meta + sizeof(int), &prox_pagina_livre, sizeof(int));
            disk.writeBlock(0, meta);
        }else{
            //maracutaia pra ler do buffer e carregar a b+tree
            std::memcpy(&id_pagina_raiz, meta, sizeof(int));
            std::memcpy(&prox_pagina_livre, meta + sizeof(int), sizeof(int));
            log.info("B+tree existente carregada: raiz = " +std::to_string(id_pagina_raiz)
                    + " próxima página livre = " +std::to_string(prox_pagina_livre));
        }
    }

    //busca os RecordIDs (ponteiros de dados) para a chave
    std::vector<RecordID> buscar(const T& chave);

    //insere nova chave e referencia do dado na bplus
    void inserir(const T& chave, const RecordID& ref_dado);
};

#endif