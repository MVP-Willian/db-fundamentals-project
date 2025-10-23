.PHONY: test clean build docker-build docker-rebuild

# Diretórios
INC_DIR = include
SRC_DIR = src
TEST_DIR = test
BIN_DIR = bin

# adiciona aqui os nomes dos arquivos de teste (sem .cpp)
TESTS = test_logger test_artigo test_diskManager test_parser test_bplus
TESTS_EXEC = $(addprefix $(BIN_DIR)/, $(TESTS))

# Adicione aqui os arquivos .cpp correspondentes as bibliotecas (.h)
# Se criar novos módulos (ex: bplus.cpp), declare uma variável igual às abaixo.
LOGGER_SRC = $(SRC_DIR)/logger.cpp
ARTIGO_SRC = $(SRC_DIR)/artigo.cpp
DISKMANAGER_SRC = $(SRC_DIR)/diskManager.cpp
PARSER_SRC = $(SRC_DIR)/parser.cpp
# Exemplo de novo módulo:
# BPLUS_SRC = $(SRC_DIR)/bplus.cpp

# compilador e flags, nao mexer
CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++17 -I$(INC_DIR)

# nome da imagem docker
IMAGE_NAME = tp2-teste

# Nomes dos executáveis FINAIS (Requisitos do TP2)
#PROGS = upload findrec seek1 seek2
PROGS = upload
EXECS = $(addprefix $(BIN_DIR)/, $(PROGS))

all: docker-build

# COMPILAR TESTES
# Para incluir novos .cpp no build, adicione na lista de dependências e também
# antes do "-o" na última linha (logo dps de $(PARSER_SRC)).
$(BIN_DIR)/%: $(TEST_DIR)/%.cpp $(LOGGER_SRC) $(ARTIGO_SRC) $(DISKMANAGER_SRC) $(PARSER_SRC)
	@mkdir -p $(BIN_DIR)
	@echo "Compilando teste: $< -> $@"
	$(CXX) $(CXXFLAGS) $< $(LOGGER_SRC) $(ARTIGO_SRC) $(DISKMANAGER_SRC) $(PARSER_SRC) -o $@

# Copie esta regra para criar novos binários (findrec, seek1, seek2)
# Substitua 'upload' e adicione os arquivos .cpp na lista de dependências.
$(BIN_DIR)/upload: $(SRC_DIR)/upload.cpp $(LOGGER_SRC) $(ARTIGO_SRC) $(DISKMANAGER_SRC) $(PARSER_SRC)
	@echo "Compilando upload: $< -> $@"
	@mkdir -p $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $< $(LOGGER_SRC) $(ARTIGO_SRC) $(DISKMANAGER_SRC) $(PARSER_SRC) -o $@


# caminho padrao do csv, pode ser sobreescrito (make upload FILE=outro.csv)
FILE ?= data/artigo.csv

# compila todos os executaveis e testes (fora do docker)
build: $(EXECS) $(TESTS_EXEC) 
	@echo "BUILD COMPLETO. Todos os binários estão em $(BIN_DIR)/"

# execuções via docker:
upload:
	docker run --rm -v /data:/data tp2-teste /app/bin/upload $(FILE)

# cria a imagem docker e executa o make build acima, mas dentro do container
docker-build:
	@echo "Construindo imagem Docker ($(IMAGE_NAME))..."
	docker build . -t $(IMAGE_NAME)

# refaz a imagem toda
docker-rebuild:
	-docker ps -q --filter "ancestor=$(IMAGE_NAME)" | xargs -r docker stop
	-docker ps -a -q --filter "ancestor=$(IMAGE_NAME)" | xargs -r docker rm
	@echo "Reconstruindo imagem docker ($(IMAGE_NAME))"
	docker build . -t tp2-teste --no-cache

# Para adicionar novos testes, copie uma linha, mude o echo e o binário.
test: docker-build
	@echo "Executando teste de log: LOG_LEVEL=debug"
	docker run --rm $(IMAGE_NAME) /app/bin/test_logger

	@echo "Executando teste da classe Artigo"
	docker run --rm $(IMAGE_NAME) /app/bin/test_artigo

	@echo "Executando teste da classe DiskManager"
	docker run --rm $(IMAGE_NAME) /app/bin/test_diskManager

	@echo "Executando teste de Parser"
	docker run --rm $(IMAGE_NAME) /app/bin/test_parser
	
	@echo "Executando teste de B+tree"
	docker run --rm $(IMAGE_NAME) /app/bin/test_bplus

# regra para limpar bin
clean:
	@echo "Limpando arquivos compilados..."
	@rm -rf $(BIN_DIR)