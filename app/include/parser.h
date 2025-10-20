#ifndef PARSER_H
#define PARSER_H

#include "artigo.h"
#include "logger.h"
#include <fstream>
#include <vector>
#include <cstring>

std::string removeAspas(const std::string& s);

Artigo parseArtigo(const std::string& linha);

#endif
