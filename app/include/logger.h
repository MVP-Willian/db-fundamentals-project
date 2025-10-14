#ifndef LOGGER_H
#define LOGGER_H

#include <iostream>
#include <map>
#include <string>
#include <chrono>
#include <fstream>
#include <algorithm>
#include <cctype>
#include <unistd.h>

/**
 Autor: André Kaled Duarte Coutinho Andrade
 Ultima modificacao: 07/10/2025

 Para saber do chrono: https://www.geeksforgeeks.org/cpp/chrono-in-c/
 Para saber do map: https://www.geeksforgeeks.org/cpp/map-associative-containers-the-c-standard-template-library-stl/
*/


/**
 Enum pra controlar mais facil quais os niveis de log
*/
enum LogLevel{
    ERROR = 0,
    WARN  = 1,
    INFO  = 2,
    DEBUG = 3
};

// Códigos ANSI para cores
#define COR_VERMELHO     "\x1b[31m"
#define COR_AMARELO  "\x1b[38;5;208m"
#define COR_VERDE  "\x1b[32m"
#define COR_CIANO    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"

class Logger{
    private:
        //dicionario que traduz as strings para o enum de níveis
        static const std::map<std::string, LogLevel> LEVEL_MAP;

        //nivel minimo para imprimir as mensagens de log (configurado via env)
        LogLevel nivel_minimo;

        //tempo inicial para medir o tempo de execucao de cada operacao
        std::chrono::time_point<std::chrono::high_resolution_clock> tempo_inicio;
        
        // Retorna a string do nivel ("INFO") e garante que nao modifica o objeto Logger (const)
        std::string level_to_string(LogLevel level) const;

    public:
        //Construtor da classe
        Logger();

        //inicia a contagem de tempo pra operacao atual
        void iniciar_timer();

        //finaliza a contagem e mostra o tempo total em ms
        void finalizar_timer(const std::string& operacao);

        // log especifico para o requisito de blocos lidos
        void log_blocos_lidos(const std::string& operacao, int blocos);

        //funcoes de log com cada nivel, quem valida qual log sera impresso vai
        //ser a funcao log
        void log(LogLevel, const std::string& mensagem);

        //funcoes de log simplificadas (pra agilizar nos logs)
        void error(const std::string& mensagem){ log(LogLevel::ERROR, mensagem); }
        void warn(const std::string& mensagem){ log(LogLevel::WARN, mensagem); }
        void info(const std::string& mensagem){ log(LogLevel::INFO, mensagem); }
        void debug(const std::string& mensagem){ log(LogLevel::DEBUG, mensagem); }
};

#endif