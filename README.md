# TP2 - Introdução a Bancos de Dados (ICC205 - 2026/02)

Implementação do Trabalho Prático II sobre armazenamento em memória secundária, hashing e índices B+Tree.

## Estrutura implementada

- Arquivo de dados: `/data/db/artigos.dat`, organizado por hashing de ID com `50000` buckets e blocos de overflow.
- Índice primário: `/data/db/indice_id.dat`, B+Tree por ID.
- Índice secundário: `/data/db/indice_titulo.dat`, B+Tree por título, permitindo chaves duplicadas.
- Todos os acessos aos arquivos de dados e índices passam pela camada `blockio`.

## Compilação local

```bash
make build
make BLOCK_SIZE=8192 build
```

Os binários são gerados em `bin/`:

```text
upload
findrec
findtitle
findrange
seek1
seek2
seekrange
```

## Docker - bloco de 4096 bytes

```bash
docker build --build-arg BLOCK_SIZE=4096 -t tp2 .
docker run --rm -v "${PWD}/data:/data" tp2 ./bin/upload /data/artigo-amostra.csv
docker run --rm -v "${PWD}/data:/data" tp2 ./bin/findrec 5
docker run --rm -v "${PWD}/data:/data" tp2 ./bin/findtitle "Design and implementation of an immersive virtual reality system based on a smartphone platform."
docker run --rm -v "${PWD}/data:/data" tp2 ./bin/findrange 1000 1100
docker run --rm -v "${PWD}/data:/data" tp2 ./bin/seek1 5
docker run --rm -v "${PWD}/data:/data" tp2 ./bin/seek2 "Design and implementation of an immersive virtual reality system based on a smartphone platform."
docker run --rm -v "${PWD}/data:/data" tp2 ./bin/seekrange 1000 1100
```

No Linux/macOS também podem ser usados os alvos do Makefile:

```bash
make docker-build
make docker-run-upload ARGS="/data/artigo-amostra.csv"
make docker-run-findrec ARGS="5"
make docker-run-findtitle ARGS='"Design and implementation of an immersive virtual reality system based on a smartphone platform."'
make docker-run-findrange ARGS="1000 1100"
make docker-run-seek1 ARGS="5"
make docker-run-seek2 ARGS='"Design and implementation of an immersive virtual reality system based on a smartphone platform."'
make docker-run-seekrange ARGS="1000 1100"
```

## Docker - bloco de 8192 bytes

Para não misturar arquivos criados com tamanhos de bloco diferentes, use outro diretório no host:

```bash
mkdir -p data8192
cp data/artigo-amostra.csv data8192/artigo-amostra.csv

docker build --build-arg BLOCK_SIZE=8192 -t tp2-8192 .
docker run --rm -v "${PWD}/data8192:/data" tp2-8192 ./bin/upload /data/artigo-amostra.csv
docker run --rm -v "${PWD}/data8192:/data" tp2-8192 ./bin/seek1 5
```

Equivalente pelo Makefile:

```bash
make BLOCK_SIZE=8192 IMAGE=tp2-8192 docker-build
```

## Arquivos gerados em `/data`

Após `upload`, a estrutura é:

```text
/data/
├── artigo-amostra.csv
└── db/
    ├── artigos.dat
    ├── indice_id.dat
    └── indice_titulo.dat
```

Os arquivos `.dat` são persistidos no diretório montado pelo Docker e reutilizados pelas consultas seguintes.

## Consultas oficiais

As consultas que devem ser executadas no relatório estão em:

```text
queries/oficial.txt
```

Elas devem ser executadas tanto com `BLOCK_SIZE=4096` quanto com `BLOCK_SIZE=8192`.

## Exemplo pequeno de entrada e saída

Com a base já carregada:

```bash
docker run --rm -v "${PWD}/data:/data" tp2 ./bin/findrec 5
```

Saída resumida esperada:

```text
id: 5
...
blocos lidos: 1
total de blocos do arquivo: 69940
arquivo de dados: /data/db/artigos.dat
```

Consulta equivalente usando o índice primário:

```bash
docker run --rm -v "${PWD}/data:/data" tp2 ./bin/seek1 5
```

Saída resumida para 4096 bytes:

```text
id: 5
...
blocos lidos no indice: 4
total de blocos do indice: 2877
blocos lidos no arquivo de dados: 1
total de blocos do arquivo de dados: 69940
arquivo de indice: /data/db/indice_id.dat
arquivo de dados: /data/db/artigos.dat
```

## Relatório

O relatório final está em:

```text
docs/tp2_relatorio.pdf
```

## Entrega

O repositório deve permanecer privado, com o professor como colaborador. A versão final deve ser marcada com:

```bash
git tag v1.0
git push origin v1.0
git ls-remote --tags origin
```
