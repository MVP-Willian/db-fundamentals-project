.PHONY: test-logger clean build

# Diretórios
INC_DIR = include
SRC_DIR = src
TEST_DIR = test
BIN_DIR = bin

# Nomes dos arquivos de origem e destino
TESTS = test_logger test_artigo test_diskManager
TESTS_EXEC = $(addprefix $(BIN_DIR)/, $(TESTS))
LOGGER_SRC = $(SRC_DIR)/logger.cpp
ARTIGO_SRC = $(SRC_DIR)/artigo.cpp
DISKMANAGER_SRC = $(SRC_DIR)/diskManager.cpp

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

# regra genérica para qualquer teste em test/*.cpp
# transforma test/test_foo.cpp -> bin/test_foo
$(BIN_DIR)/%: $(TEST_DIR)/%.cpp $(LOGGER_SRC) $(ARTIGO_SRC) $(DISKMANAGER_SRC)
	@mkdir -p $(BIN_DIR)
	@echo "Compilando teste: $< -> $@"
	$(CXX) $(CXXFLAGS) $< $(LOGGER_SRC) $(ARTIGO_SRC) $(DISKMANAGER_SRC) -o $@

# aqui ele deve compilar todos os executaveis, por enquanto só o teste
# ele compila localmente e nao manda para o docker
build: $(EXECS) $(TESTS_EXEC)
	@echo "BUILD COMPLETO. Todos os binários estão em $(BIN_DIR)/"

# Regra genérica para compilar os programas finais (upload, findrec, seek1, seek2)
# % é um curinga. Esta regra ensina o make a transformar src/main_X.cpp em bin/X
$(BIN_DIR)/%: $(SRC_DIR)/main_%.cpp $(LOGGER_SRC) $(ARTIGO_SRC) $(DISKMANAGER_SRC)
	@echo "Compilando programa principal: $< -> $@"
	@mkdir -p $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $< $(LOGGER_SRC) $(ARTIGO_SRC) $(DISKMANAGER_SRC) -o $@

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
	docker run --rm $(IMAGE_NAME) /app/bin/test_logger

	@echo "Executando teste da classe Artigo: LOG_LEVEL=debug"
	docker run --rm $(IMAGE_NAME) /app/bin/test_artigo
	@echo "Executando teste da classe DiskManager: LOG_LEVEL=debug"
	docker run --rm $(IMAGE_NAME) /app/bin/test_diskManager

# regra para limpar bin
clean:
	@echo "Limpando arquivos compilados..."
	@rm -rf $(BIN_DIR)