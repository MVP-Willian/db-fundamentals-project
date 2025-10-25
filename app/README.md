# db-fundamentals-project

## Testando execuções de /test
```bash
make docker-build
```
depois de terminar o build, execute:
```bash
make test
```
---
## Usando o programa
Para iniciar e compilar os programas use o comando make para carregar a imagem docker
```bash
make
```
### Upload
Carregue o programa com dados de um csv:
```bash
make upload FILE=<localizacao_do_csv.csv>
```
Este passo é equivalente ao comando docker:
```bash
docker run --rm -v $(PWD)/data:/data tp2 /app/bin/upload data/input.csv
```
---
### findrec

---
### Seek1
Para buscar, você pode usar o comando make abaixo:
```bash
make seek1 ID=<id_de_busca>
```
Esse passo é equivalente ao comando docker:
```bash
docker run --rm -v $(PWD)/data:/data tp2 /app/bin/seek1 <id_aqui>
```
O programa retorna o dado quando a busca encontra seu dado, o retorno padrão para erro é 1

---
### Seek2