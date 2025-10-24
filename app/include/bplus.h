#include <iostream>
#include <string>
#include <stdexcept>
#include <queue>
#include "logger.h"

template <typename Chave, typename Dado>
class No {
public:
    int numChaves;
    Chave *chaves;
    bool folha;

    No(int ordem, bool ehFolha) : numChaves(0), folha(ehFolha) {
        // ordem (t) é o grau mínimo da b-tree do Cormen
        // número máximo de chaves é 2t.
        chaves = new Chave[2 * ordem + 1];
    }
    virtual ~No() { delete[] chaves; }
};

template <typename Chave, typename Dado>
class NoInterno : public No<Chave, Dado> {
public:
    No<Chave, Dado> **filhos; //ponteiro pros filhos

    NoInterno(int ordem) : No<Chave, Dado>(ordem, false) {
        this->filhos = new No<Chave, Dado>*[2 * ordem + 2]; // 2t+1 filhos possiveis
    }
};

template <typename Chave, typename Dado>
class NoFolha : public No<Chave, Dado> {
public:
    Dado *dados; // vetor de dados ligados com as chaves
    NoFolha<Chave, Dado> *proximo; // ponteiro pra próxima folha

    NoFolha(int ordem) : No<Chave, Dado>(ordem, true), proximo(nullptr) {
        this->dados = new Dado[2 * ordem + 1]; // armazenamento para os dados
    }
    // Implementação de Busca/Inserção/Remoção específica para nó folha (armazena dados)
};

//pra imprimir qualquer coisa desse fresco de string e int
template<typename T>
std::string to_string_generic(const T& val) {
    if constexpr (std::is_same_v<T, std::string>) return val;
    else return std::to_string(val);
}

template <typename Chave, typename Dado>
class BPlusTree {
private:
    No<Chave, Dado> *raiz;
    int ordem; //ordem da b+
    Logger logger;

    // divide um nó flolha quando ta cheio e retorna a nova folha, atualiza a chave subida
    NoFolha<Chave, Dado>* splitFolha(NoFolha<Chave, Dado>* folha, Chave& chavePromovida) {
        int t = ordem;
        auto novaFolha = new NoFolha<Chave, Dado>(t);
        novaFolha->numChaves = t; // metade das chaves vai pra nova folha

        // copia a metade das chaves e dados
        for (int i = 0; i < t; i++) {
            novaFolha->chaves[i] = folha->chaves[i + t];
            novaFolha->dados[i] = folha->dados[i + t];
        }

        // atualiza numero de chaves da folha original
        folha->numChaves = t;

        // encadeia as folhas
        novaFolha->proximo = folha->proximo;
        folha->proximo = novaFolha;

        chavePromovida = novaFolha->chaves[0];

        //debug pa acompanhar
        logger.debug("Split de folha: promovendo chave " + to_string_generic(chavePromovida));
        logger.debug("Folha original antes do split: " + imprimirChaves(folha));
        logger.debug("Nova folha criada: " + imprimirChaves(novaFolha));
        return novaFolha;
    }

    // divide um nó interno quando encher, retorna o novo no interno e atualiza a chave subida
    NoInterno<Chave, Dado>* splitInterno(NoInterno<Chave, Dado>* interno, Chave& chavePromovida) {
        int t = ordem;
        auto novoInterno = new NoInterno<Chave, Dado>(t);

        // chave do meio vai subir
        chavePromovida = interno->chaves[t];

        // copia a metade das chaves e filhos pro novo nó
        novoInterno->numChaves = t - 1;
        for (int i = 0; i < t - 1; i++) {
            novoInterno->chaves[i] = interno->chaves[i + t + 1];
        }
        for(int i = 0; i < t; i++)
            novoInterno->filhos[i] = interno->filhos[i + t + 1];

        interno->numChaves = t;

        //debug pra acompanhar
        logger.debug("Split de nó interno: promovendo chave " + to_string_generic(chavePromovida));
        logger.debug("Nó interno original: " + imprimirChaves(interno));
        logger.debug("Novo nó interno: " + imprimirChaves(novoInterno));

        return novoInterno;
    }

    // inserção recursiva igual arvore msm
    No<Chave, Dado>* inserirRec(No<Chave, Dado>* no, const Chave& chave, const Dado& dado, Chave& chavePromovida) {
        if (no->folha) {
            auto folha = dynamic_cast<NoFolha<Chave, Dado>*>(no);

            // inserir ordenadamente dentro da folha
            int i = folha->numChaves - 1;
            while (i >= 0 && chave < folha->chaves[i]) {
                folha->chaves[i + 1] = folha->chaves[i];
                folha->dados[i + 1] = folha->dados[i];
                i--;
            }
            folha->chaves[i + 1] = chave;
            folha->dados[i + 1] = dado;
            folha->numChaves++;

            // se a folha encheu, split
            if (folha->numChaves == 2 * ordem) {
                return splitFolha(folha, chavePromovida);
            }
            return nullptr;
        }

        // para nó interno
        auto interno = dynamic_cast<NoInterno<Chave, Dado>*>(no);
        int i = interno->numChaves - 1;
        while (i >= 0 && chave < interno->chaves[i]) i--;
        i++;

        // desce recursivamente
        Chave chaveSubida;
        logger.debug("Descendo para o filho " + std::to_string(i) + " da chave " + to_string_generic(chave));
        No<Chave, Dado>* novoFilho = inserirRec(interno->filhos[i], chave, dado, chaveSubida);

        if (novoFilho) {
            // insere chaveSubida no nó interno
            int j = interno->numChaves - 1;
            while (j >= 0 && chaveSubida < interno->chaves[j]) {
                interno->chaves[j + 1] = interno->chaves[j];
                interno->filhos[j + 2] = interno->filhos[j + 1];
                j--;
            }
            interno->chaves[j + 1] = chaveSubida;
            interno->filhos[j + 2] = novoFilho;
            interno->numChaves++;

            // Se o nó interno encheu, split
            if (interno->numChaves == 2 * ordem) {
                return splitInterno(interno, chavePromovida);
            }
        }

        return nullptr;
    }

