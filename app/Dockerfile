# exemplo base: pode usar debian:bookworm-slim ou ubuntu:24.04
FROM ubuntu:24.04

RUN apt-get update && apt-get install -y --no-install-recommends \
      g++ make cmake && \
    rm -rf /var/lib/apt/lists/*

WORKDIR /app
COPY . /app

# compile usando Makefile padrão (ajuste conforme seu build)
RUN make build
RUN ls -l /app/bin

# diretório para dados persistentes (montado como volume)
VOLUME ["/data"]

# variáveis de ambiente para facilitar testes
ENV CSV_PATH=/data/input.csv \
    DATA_DIR=/data/db \
    LOG_LEVEL=info

# por padrão, mostra ajuda dos binários
CMD ["bash", "-c", "echo 'Use: docker run ... upload|findrec|seek1|seek2'; ls -l bin/"]
