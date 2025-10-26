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

bool DiskManager::readBlock(long long id_block, char* buffer){
    if(!file.is_open()){
        log.error("Tentativa de ler o bloco, mas o arquivo não tá aberto");
        return false;
    }
    memset(buffer, 0, BLOCK_SIZE);
    // Calculate offset
    long long offset = static_cast<long long>(id_block) * BLOCK_SIZE;

    // Clear stream state before seeking
    file.clear();
    file.seekg(offset, std::ios::beg);

    // Check if seek failed (e.g., offset beyond file size *initially*)
    if (file.fail()) {
        log.error("Falha no seekg para o bloco " + std::to_string(id_block) + " (offset: " + std::to_string(offset) + ")");
        file.clear(); // Clear the fail bit
        return false; // Indicate failure
    }

    // Try to read
    file.read(buffer, BLOCK_SIZE);

    // Check if read failed or didn't read enough bytes

    // Use gcount() which tells how many bytes were actually read
    if (file.fail() || file.gcount() < BLOCK_SIZE) {
        // Log different messages depending on EOF or other errors
        if (file.eof()) {
            log.error("Falha ao ler o bloco " + std::to_string(id_block) + ": Fim de arquivo inesperado. Bytes lidos: " + std::to_string(file.gcount()));
        } else {
            log.error("Falha ao ler o bloco " + std::to_string(id_block) + ". Erro de I/O. Bytes lidos: " + std::to_string(file.gcount()));
        }
        file.clear(); // Clear error flags (like eofbit, failbit)
        return false; // Indicate failure
    } else {
        // Read successful
        blocos_lidos++;
        log.debug("Bloco " + std::to_string(id_block) + " lido com sucesso");
        // No need to clear here if read was fully successful, but doesn't hurt
        // file.clear();
        return true; // Indicate success
    }
}

bool DiskManager::writeBlock(long long id_block, const char* buffer){
    if(!file.is_open()){
        log.error("Tentativa de escrever bloco, mas arquivo não está aberto");
        return false;
    }    

    // move o ponteiro de escrita para o bloco correto
    file.seekp(static_cast<long long>(id_block) * BLOCK_SIZE, std::ios::beg);
    file.write(buffer, BLOCK_SIZE);
    file.flush();

    if(!file){
        log.error("Falha ao escrever o bloco " + std::to_string(id_block));
        file.clear();
        return false;
    } else {
        log.debug("Bloco " + std::to_string(id_block) + " escrito com sucesso");
        return true;
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
