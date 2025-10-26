#ifndef DISKMANAGER_H
#define DISKMANAGER_H

#include <fstream>
#include <filesystem>
#include <cstring>
#include <string>
#include "logger.h"


class DiskManager{
private:
    std::fstream file;
    std::string path;
    Logger& log;
    int blocos_lidos = 0;

public:
    DiskManager(const std::string& path, Logger& logger);

    ~DiskManager();

    /**
    Operacoes de I/O do disco
    */

    /**
    @brief lê um bloco de tamanho fixo do disco 
    @param id_block representa o ID sequencial do bloco a ser lido
    @param buffer ponteiro para o buffer na RAM onde o bloco vai ser copiado (buffer deve ter tamanho BLOCK_SIZE)
    */
    void readBlock(int id_block, char* buffer);

    /**
    @brief escreve em um bloco de tamanho fixo da RAM pro disco
    @param id_block o ID do bloco que sera escrito/sobreescrito
    @param buffer ponteiro para os dados da RAM que serão escritos no disco
    */
    void writeBlock(int id_block, const char* buffer);

    /**
    @brief calcula o numero total de blocos existentes no arquivo
    @return o numero total de blocos no arquivo, ou -1 para erro
    */
    long getTotalBlocks();

    /**
    @brief retorna o numero de blocos lidos desde a ultima chamada e reseta o contador
    @return o numero total de blocos lidos na ultima operacao
    */
    int getAndResetBlocksRead();
};

#endif