    //faz string de todas as chaves de um nó
    std::string imprimirChaves(No<Chave, Dado>* no) {
        std::string s = "[";
        for (int i = 0; i < no->numChaves; i++) {
            s += to_string_generic(no->chaves[i]);
            if (i < no->numChaves - 1) s += ",";
        }
        s += "]";
        return s;
    }


public:
    BPlusTree(int t) : raiz(nullptr), ordem(t) {}

    //tipo uma api pra chamar o inserirRec
    void inserir(const Chave& chave, const Dado& dado) {
        if (!raiz) {
            raiz = new NoFolha<Chave, Dado>(ordem);
            auto folha = dynamic_cast<NoFolha<Chave, Dado>*>(raiz);
            folha->chaves[0] = chave;
            folha->dados[0] = dado;
            folha->numChaves = 1;
            return;
        }

        // inserção com split em cascata
        Chave chavePromovida;
        No<Chave, Dado>* novoFilho = inserirRec(raiz, chave, dado, chavePromovida);

        if (novoFilho) {
            // cria nova raiz se a antiga se dividiu
            auto novaRaiz = new NoInterno<Chave, Dado>(ordem);
            novaRaiz->chaves[0] = chavePromovida;
            novaRaiz->filhos[0] = raiz;
            novaRaiz->filhos[1] = novoFilho;
            novaRaiz->numChaves = 1;
            raiz = novaRaiz;
        }
    }

    // busca o dado na arvore
    Dado buscar(const Chave& chave) {
        No<Chave, Dado>* atual = raiz;
        logger.debug("Buscando chave " + to_string_generic(chave) + " no nó " + imprimirChaves(atual));
        while (atual && !atual->folha) {
            auto interno = dynamic_cast<NoInterno<Chave, Dado>*>(atual);
            int i = 0;
            while (i < interno->numChaves && chave >= interno->chaves[i]) i++;
            atual = interno->filhos[i];
        }

        if (!atual) throw std::runtime_error("Chave não encontrada");
        auto folha = dynamic_cast<NoFolha<Chave, Dado>*>(atual);
        for (int i = 0; i < folha->numChaves; i++) {
            if (folha->chaves[i] == chave) return folha->dados[i];
        }
        throw std::runtime_error("Chave não encontrada");
    }

    //pra moostrar como a arvore ta
    void imprimir() {
        if (!raiz) {
            std::cout << "[árvore vazia]\n";
            return;
        }

        std::queue<std::pair<No<Chave, Dado>*, int>> fila;
        fila.push({raiz, 0});

        int nivelAtual = 0;
        std::cout << "Nível 0: ";

        while (!fila.empty()) {
            auto [no, nivel] = fila.front();
            fila.pop();

            if (nivel != nivelAtual) {
                nivelAtual = nivel;
                std::cout << "\nNível " << nivelAtual << ": ";
            }

            std::cout << "[";
            for (int i = 0; i < no->numChaves; i++) {
                std::cout << no->chaves[i];
                if (i < no->numChaves - 1) std::cout << ",";
            }
            std::cout << "] ";

            if (!no->folha) {
                auto interno = dynamic_cast<NoInterno<Chave, Dado>*>(no);
                for (int i = 0; i <= interno->numChaves; i++) {
                    fila.push({interno->filhos[i], nivel + 1});
                }
            }
        }

        std::cout << "\n";
    }

    //mostra o ultimmo nivel da arvore
    void imprimirFolhas() {
        if (!raiz) {
            std::cout << "[árvore vazia]\n";
            return;
        }

        // a primeira folha
        No<Chave, Dado>* atual = raiz;
        while (!atual->folha) {
            auto interno = dynamic_cast<NoInterno<Chave, Dado>*>(atual);
            atual = interno->filhos[0];
        }

        std::cout << "Folhas (da esquerda para direita):\n";
        int idx = 0;
        auto folha = dynamic_cast<NoFolha<Chave, Dado>*>(atual);
        while (folha) {
            std::cout << "Folha " << idx++ << ": [";
            for (int i = 0; i < folha->numChaves; i++) {
                std::cout << folha->chaves[i];
                if (i < folha->numChaves - 1) std::cout << ", ";
            }
            std::cout << "]";
            if (folha->proximo) std::cout << " -> ";
            folha = folha->proximo;
        }
        std::cout << "\n";
    }

};