# Trabalho Prático II — Introdução a Bancos de Dados

Universidade Federal do Amazonas — UFAM  
Instituto de Computação — IComp  
Disciplina: ICC205 — Introdução a Bancos de Dados

## 1. Descrição

Este projeto implementa armazenamento de registros em memória secundária
utilizando organização por hashing e índices B+Tree.

A base contém artigos científicos armazenados no arquivo `artigos.dat`.

Foram implementados:

- arquivo hash para armazenamento dos registros;
- tratamento de overflow nos buckets;
- índice B+Tree por identificador;
- índice B+Tree secundário por título;
- busca pontual por identificador;
- busca por título;
- busca por intervalo de identificadores.

Os índices armazenam o endereço físico dos registros no formato:

text
(bloco, slot)



# data/

Este diretório fica **vazio no repositório** e é usado em tempo de execução.

1. Baixe a amostra oficial [`artigo-amostra.csv.gz`](https://drive.google.com/file/d/1mUXzZ3ckewhY5uEQsCA0CP4kRjVhxFY8/view?usp=sharing) e coloque-a aqui.
2. Descompacte antes da primeira carga:

gzip -dk data/artigo-amostra.csv.gz


3. O diretório é montado como `/data` no contêiner. Os arquivos de dados e de
   índice produzidos pelo trabalho devem ser gravados em `/data/db`, de modo que
   uma execução aproveite os arquivos criados pela anterior.

Não versione o CSV nem os arquivos gerados.
