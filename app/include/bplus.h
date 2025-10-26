#include <iostream>
#include <string>
#include <stdexcept>
#include <cstring>
#include <vector>
#include <cmath>
#include "logger.h"
#include "diskManager.h"
#include "artigo.h"

//"metadados" sobre um nó
struct CabecalhoPagina {
    bool folha;     // 1 se a página é folha, 0 se interna
    int numChaves;  // num de chaves guardadas
};

// o bloco 0 (Superbloco) guarda metadados da árvore no geral
struct Superbloco {
    int idBlocoRaiz;    // ID do bloco raiz
    int ordemInterna; // ordem máxima de um nó interno
    int ordemFolha;   // ordem máxima de uma folha
    bool primaria;
};

const int ORDEM_INTERNA_INT = (BLOCK_SIZE - sizeof(CabecalhoPagina)) / (sizeof(int) + sizeof(int)) - 1; // ~509 
const int ORDEM_FOLHA_INT_LONG = (BLOCK_SIZE - sizeof(CabecalhoPagina) - sizeof(int)) / (sizeof(int) + sizeof(long)); // ~340 
const int TAMANHO_CHAVE_TITULO = 301; // Para char[300] + null
const int ORDEM_INTERNA_CHAR300 = std::floor((BLOCK_SIZE - sizeof(CabecalhoPagina) - sizeof(int)) / (TAMANHO_CHAVE_TITULO + sizeof(int))); // = 13
const int ORDEM_FOLHA_CHAR300_INT = std::floor((BLOCK_SIZE - sizeof(CabecalhoPagina) - sizeof(int)) / (TAMANHO_CHAVE_TITULO + sizeof(int))); // = 13

template <typename Chave, typename Dado, int ORDEM_INTERNA, int ORDEM_FOLHA>
class BPlusTree {
private:
    int idBlocoRaiz;
    DiskManager& dm;
    Logger& logger;
    //coisinhas pro buffer
    std::vector<std::pair<Chave, Dado>> buffer;
    const int TAMANHO_BUFFER = 1000; // número de registros antes de inserir
    bool primaria; //bool pra mudar a inserção/busca se for primaria/secundaria

    //nó interno, só tem chave e ponteiros pros filhos
    struct PaginaInterna {
        CabecalhoPagina cabecalho;
        Chave chaves[ORDEM_INTERNA];
        int idsFilhos[ORDEM_INTERNA + 1];

        PaginaInterna() {
            cabecalho.folha = false;
            cabecalho.numChaves = 0;
            for(int i = 0; i <= ORDEM_INTERNA; i++) idsFilhos[i] = -1; // -1 = nulo
        }
    };

    //nó folha, tem as chaves, dados, e o ponteiro pras proximas folhas
    struct PaginaFolha {
        CabecalhoPagina cabecalho;
        Chave chaves[ORDEM_FOLHA];
        Dado dados[ORDEM_FOLHA]; // v de dados/offsets
        int idProximaFolha;      // ponteiro das folhas

        PaginaFolha() {
            cabecalho.folha = true;
            cabecalho.numChaves = 0;
            idProximaFolha = -1; // -1 é pra nulo
        }
    };

    // a < b
    bool keyCompareLess(const Chave& a, const Chave& b) {
        if constexpr (std::is_array_v<Chave>) {
            return std::memcmp(&a, &b, sizeof(Chave)) < 0;
        } else {
            return a < b;
        }
    }

    // a >= b
    bool keyCompareGreaterEqual(const Chave& a, const Chave& b) {
        if constexpr (std::is_array_v<Chave>) {
            return std::memcmp(&a, &b, sizeof(Chave)) >= 0;
        } else {
            return a >= b;
        }
    }

    // a == b
    bool keyCompareEqual(const Chave& a, const Chave& b) {
        if constexpr (std::is_array_v<Chave>) {
            return std::memcmp(&a, &b, sizeof(Chave)) == 0;
        } else {
            return a == b;
        }
    }

