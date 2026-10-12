#ifndef DISK_RECORD_HPP
#define DISK_RECORD_HPP

#include <cstring>
#include <string>

#include "record.hpp"

constexpr std::size_t TITLE_SIZE = 301;
constexpr std::size_t AUTHORS_SIZE = 151;
constexpr std::size_t UPDATE_SIZE = 20;
constexpr std::size_t SNIPPET_SIZE = 512;

struct DiskArticle {
    int id;
    char titulo[TITLE_SIZE];
    int ano;
    char autores[AUTHORS_SIZE];
    int citacoes;
    char atualizacao[UPDATE_SIZE];
    char snippet[SNIPPET_SIZE];
};

inline void copy_text(char* destino, std::size_t tamanho, const std::string& texto) {
    if (tamanho == 0) return;

    std::strncpy(destino, texto.c_str(), tamanho - 1);
    destino[tamanho - 1] = '\0';
}

inline DiskArticle to_disk_article(const Article& artigo) {
    DiskArticle disco{};

    disco.id = artigo.id;
    disco.ano = artigo.ano;
    disco.citacoes = artigo.citacoes;

    copy_text(disco.titulo, TITLE_SIZE, artigo.titulo);
    copy_text(disco.autores, AUTHORS_SIZE, artigo.autores);
    copy_text(disco.atualizacao, UPDATE_SIZE, artigo.atualizacao);
    copy_text(disco.snippet, SNIPPET_SIZE, artigo.snippet);

    return disco;
}

inline Article from_disk_article(const DiskArticle& disco) {
    Article artigo;

    artigo.id = disco.id;
    artigo.titulo = disco.titulo;
    artigo.ano = disco.ano;
    artigo.autores = disco.autores;
    artigo.citacoes = disco.citacoes;
    artigo.atualizacao = disco.atualizacao;
    artigo.snippet = disco.snippet;

    return artigo;
}

#endif