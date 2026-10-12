#include <cstdio>
#include <exception>
#include <string>
#include <algorithm>

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
        int inicio = std::stoi(argv[1]);
        int fim = std::stoi(argv[2]);

        if (inicio > fim) {
            std::swap(inicio, fim);
        }

        const std::string data_path =
            "/data/db/artigos.dat";

        hashfile::HashFile hash;
        hash.open(data_path);

        hash.reset_stats();

        std::printf(
            "\n========== REGISTROS NO INTERVALO ==========\n"
        );

        const std::uint64_t encontrados =
            hash.find_range(inicio, fim);

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