    /**
        Lê o cabeçalho de uma página a partir do ID do bloco,
        Retorna uma struct CabecalhoPagina falando se o nó é folha/interna 
        e o numero de chaves
        basicamente lê os primeiros bytes do bloco e copia pra struct
    */
    CabecalhoPagina lerCabecalho(int idBloco) {
        char buffer[BLOCK_SIZE];
        dm.readBlock(idBloco, buffer);
        CabecalhoPagina cabecalho;
        std::memcpy(&cabecalho, buffer, sizeof(CabecalhoPagina));
        return cabecalho;
    }

    /**
        lê uma página interna do disco, copia o conteudo para a struct PaginaInterna
        A pagina tem chaves e IDs dos filhos, serve pra manipulação em memória da arvore
    */
    void lerPaginaInterna(int idBloco, PaginaInterna& pagina) {
        char buffer[BLOCK_SIZE];
        dm.readBlock(idBloco, buffer);
        std::memcpy(&pagina, buffer, sizeof(PaginaInterna));
    }

    /**
        Lê uma pagina folha do disco, copia o conteudo pra struct PaginaFolha
        tem chaves e os dados associados, e o ponteiro pra próxima folha
        permite percorrer e modificar a folha em memória
    */
    void lerPaginaFolha(int idBloco, PaginaFolha& pagina) {
        char buffer[BLOCK_SIZE];
        dm.readBlock(idBloco, buffer);
        std::memcpy(&pagina, buffer, sizeof(PaginaFolha));
    }

    /**
        Escreve uma página interna de volta pro disco
        Atualiza o bloco com o conteúdo da struct PaginaInterna, 
        incluindo a chave e ponteiros filhos
    */
    void escreverPaginaInterna(int idBloco, const PaginaInterna& pagina) {
        char buffer[BLOCK_SIZE];
        std::memset(buffer, 0, BLOCK_SIZE); 
        std::memcpy(buffer, &pagina, sizeof(PaginaInterna));
        dm.writeBlock(idBloco, buffer);
    }

    /**
        escreve uma pagina folha de volta ao disco
        atualiza o bloco com chaves, dados e ponteiros para a próxima folha
    */
    void escreverPaginaFolha(int idBloco, const PaginaFolha& pagina) {
        char buffer[BLOCK_SIZE];
        std::memset(buffer, 0, BLOCK_SIZE); 
        std::memcpy(buffer, &pagina, sizeof(PaginaFolha));
        dm.writeBlock(idBloco, buffer);
    }

    
    /** 
        Salva metadados no bloco 0, metadados da árvore B+.
        guarda ID da raiz, ordens máximas dos nós internos e folhas, e se é índice primário.
        isso permite reconstituir a árvore ao abrir o arquivo de novo (pra buscar)
    */
    void salvarSuperbloco() {
        char buffer[BLOCK_SIZE];
        std::memset(buffer, 0, BLOCK_SIZE);
        Superbloco sb;
        sb.idBlocoRaiz = this->idBlocoRaiz;
        sb.ordemInterna = ORDEM_INTERNA;
        sb.ordemFolha = ORDEM_FOLHA;
        sb.primaria = this->primaria;
        std::memcpy(buffer, &sb, sizeof(Superbloco));
        dm.writeBlock(0, buffer);
    }

    /**
        aloca um novo bloco no disco
        retorna o próximo ID livre pra usar em splits ou novas páginas
    */
    int alocarNovoBloco() {
        return dm.getTotalBlocks(); // pega o prox ID livre
    }

