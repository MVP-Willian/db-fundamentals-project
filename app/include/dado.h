#include <iostream>
#include <vector>
#include <string>

template <typename Chave>
using FilePointer = long

class Dado{
    private:
        Chave chave;
        FilePointer ponteiro;

    public:
        //construtor
        Dado();
        Dado(const Chave& chave, FilePointer ponteiro);

        //get e set
        void setChave(const Chave& chave){
            chave(chave);
        }
        void setPonteiro(FilePointer ponteiro){
            ponteiro(ponteiro);
        }

        FilePointer getLocal(){
            return ponteiro;
        }

        bool operator<(const Dado& outro){ return chave < outro.chave;}
        bool operator>(const Dado& outro){return chave > outro.chave;}
        bool operator==(const Dado& outro){return chave == outro.chave;}
};