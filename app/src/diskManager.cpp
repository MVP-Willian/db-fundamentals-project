#include "diskManager.h"
#include "logger.h"
#include "artigo.h" 

DiskManager::DiskManager(const std::string& path, Logger& logger):
path(path),log(logger)
{
    try {
        std::filesystem::path diretorio(path);
        std::filesystem::create_directories(diretorio.parent_path());
        log.info("Diretório criado ou já existente: " +diretorio.string());
    } catch (const std::exception& e) {
        log.error("Erro ao criar diretório: " + std::string(e.what()));
    }    

    bool arquivo_novo = false;

    file.open(path,std::ios::binary | std::ios::in | std::ios::out);
    log.debug("Inicializando DiskManager com arquivo: " + path);

    //se nao existir ele cria o arquivo automaticamente
    if(!file.is_open()){
        log.warn("Arquivo nao encontrado, tentando criar: " +path);
        file.open(path, std::ios::binary | std::ios::out);
        if(file.is_open()){
            arquivo_novo = true;
            file.close();
            file.open(path,std::ios::binary | std::ios::in | std::ios::out);
        }
    }

    if(!file.is_open()){
        log.error("Falha ao abrir o arquivo de disco: " +path);
        return;
    }
    log.info("Arquivo " +path +" aberto com sucesso!");
    if(arquivo_novo){
        char buffer_metadados[BLOCK_SIZE] = {0};
        writeBlock(0, buffer_metadados);
        file.clear();
        file.seekp(0, std::ios::beg);
    }
    
}

DiskManager::~DiskManager(){
    if(file.is_open()){
        file.close();
        log.debug("Arquivo " +path +" fechado");
    }
}

void DiskManager::readBlock(int id_block, char* buffer){
    if(!file.is_open()){
        log.error("Tentativa de ler o bloco, mas o arquivo não tá aberto");
        return;
    }
    memset(buffer, 0, BLOCK_SIZE);
    // move o ponteiro de escrita para o bloco correto
    file.seekg(id_block * BLOCK_SIZE, std::ios::beg);
    file.read(buffer, BLOCK_SIZE);

    if(!file){
        log.error("Falha ao ler o bloco " + std::to_string(id_block));
        file.clear();
    } else {
        blocos_lidos++;
        log.debug("Bloco " + std::to_string(id_block) + " lido com sucesso");
    }
}

void DiskManager::writeBlock(int id_block, const char* buffer){
    if(!file.is_open()){
        log.error("Tentativa de escrever bloco, mas arquivo não está aberto");
        return;
    }    

    // move o ponteiro de escrita para o bloco correto
    file.seekp(id_block * BLOCK_SIZE, std::ios::beg);
    file.write(buffer, BLOCK_SIZE);
    file.flush();

    if(!file){
        log.error("Falha ao escrever o bloco " + std::to_string(id_block));
        file.clear();
    } else {
        log.debug("Bloco " + std::to_string(id_block) + " escrito com sucesso");
    }
}

long DiskManager::getTotalBlocks(){
    if(!file.is_open()) return -1;
    file.seekg(0, std::ios::end);
    auto tam = file.tellg();
    return tam/BLOCK_SIZE;
}

int DiskManager::getAndResetBlocksRead(){
    int temp = blocos_lidos;
    blocos_lidos = 0;
    return temp;
}