    /**
        funcao recursiva que insere uma chave e dado na árvore B+.
        basicamente percorre a árvore a partir do bloco atual até encontrar a folha certa
        se a folha tiver cheia, cria um split e promove uma chave para o pai.
        Para nós internos, insere a chave promovida no nó e realiza split se precisar.
        Retorna -1 se não houve split, ou o ID do novo bloco criado.
    */
    int inserirRec(int idBlocoAtual, const Chave& chave, const Dado& dado, Chave& chavePromovida) {
        if (idBlocoAtual < 1) { logger.error("Erro interno: inserirRec bloco inválido ID: " + std::to_string(idBlocoAtual)); throw std::runtime_error("Acesso a bloco inválido no índice.");}

        CabecalhoPagina cabecalho = lerCabecalho(idBlocoAtual);

        if (cabecalho.folha) {
            // --- FOLHA: Usa buffer temporário (std::vector) ---
            PaginaFolha folha; // Struct com tamanho ORDEM
            lerPaginaFolha(idBlocoAtual, folha);

            //so verifica se for primaria
            if(this->primaria){
                // verifica duplicada (importante para índice primário)
                for (int k = 0; k < folha.cabecalho.numChaves; ++k) {
                    if (keyCompareEqual(chave, folha.chaves[k])) {
                        logger.error("Chave duplicada: " + to_string_log(chave));
                        throw std::runtime_error("Chave duplicada não permitida.");
                    }
                }
            }
            // Cria buffers temporários na HEAP com +1 espaço
            std::vector<Chave> chavesTemp(ORDEM_FOLHA + 1);
            std::vector<Dado> dadosTemp(ORDEM_FOLHA + 1);
            // Copia dados existentes para os temporários
            std::memcpy(chavesTemp.data(), folha.chaves, folha.cabecalho.numChaves * sizeof(Chave));
            std::memcpy(dadosTemp.data(), folha.dados, folha.cabecalho.numChaves * sizeof(Dado));
            int numChavesTemp = folha.cabecalho.numChaves;

            // Insere ordenado nos buffers temporários
            int i = numChavesTemp - 1;
            while (i >= 0 && keyCompareLess(chave, chavesTemp[i])) {
                chavesTemp[i + 1] = chavesTemp[i];
                dadosTemp[i + 1] = dadosTemp[i];
                i--;
            }
             if ((i + 1) > ORDEM_FOLHA) { logger.error("Overflow buffer temporário folha indice " + std::to_string(i+1)); throw std::runtime_error("Overflow buffer temporário folha.");}
            chavesTemp[i + 1] = chave;
            dadosTemp[i + 1] = dado;
            numChavesTemp++;

            // Split se o contador TEMPORÁRIO exceder a ordem MÁXIMA
            if (numChavesTemp > ORDEM_FOLHA) {
                 // Chama split passando ponteiros para os dados dos vetores
                 // Passa também o idProximaFolha original
                 return splitFolha(idBlocoAtual, folha.idProximaFolha, chavesTemp.data(), dadosTemp.data(), numChavesTemp, chavePromovida);
            } else {
                // Se não houve split, copia dados temporários de volta para a struct da página
                std::memcpy(folha.chaves, chavesTemp.data(), numChavesTemp * sizeof(Chave));
                std::memcpy(folha.dados, dadosTemp.data(), numChavesTemp * sizeof(Dado));
                folha.cabecalho.numChaves = numChavesTemp;
                // Limpa o resto do buffer da struct (opcional, boa prática)
                // Limpa a partir do índice numChavesTemp até ORDEM_FOLHA-1
                if (numChavesTemp < ORDEM_FOLHA) {
                    std::memset(&folha.chaves[numChavesTemp], 0, (ORDEM_FOLHA - numChavesTemp) * sizeof(Chave));
                    std::memset(&folha.dados[numChavesTemp], 0, (ORDEM_FOLHA - numChavesTemp) * sizeof(Dado));
                }
                escreverPaginaFolha(idBlocoAtual, folha); // Salva a página atualizada
                return -1; // Sem split
            }
        } else {
            // --- NÓ INTERNO: Usa buffer temporário (std::vector) ---
            PaginaInterna interno; // Struct com tamanho ORDEM
            lerPaginaInterna(idBlocoAtual, interno);

            // busca binaria mt louco
            int L = 0, R = interno.cabecalho.numChaves;
            while (L < R) {
                int meio = L + (R - L) / 2;
                if (keyCompareGreaterEqual(chave, interno.chaves[meio]))
                    L = meio + 1; //tenta a direita
                else R = meio; //achou, guarda e tenta a esquerda
            }
            int indicePonteiro = L;
            // valida o índice do ponteiro
            if (indicePonteiro < 0 || indicePonteiro > interno.cabecalho.numChaves) {
                 logger.error("Erro interno: indicePonteiro inválido ("+std::to_string(indicePonteiro)+") no nó interno "+std::to_string(idBlocoAtual));
                 throw std::runtime_error("Erro de lógica na busca do filho interno.");
            }
            int idBlocoFilho = interno.idsFilhos[indicePonteiro];
            if (idBlocoFilho == -1) { logger.error("Ponteiro nulo filho no nó interno "+std::to_string(idBlocoAtual)+" indice "+std::to_string(indicePonteiro)); throw std::runtime_error("Ponteiro nulo filho.");}

            // Chamada Recursiva
            Chave chaveSubida; // Receberá a chave promovida do filho
            int idNovoIrmao = inserirRec(idBlocoFilho, chave, dado, chaveSubida);

            if (idNovoIrmao == -1) { return -1; } // Sem split abaixo

            // Houve split no filho: Prepara buffers temporários para inserir chaveSubida
            std::vector<Chave> chavesTempI(ORDEM_INTERNA + 1);
            std::vector<int> filhosTemp(ORDEM_INTERNA + 2);
            // Copia dados do nó atual para os buffers
            std::memcpy(chavesTempI.data(), interno.chaves, interno.cabecalho.numChaves * sizeof(Chave));
            std::memcpy(filhosTemp.data(), interno.idsFilhos, (interno.cabecalho.numChaves + 1) * sizeof(int));
            int numChavesTempI = interno.cabecalho.numChaves;

            // Insere chaveSubida e idNovoIrmao ordenados nos buffers temporários
            int j = numChavesTempI - 1;
            while (j >= 0 && keyCompareLess(chaveSubida, chavesTempI[j])) {
                chavesTempI[j + 1] = chavesTempI[j];
                filhosTemp[j + 2] = filhosTemp[j + 1];
                j--;
            }
             // Verifica limites (segurança extra)
             if ((j + 1) > ORDEM_INTERNA || (j + 2) > ORDEM_INTERNA + 1) { // Índices máximos são ORDEM_INTERNA e ORDEM_INTERNA+1
                 logger.error("Erro interno: Overflow no buffer temporário interno ao calcular índice j="+std::to_string(j));
                 throw std::runtime_error("Overflow buffer temporário interno.");
            }
            chavesTempI[j + 1] = chaveSubida;
            filhosTemp[j + 2] = idNovoIrmao;
            numChavesTempI++;

            // Verifica se precisa de split neste nó interno
            if (numChavesTempI > ORDEM_INTERNA) {
                // Chama split passando ponteiros para os dados dos vetores
                return splitInterno(idBlocoAtual, chavesTempI.data(), filhosTemp.data(), numChavesTempI, chavePromovida);
            } else {
                // Copia dados dos buffers temporários de volta para a struct da página
                 std::memcpy(interno.chaves, chavesTempI.data(), numChavesTempI * sizeof(Chave));
                 std::memcpy(interno.idsFilhos, filhosTemp.data(), (numChavesTempI + 1) * sizeof(int));
                 interno.cabecalho.numChaves = numChavesTempI;
                 // Limpa o resto (opcional)
                 if (numChavesTempI < ORDEM_INTERNA) std::memset(&interno.chaves[numChavesTempI], 0, (ORDEM_INTERNA - numChavesTempI) * sizeof(Chave));
                 // Limpa ponteiros filhos de numChavesTempI + 1 até ORDEM_INTERNA
                 if (numChavesTempI + 1 <= ORDEM_INTERNA) std::memset(&interno.idsFilhos[numChavesTempI + 1], -1, (ORDEM_INTERNA + 1 - (numChavesTempI + 1)) * sizeof(int));

                escreverPaginaInterna(idBlocoAtual, interno); // Salva página atualizada
                return -1; // Sem split neste nível
            }
        }
    }

