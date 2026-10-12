#include <cstdio>
#include <exception>
#include <string>
#include <vector>

#include "disk_record.hpp"
#include "hash_file.hpp"
#include "title_bplustree.hpp"

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

        const std::string index_path =
            "/data/db/indice_titulo.dat";

        titlebptree::TitleBPlusTree index;
        index.open(index_path);

        hashfile::HashFile hash;
        hash.open(data_path);

        index.reset_stats();
        hash.reset_stats();

        std::vector<titlebptree::SearchResult>
            resultados;

        index.search(
            titulo,
            resultados
        );

        if (resultados.empty()) {
            std::printf(
                "registro nao encontrado\n"
            );

            std::printf(
                "blocos lidos no indice: %llu\n",
                static_cast<unsigned long long>(
                    index.stats().reads
                )
            );

            std::printf(
                "total de blocos do indice: %llu\n",
                static_cast<unsigned long long>(
                    index.block_count()
                )
            );

            std::printf(
                "blocos lidos no arquivo de dados: %llu\n",
                static_cast<unsigned long long>(
                    hash.stats().reads
                )
            );

            std::printf(
                "total de blocos do arquivo de dados: %llu\n",
                static_cast<unsigned long long>(
                    hash.block_count()
                )
            );

            std::printf(
                "arquivo de indice: %s\n",
                index_path.c_str()
            );

            std::printf(
                "arquivo de dados: %s\n",
                data_path.c_str()
            );

            hash.close();
            index.close();

            return 1;
        }

        std::printf(
            "\n========== RESULTADOS ==========\n"
        );

        std::uint64_t encontrados = 0;

        for (const auto& resultado : resultados) {
            hashfile::RecordAddress endereco{
                resultado.address.block,
                resultado.address.slot
            };

            DiskArticle disco{};

            if (!hash.read_at(
                    endereco,
                    disco
                )) {
                continue;
            }

            Article artigo =
                from_disk_article(disco);

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

            std::printf(
                "bloco: %llu | slot: %u\n",
                static_cast<unsigned long long>(
                    endereco.block
                ),
                endereco.slot
            );

            ++encontrados;
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
            "blocos lidos no indice: %llu\n",
            static_cast<unsigned long long>(
                index.stats().reads
            )
        );

        std::printf(
            "total de blocos do indice: %llu\n",
            static_cast<unsigned long long>(
                index.block_count()
            )
        );

        std::printf(
            "blocos lidos no arquivo de dados: %llu\n",
            static_cast<unsigned long long>(
                hash.stats().reads
            )
        );

        std::printf(
            "total de blocos do arquivo de dados: %llu\n",
            static_cast<unsigned long long>(
                hash.block_count()
            )
        );

        std::printf(
            "arquivo de indice: %s\n",
            index_path.c_str()
        );

        std::printf(
            "arquivo de dados: %s\n",
            data_path.c_str()
        );

        hash.close();
        index.close();

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