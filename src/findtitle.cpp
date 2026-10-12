#include <cstdio>
#include <exception>
#include <string>
#include <vector>

#include "disk_record.hpp"
#include "hash_file.hpp"

int main(int argc, char** argv) {
    if (argc < 2) {
        std::fprintf(
            stderr,
            "uso: %s <titulo>\n",
            argv[0]
        );

        return 2;
    }

    try {
        const std::string titulo =
            argv[1];

        const std::string data_path =
            "/data/db/artigos.dat";

        hashfile::HashFile hash;

        hash.open(
            data_path
        );

        hash.reset_stats();

        std::vector<DiskArticle> resultados;

        const std::uint64_t encontrados =
            hash.find_all_by_title(
                titulo,
                resultados
            );

        if (encontrados == 0) {
            std::printf(
                "registro nao encontrado\n"
            );

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

        std::printf(
            "\n========== RESULTADOS ==========\n"
        );

        for (
            const DiskArticle& disco :
            resultados
        ) {
            Article artigo =
                from_disk_article(
                    disco
                );

            std::printf("\n");

            std::printf(
                "id: %d\n",
                artigo.id
            );

            std::printf(
                "titulo: %s\n",
                artigo.titulo.c_str()
            );

            std::printf(
                "ano: %d\n",
                artigo.ano
            );

            std::printf(
                "autores: %s\n",
                artigo.autores.c_str()
            );

            std::printf(
                "citacoes: %d\n",
                artigo.citacoes
            );

            std::printf(
                "atualizacao: %s\n",
                artigo.atualizacao.c_str()
            );

            std::printf(
                "snippet: %s\n",
                artigo.snippet.c_str()
            );
        }

        std::printf(
            "\n================================\n"
        );

        std::printf(
            "registros encontrados: %llu\n",
            static_cast<unsigned long long>(
                encontrados
            )
        );

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

    } catch (
        const std::exception& e
    ) {
        std::fprintf(
            stderr,
            "erro: %s\n",
            e.what()
        );

        return 1;
    }
}