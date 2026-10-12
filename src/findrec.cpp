#include <cstdio>
#include <cstdlib>
#include <exception>
#include <string>

#include "disk_record.hpp"
#include "hash_file.hpp"

int main(int argc, char** argv) {
    if (argc < 2) {
        std::fprintf(stderr, "uso: %s <id>\n", argv[0]);
        return 2;
    }

    try {
        const int id = std::stoi(argv[1]);
        const std::string data_path = "/data/db/artigos.dat";

        hashfile::HashFile hash;
        hash.open(data_path);

        hash.reset_stats();

        DiskArticle disco{};

        const bool encontrado =
            hash.find_by_id(id, disco);

        if (!encontrado) {
            std::printf("registro nao encontrado\n");

            std::printf(
                "blocos lidos: %llu\n",
                static_cast<unsigned long long>(
                    hash.stats().reads
                )
            );

            std::printf(
                "total de blocos do arquivo: %llu\n",
                static_cast<unsigned long long>(
                    hash.block_count()
                )
            );

            std::printf(
                "arquivo de dados: %s\n",
                data_path.c_str()
            );

            hash.close();

            return 1;
        }

        Article artigo = from_disk_article(disco);

        std::printf("\n");
        std::printf("========== REGISTRO ==========\n");
        std::printf("id: %d\n", artigo.id);
        std::printf("titulo: %s\n", artigo.titulo.c_str());
        std::printf("ano: %d\n", artigo.ano);
        std::printf("autores: %s\n", artigo.autores.c_str());
        std::printf("citacoes: %d\n", artigo.citacoes);
        std::printf(
            "atualizacao: %s\n",
            artigo.atualizacao.c_str()
        );
        std::printf(
            "snippet: %s\n",
            artigo.snippet.c_str()
        );

        std::printf("==============================\n");

        std::printf(
            "blocos lidos: %llu\n",
            static_cast<unsigned long long>(
                hash.stats().reads
            )
        );

        std::printf(
            "total de blocos do arquivo: %llu\n",
            static_cast<unsigned long long>(
                hash.block_count()
            )
        );

        std::printf(
            "arquivo de dados: %s\n",
            data_path.c_str()
        );

        hash.close();

        return 0;

    } catch (const std::exception& e) {
        std::fprintf(
            stderr,
            "erro: %s\n",
            e.what()
        );

        return 1;
    }
}