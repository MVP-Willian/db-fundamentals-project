#include "logger.h"

//inicia o dicionario constante de niveis de log
const std::map<std::string, LogLevel> Logger::LEVEL_MAP = {
    {"error", LogLevel::ERROR},
    {"warn", LogLevel::WARN},
    {"info", LogLevel::INFO},
    {"debug", LogLevel::DEBUG}
};

//construtor
Logger::Logger(){
    //tenta ler a variavel de ambiente LOG_LEVEL
    const char* env_level = std::getenv("LOG_LEVEL");

    //nivel padrao se nao tiver variavel do ambiente
    std::string level_str = "info";
    if(env_level && *env_level){
        level_str = env_level;
        level_str.erase(std::remove_if(level_str.begin(), level_str.end(), ::isspace), level_str.end());
    }

    //converte pra minuscula
    for (char &c : level_str) {
        // Usa static_cast para garantir que o tipo int seja passado corretamente para tolower
        c = static_cast<char>(std::tolower(static_cast<int>(c)));
    }
    
        //config nivel do log atual
    if(LEVEL_MAP.count(level_str)){
        nivel_minimo = LEVEL_MAP.at(level_str);
    }else{
        nivel_minimo = LogLevel::INFO;
        std::cerr << "[AVISO] LOG_LEVEL inválido. Usando INFO por padrão" << std::endl;
    }
}

std::string Logger::level_to_string(LogLevel level) const{
    switch(level){
        case LogLevel::ERROR: return "ERROR";
        case LogLevel::WARN:  return "WARN ";
        case LogLevel::INFO:  return "INFO ";
        case LogLevel::DEBUG: return "DEBUG";
        default: return "DESCONHECIDO";
    }
}

void Logger::log(LogLevel nivel, const std::string& mensagem){
    if(nivel <= nivel_minimo)
        std::cout << "[" << level_to_string(nivel) << "] " << mensagem << std::endl;
}

void Logger::iniciar_timer(){
    tempo_inicio = std::chrono::high_resolution_clock::now();
}

void Logger::finalizar_timer(const std::string& operacao){
    auto tempo_fim = std::chrono::high_resolution_clock::now();

    auto duracao = std::chrono::duration_cast<std::chrono::milliseconds>(tempo_fim - tempo_inicio).count();

    this->info("Tempo de execução da operação '" +operacao +"': " +std::to_string(duracao) +"ms");
}

void Logger::log_blocos_lidos(const std::string& operacao, int blocos_lidos) {
    // Requisito: Quantidade de blocos lidos por operação [cite: 103]
    this->info("Mensuração (" + operacao + "): Blocos Lidos: " + std::to_string(blocos_lidos));
}