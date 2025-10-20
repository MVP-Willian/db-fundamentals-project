#include "parser.h"

/***
usando https://cplusplus.com/reference/sstream/stringstream/
// */

std::string removeAspas(const std::string& s) {
    std::string res = s;
    if(!res.empty() && res.front() == '"' && res.back() == '"'){
        res = res.substr(1, res.size() - 2);
    }
    return res;
}

int toInt(const std::string& s) {
    return s.empty() ? 0 : std::stoi(s);
}

std::vector<std::string> splitCSV(const std::string& linha) {
    std::vector<std::string> campos;
    std::string campo;
    bool dentroAspas = false;

    for (char c : linha) {
        if (c == '"') {
            dentroAspas = !dentroAspas;
        } else if (c == ';' && !dentroAspas) {
            campos.push_back(campo);
            campo.clear();
        } else {
            campo += c;
        }
    }
    campos.push_back(campo); // último campo
    // remover aspas externas
    for (auto& s : campos) s = removeAspas(s);
    return campos;
}

Artigo parseArtigo(const std::string& linha){
    std::stringstream linhaCampos(linha);
    std::string campo;
    std::vector<std::string> campos = splitCSV(linha);

    Artigo art;
    if(campos.size() >= 7){
        art.setId(toInt(campos[0]));
        art.setTitulo(campos[1].c_str());
        art.setAno(toInt(campos[2]));
        art.setAutores(campos[3].c_str());
        art.setCitacoes(toInt(campos[4]));
        art.setAtualizacao(campos[5].c_str());
        art.setSnippet(campos[6].c_str());
    }

    return art;
}