    /**
        divide uma folha cheia em duas páginas separadas.
        Copia as chaves e dados temporários para a folha original e para a nova folha.
        Ajusta o ponteiro da lista encadeada de folhas e promove a primeira chave da nova folha.
        Retorna o ID do novo bloco criado (irmão direito da folha original).
    */
    int splitFolha(int idBlocoOriginal, int idProximaOriginal,
                   const Chave* chavesTemp, const Dado* dadosTemp, int numChavesTemp, // numChavesTemp = ORDEM_FOLHA + 1
                   Chave& chavePromovida){
        int t = (ORDEM_FOLHA + 1) / 2; // Número de chaves que ficam na original
        PaginaFolha folhaOriginal; // Cria página original em memória
        PaginaFolha novaFolha;     // Cria nova página em memória
        int idNovoBloco = alocarNovoBloco(); // localizacao do bloco no disco pra nova folha

        int numChavesNova = numChavesTemp - t;
        folhaOriginal.cabecalho.numChaves = t;
        novaFolha.cabecalho.numChaves = numChavesNova;

        if (t < 0 || numChavesNova < 0 || t > ORDEM_FOLHA || numChavesNova > ORDEM_FOLHA || (t + numChavesNova) != numChavesTemp) {
             logger.error("Erro cálculo splitFolha: t="+std::to_string(t)+", numNova="+std::to_string(numChavesNova)+", numTemp="+std::to_string(numChavesTemp));
             throw std::runtime_error("Erro de lógica no splitFolha.");
        }

        // Copia dos buffers temporários (usando ponteiros) para as páginas finais
        std::memcpy(folhaOriginal.chaves, chavesTemp, t * sizeof(Chave));
        std::memcpy(folhaOriginal.dados, dadosTemp, t * sizeof(Dado));
        std::memcpy(novaFolha.chaves, &chavesTemp[t], numChavesNova * sizeof(Chave));
        std::memcpy(novaFolha.dados, &dadosTemp[t], numChavesNova * sizeof(Dado));

        // Ajusta ponteiros da lista encadeada
        novaFolha.idProximaFolha = idProximaOriginal; // A nova aponta para quem a original apontava
        folhaOriginal.idProximaFolha = idNovoBloco;   // A original aponta para a nova

        chavePromovida = novaFolha.chaves[0]; // Primeira chave da nova folha sobe

        // Limpa o resto dos arrays nas structs (opcional, boa prática)
        // Limpa a partir do índice t até ORDEM_FOLHA - 1
        if (t < ORDEM_FOLHA) {
            std::memset(&folhaOriginal.chaves[t], 0, (ORDEM_FOLHA - t) * sizeof(Chave));
            std::memset(&folhaOriginal.dados[t], 0, (ORDEM_FOLHA - t) * sizeof(Dado));
        }

        escreverPaginaFolha(idBlocoOriginal, folhaOriginal); // Escreve original (agora com t chaves)
        escreverPaginaFolha(idNovoBloco, novaFolha); // Escreve nova (com numChavesNova chaves)

        logger.debug("Split folha: Bloco " + std::to_string(idBlocoOriginal) + "(" + std::to_string(t) + ") / Bloco " +
                     std::to_string(idNovoBloco) + "(" + std::to_string(numChavesNova) + "). Chave: " + to_string_log(chavePromovida));
        return idNovoBloco;
    }

