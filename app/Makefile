.PHONY: test-logger clean build

# Diretórios
INC_DIR = include
SRC_DIR = src
TEST_DIR = test
BIN_DIR = bin

# Nomes dos arquivos de origem e destino
TEST_SRC = $(TEST_DIR)/test_logger.cpp
LOGGER_SRC = $(SRC_DIR)/logger.cpp
TEST_EXEC = $(BIN_DIR)/test_logger

# Flags do compilador: -I$(INC_DIR) diz ao g++ onde procurar os .h
CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++17 -I$(INC_DIR)

# constantes
IMAGE_NAME = tp2-teste

# Nomes dos executáveis FINAIS (Requisitos do TP2)
PROGS = upload findrec seek1 seek2

# cria os caminhos bin/upload, bin/findrec, etc.
EXECS = $(addprefix $(BIN_DIR)/, $(PROGS))

all: docker-build

# regra unica para construir qualquer executável de teste
# Ele depende dos arquivos .cpp do teste e do logger
$(TEST_EXEC): $(TEST_SRC) $(LOGGER_SRC)
	@mkdir -p $(BIN_DIR)
	@echo "Compilando: $< e $(LOGGER_SRC) -> $(TEST_EXEC)"
	$(CXX) $(CXXFLAGS) $(TEST_SRC) $(LOGGER_SRC) -o $(TEST_EXEC)

# aqui ele deve compilar todos os executaveis, por enquanto só o teste
# ele compila localmente e nao manda para o docker
build: $(EXECS) $(TEST_EXEC)
	@echo "BUILD COMPLETO. Todos os binários estão em $(BIN_DIR)/"

# Regra genérica para compilar os programas finais (upload, findrec, seek1, seek2)
# % é um curinga. Esta regra ensina o make a transformar src/main_X.cpp em bin/X
$(BIN_DIR)/%: $(SRC_DIR)/main_%.cpp $(LOGGER_SRC)
	@echo "Compilando programa principal: $< -> $@"
	@mkdir -p $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $< $(LOGGER_SRC) -o $@

# cria a imagem docker
# dentro do dockerfile tem um RUN make build pra gerar os
# binarios dentro do container :)
docker-build:
	@echo "Construindo imagem Docker ($(IMAGE_NAME))..."
	docker build . -t $(IMAGE_NAME)

docker-rebuild:
	@echo "Reconstruindo imagem docker ($(IMAGE_NAME))"
	docker build . -t tp2-teste --no-cache

# constroi a imagem se precisar e executa os testes
test: docker-build
	@echo "Executando teste de log: LOG_LEVEL=debug"
	docker run --rm \
	    $(IMAGE_NAME) \
	    bash -c 'LOG_LEVEL=debug /app/bin/test_logger'

# regra para limpar bin
clean:
	@echo "Limpando arquivos compilados..."
	@rm -rf $(BIN_DIR)