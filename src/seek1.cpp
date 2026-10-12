#include <cstdio>
#include <exception>
#include <string>

#include "bplustree.hpp"
#include "disk_record.hpp"
#include "hash_file.hpp"

int main(int argc, char** argv) {
    if (argc < 2) {
        std::fprintf(
            stderr,
            "uso: %s <id>\n",
            argv[0]
        );

        return 2;
    }

    try {
        const int id = std::stoi(argv[1]);

        const std::string data_path =
            "/data/db/artigos.dat";

        const std::string index_path =
            "/data/db/indice_id.dat";

        bptree::BPlusTree index;
        index.open(index_path);

        hashfile::HashFile hash;
        hash.open(data_path);

        index.reset_stats();
        hash.reset_stats();

        bptree::RecordAddress index_address{};

        const bool encontrado =
            index.search(
                id,
                index_address
            );

        if (!encontrado) {
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

        hashfile::RecordAddress data_address{
            index_address.block,
            index_address.slot
        };

        DiskArticle disco{};

        if (!hash.read_at(
                data_address,
                disco
            )) {
            std::fprintf(
                stderr,
                "erro ao acessar registro no arquivo de dados\n"
            );

            hash.close();
            index.close();

            return 1;
        }

        Article artigo =
            from_disk_article(disco);

        std::printf(
            "\n========== REGISTRO ==========\n"
        );

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
            "==============================\n"
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
            "bloco do registro: %llu\n",
            static_cast<unsigned long long>(
                data_address.block
            )
        );

        std::printf(
            "slot do registro: %u\n",
            data_address.slot
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