    /**
        divide um nó interno cheio em dois nós internos
        a chave do meio é promovida pro nó pai, o resto é dividido entre o nó original e o novo
        copia chaves e filhos temporarios pra novas paginas e limpa posições restantes
        retorna o ID do novo bloco criado
    */
    int splitInterno(int idBlocoOriginal,
                     const Chave* chavesTempI, const int* filhosTemp, int numChavesTempI, // numChavesTempI = ORDEM_INTERNA + 1
                     Chave& chavePromovida){
        int t = (ORDEM_INTERNA + 1) / 2; // Índice da chave a promover
        PaginaInterna internoOriginal; // Cria página original (vazia) em memória (tamanho M)
        PaginaInterna novoInterno;     // Cria nova página (vazia) em memória (tamanho M)
        int idNovoBloco = alocarNovoBloco(); // Aloca bloco no disco

        chavePromovida = chavesTempI[t]; // Chave do meio sobe

        int numChavesNova = numChavesTempI - t - 1; // Chaves que vão para o novo nó
        internoOriginal.cabecalho.numChaves = t;    // Nó original fica com t chaves
        novoInterno.cabecalho.numChaves = numChavesNova; // Novo nó fica com o resto

        // Verifica limites (Defensivo)
        // t+1 filhos na original, numChavesNova+1 filhos na nova
        if (t < 0 || numChavesNova < 0 || (t + 1 + numChavesNova) != numChavesTempI || (t+1) > (ORDEM_INTERNA+1) || (numChavesNova+1) > (ORDEM_INTERNA+1)) {
            logger.error("Erro cálculo splitInterno: t="+std::to_string(t)+", numNova="+std::to_string(numChavesNova)+", numTemp="+std::to_string(numChavesTempI));
            throw std::runtime_error("Erro de lógica no splitInterno.");
        }


        // Copia chaves e filhos dos temporários para as páginas finais
        std::memcpy(internoOriginal.chaves, chavesTempI, t * sizeof(Chave));                     // Copia primeiras t chaves
        std::memcpy(internoOriginal.idsFilhos, filhosTemp, (t + 1) * sizeof(int));             // Copia primeiros t+1 filhos
        std::memcpy(novoInterno.chaves, &chavesTempI[t + 1], numChavesNova * sizeof(Chave));       // Copia chaves restantes (índices t+1 até fim)
        std::memcpy(novoInterno.idsFilhos, &filhosTemp[t + 1], (numChavesNova + 1) * sizeof(int)); // Copia filhos restantes (índices t+1 até fim)

        // Limpa o resto dos arrays nas structs (opcional)
        // Limpa chaves a partir de t até ORDEM_INTERNA-1
        if (t < ORDEM_INTERNA) std::memset(&internoOriginal.chaves[t], 0, (ORDEM_INTERNA - t) * sizeof(Chave));
        // Limpa filhos a partir de t+1 até ORDEM_INTERNA
        if (t + 1 <= ORDEM_INTERNA) std::memset(&internoOriginal.idsFilhos[t + 1], -1, (ORDEM_INTERNA + 1 - (t + 1)) * sizeof(int));


        escreverPaginaInterna(idBlocoOriginal, internoOriginal); // Escreve original (agora com t chaves)
        escreverPaginaInterna(idNovoBloco, novoInterno);         // Escreve novo (com numChavesNova chaves)

        logger.debug("Split interno: Bloco " + std::to_string(idBlocoOriginal) + "(" + std::to_string(t) + ") / Bloco " +
                     std::to_string(idNovoBloco) + "(" + std::to_string(numChavesNova) + "). Chave: " + to_string_log(chavePromovida));

        return idNovoBloco; // Retorna ID do novo bloco ("irmão" direito)
    }

