#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <exception>
#include <string>
#include <vector>

#include "bplustree.hpp"
#include "disk_record.hpp"
#include "hash_file.hpp"

int main(int argc, char** argv) {
    if (argc < 3) {
        std::fprintf(
            stderr,
            "uso: %s <inicio> <fim>\n",
            argv[0]
        );

        return 2;
    }

    try {
        int inicio =
            std::stoi(argv[1]);

        int fim =
            std::stoi(argv[2]);

        if (inicio > fim) {
            std::swap(
                inicio,
                fim
            );
        }

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

        std::vector<bptree::RangeEntry>
            resultados;

        index.range_search(
            inicio,
            fim,
            resultados
        );

        std::printf(
            "\n========== REGISTROS NO INTERVALO ==========\n"
        );

        std::uint64_t encontrados = 0;

        for (const auto& entry : resultados) {
            hashfile::RecordAddress endereco{
                entry.address.block,
                entry.address.slot
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

            std::printf(
                "id: %d | titulo: %s\n",
                artigo.id,
                artigo.titulo.c_str()
            );

            ++encontrados;
        }

        std::printf(
            "============================================\n"
        );

        std::printf(
            "intervalo: [%d, %d]\n",
            inicio,
            fim
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