    /**
        funcao auxiliar, converte chave pra string só pra mostrar nos logs
    */
    std::string to_string_log(const Chave& val) {
        if constexpr (std::is_convertible_v<Chave, std::string>) {
            return std::string(val); 
        }
        // Se for um tipo numérico (int, long)
        else {
            return std::to_string(val);
        }
    }

public:
    // Construtor que carrega uma B+Tree existente do disco ou cria uma nova se não existir
    // Inicializa superbloco e raiz, valida compatibilidade de ordens e tipo de índice
    // garante que a árvore tá pronta para inserções e buscas
    BPlusTree(DiskManager& diskManager, Logger& logger, bool primaria = true)
        : primaria(primaria), dm(diskManager), logger(logger), idBlocoRaiz(-1) {

        if (dm.getTotalBlocks() < 2) {
            logger.warn("Índice B+Tree não encontrado ou inválido (blocos < 2). Criando novo índice...");

            idBlocoRaiz = 1;
            PaginaFolha raizInicial;

            // Garante que o DiskManager 'alocou' o bloco 1 antes de escrever nele
            if (dm.getTotalBlocks() < 2) {
                 char bufferVazio[BLOCK_SIZE];
                 std::memset(bufferVazio, 0, BLOCK_SIZE);
                 if (dm.getTotalBlocks() == 0) dm.writeBlock(0, bufferVazio);
                 dm.writeBlock(1, bufferVazio);
            }
            escreverPaginaFolha(idBlocoRaiz, raizInicial);

            salvarSuperbloco();

            logger.info("Novo índice B+Tree criado. Superbloco (0) e Raiz (1) inicializados.");
        } else {
            char buffer[BLOCK_SIZE];
            dm.readBlock(0, buffer); // le o Superbloco
            Superbloco sb;
            std::memcpy(&sb, buffer, sizeof(Superbloco));

            if (sb.ordemInterna != ORDEM_INTERNA || sb.ordemFolha != ORDEM_FOLHA) {
                 logger.error("Incompatibilidade de Ordem do índice! Esperado F:" + std::to_string(ORDEM_FOLHA) +
                             ", I:" + std::to_string(ORDEM_INTERNA) + ". Encontrado F:" + std::to_string(sb.ordemFolha) +
                             ", I:" + std::to_string(sb.ordemInterna));
                throw std::runtime_error("Incompatibilidade de Ordem do índice.");
            }

            //nao devia abrir o arquivo da b+secundaria se for instanciada como primaria
            if(sb.primaria != this->primaria){
                logger.error(std::string("Tipo de indice incompatível, tentando abrir índice primária como secundária:\n")
                +"Estrutura primária? = " +std::to_string(sb.primaria) +" | "
                +"B+ instanciada primária? = " +std::to_string(this->primaria));
                throw std::runtime_error("Incompatibilidade de tipos de indices");
            }

            if (sb.idBlocoRaiz < 1 || sb.idBlocoRaiz >= dm.getTotalBlocks()) {
                logger.error("ID do bloco raiz inválido (" + std::to_string(sb.idBlocoRaiz) + ") lido do Superbloco.");
                 throw std::runtime_error("ID do bloco raiz inválido no Superbloco.");
            }

            idBlocoRaiz = sb.idBlocoRaiz;
            logger.info("Índice B+Tree (primaria=" +std::to_string(sb.primaria) +") carregado. Raiz no bloco " + std::to_string(idBlocoRaiz));
        }
    }

    ~BPlusTree(){}

    /**
        insere um registro (chave/dado) na b+
        chama a funcao recursiva pra percorrer a arvore e inserir na folha
        se tiver split na raiz, cria uma nova raiz com dois filhos
    */
    void inserir(const Chave& chave, const Dado& dado) {
        Chave chavePromovida;
        int idNovoFilho = inserirRec(idBlocoRaiz, chave, dado, chavePromovida);

        if (idNovoFilho != -1) {
            // a raiz tomou-lhe split
            PaginaInterna novaRaiz;
            int idNovaRaiz = alocarNovoBloco();

            novaRaiz.cabecalho.numChaves = 1;
            novaRaiz.chaves[0] = chavePromovida;
            novaRaiz.idsFilhos[0] = idBlocoRaiz; //filho esquerdo é raiz antiga
            novaRaiz.idsFilhos[1] = idNovoFilho; //filho direito (novo bloco)

            escreverPaginaInterna(idNovaRaiz, novaRaiz);

            // ATUALIZA O ID DA RAIZ e SALVA NO SUPERBLOCO
            idBlocoRaiz = idNovaRaiz;
            salvarSuperbloco();
            
            logger.warn("SPLIT DE RAIZ! Nova raiz no bloco: " + std::to_string(idBlocoRaiz));
        }
    }

    /**
        busca um registro pela chave em indice primário, percorre a arvore do bloco raiz
        até a folha certa.
        retorna o dado encontrado e a quantidade de blocos lidos
        lança exceção de runtime se a chave nao existir na b+
    */
    std::pair<Dado, int> buscar(const Chave& chave) {
        int idBlocoAtual = idBlocoRaiz;

        // zera o contador do diskManager antes da busca
        dm.getAndResetBlocksRead(); 

        while (true) {
            CabecalhoPagina cabecalho = lerCabecalho(idBlocoAtual);
            
            if (cabecalho.folha) {
                PaginaFolha folha;
                lerPaginaFolha(idBlocoAtual, folha); // le o bloco de folha

                // linear na folha
                for (int i = 0; i < folha.cabecalho.numChaves; i++) {
                    if (keyCompareEqual(folha.chaves[i], chave)) {
                        // Pega a contagem DEPOIS da busca
                        int blocosLidos = dm.getAndResetBlocksRead();
                        logger.info("Chave " + to_string_log(chave) + " encontrada. Blocos lidos do índice: " + std::to_string(blocosLidos));
                        return {folha.dados[i], blocosLidos};
                    }
                }
                throw std::runtime_error("Chave não encontrada (não está na folha)");
            
            } else {
                PaginaInterna interno;
                lerPaginaInterna(idBlocoAtual, interno);

                // acha o próximo filho (busca binária seria daora)
                int i = 0;
                while (i < interno.cabecalho.numChaves && keyCompareGreaterEqual(chave, interno.chaves[i])) {
                    i++;
                }
                idBlocoAtual = interno.idsFilhos[i];

                if (idBlocoAtual == -1) {
                     throw std::runtime_error("Chave não encontrada (ponteiro nulo no índice)");
                }
            }
        }
    }

    /**
        mesma coisa de buscar, mas é pra busca em indice secundária
        desce até a primeira folha possível e percorre a lista de folhas
        coleta todos os dados iguais a chave buscada
        retorna vetor de resultados e numero de blocos lidos
    */
    std::pair<std::vector<Dado>, int> buscarTodos(const Chave& chave) {
        int idBlocoAtual = idBlocoRaiz;
        dm.getAndResetBlocksRead();
        std::vector<Dado> resultados;

        // desce a arvore até a folha correta
        while (true) {
            CabecalhoPagina cabecalho = lerCabecalho(idBlocoAtual);
            if (cabecalho.folha) break; // achouu

            PaginaInterna interno;
            lerPaginaInterna(idBlocoAtual, interno);

            // busca linear
            int i = 0;
            while (i < interno.cabecalho.numChaves && keyCompareGreaterEqual(chave, interno.chaves[i])) {
                i++;
            }
            idBlocoAtual = interno.idsFilhos[i];

            if (idBlocoAtual == -1) {
                int blocosLidos = dm.getAndResetBlocksRead();
                logger.info("Chave "+to_string_log(chave)+" não encontrada (ponteiro nulo). Blocos lidos: "+std::to_string(blocosLidos));
                return {resultados, blocosLidos}; //ta vazio
            }
        }

        PaginaFolha folha;
        lerPaginaFolha(idBlocoAtual, folha);
        int idxInicio = 0;
        // vai pra primeira chave que nao é menor que a chave buscada
        while (idxInicio < folha.cabecalho.numChaves && keyCompareLess(folha.chaves[idxInicio], chave)) {
            idxInicio++;
        }

        // busca linear da lista de folhas
        bool buscando = true;
        bool primeiraFolha = true;
        while (buscando && idBlocoAtual != -1) {
            
            if(!primeiraFolha){
                lerPaginaFolha(idBlocoAtual, folha);//le o prox bloco
                idxInicio = 0;
            }
            primeiraFolha = false;

            for (int i = idxInicio; i < folha.cabecalho.numChaves; i++) {
                if (keyCompareEqual(folha.chaves[i], chave)) {
                    resultados.push_back(folha.dados[i]);
                } else {
                    // cabou a busca, chave maior que a buscada
                    buscando = false;
                    break;
                }
            }

            //ainda lendo procurando folhas
            if (buscando) {
                idBlocoAtual = folha.idProximaFolha;
                idxInicio = 0;
            }
        }

        int blocosLidos = dm.getAndResetBlocksRead();
        if (resultados.empty()) {
            logger.info("Chave " + to_string_log(chave) + " não encontrada");
        } else {
            logger.info("Chave " + to_string_log(chave) + " encontrada com " + std::to_string(resultados.size()) + " registros");
        }
        logger.info("Blocos lidos: " + std::to_string(blocosLidos));
        return {resultados, blocosLidos};
    }

    /**
        insere vários registros de uma vez, ordenando antes de inserir
        usa otimização em lotes pra evitar split repetidos desnecessários
    */
    void inserirLote(const std::vector<std::pair<Chave, Dado>>& registros) {
        // exemplo para a primária:
        std::vector<std::pair<Chave, Dado>> temp = registros;
        std::sort(temp.begin(), temp.end(), [](const auto& a, const auto& b){
            return a.first < b.first; // ou chave primária
        });

        for (auto& reg : temp) {
            inserir(reg.first, reg.second); // inserção normal na B+Tree
        }
    }

    /**
        insere no buffer temporário, quando cheio chama a funcao inserirLote,
        é pra usar ao inserir varios registros no upload
    */
    void inserirNoBuffer(const Chave& chave, const Dado& dado) {
        buffer.emplace_back(chave, dado);
        if (buffer.size() >= TAMANHO_BUFFER) {
            inserirLote(buffer);
            buffer.clear();
        }
    }

    /**
        força a escrita dos registros restantes no buffer quando nao está cheio
        garante que nenhum registro fica pendente na memória
    */
    void flushBuffer() {
        if (!buffer.empty()) {
            inserirLote(buffer);
            buffer.clear();
        }
    }
};
