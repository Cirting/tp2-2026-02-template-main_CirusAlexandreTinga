#include <cstdio>
#include <cstdint>
#include <exception>
#include <filesystem>
#include <string>

#include "blockio.hpp"
#include "bplustree.hpp"
#include "csv_reader.hpp"
#include "disk_record.hpp"
#include "hash_file.hpp"
#include "title_bplustree.hpp"

int main(int argc, char** argv) {
    if (argc < 2) {
        std::fprintf(
            stderr,
            "uso: %s <arquivo.csv>\n",
            argv[0]
        );

        return 2;
    }

    const std::string csv_path =
        argv[1];

    const std::string data_path =
        "/data/db/artigos.dat";

    const std::string index_id_path =
        "/data/db/indice_id.dat";

    const std::string index_title_path =
        "/data/db/indice_titulo.dat";

    try {
        std::filesystem::create_directories(
            "/data/db"
        );

        CsvReader reader(
            csv_path
        );

        /*
         * Arquivo principal organizado por hashing.
         */
        hashfile::HashFile hash;

        hash.create(
            data_path
        );

        /*
         * Indice primario:
         * ID -> endereco do registro.
         */
        bptree::BPlusTree index_id;

        index_id.create(
            index_id_path
        );

        /*
         * Indice secundario:
         * titulo -> endereco(s) do registro.
         */
        titlebptree::TitleBPlusTree
            index_title;

        index_title.create(
            index_title_path
        );

        Article artigo;

        std::uint64_t registros = 0;

        while (
            reader.next(artigo)
        ) {
            /*
             * Converte o registro do CSV
             * para o formato fisico.
             */
            DiskArticle disco =
                to_disk_article(
                    artigo
                );

            /*
             * Grava no arquivo hash.
             *
             * A insercao devolve o bloco
             * e o slot onde o registro
             * ficou armazenado.
             */
            hashfile::RecordAddress
                endereco =
                    hash.insert(
                        disco
                    );

            /*
             * Endereco para a B+Tree por ID.
             */
            bptree::RecordAddress
                endereco_id{
                    endereco.block,
                    endereco.slot
                };

            index_id.insert(
                artigo.id,
                endereco_id
            );

            /*
             * Endereco para a B+Tree por titulo.
             */
            titlebptree::RecordAddress
                endereco_titulo{
                    endereco.block,
                    endereco.slot
                };

            index_title.insert(
                artigo.titulo,
                endereco_titulo
            );

            ++registros;
        }

        const std::uint64_t blocos_dados =
            hash.block_count();

        const std::uint64_t blocos_indice_id =
            index_id.block_count();

        const std::uint64_t blocos_indice_titulo =
            index_title.block_count();

        const double ocupacao =
            blocos_dados == 0
                ? 0.0
                : (
                    static_cast<double>(
                        registros
                    ) *
                    sizeof(DiskArticle) /
                    (
                        static_cast<double>(
                            blocos_dados
                        ) *
                        blockio::kBlockSize
                    )
                ) * 100.0;

        std::printf("\n");

        std::printf(
            "========== RESULTADO DA CARGA ==========\n"
        );

        std::printf(
            "registros carregados: %llu\n",
            static_cast<unsigned long long>(
                registros
            )
        );

        std::printf(
            "linhas ignoradas: %llu\n",
            static_cast<unsigned long long>(
                reader.skipped()
            )
        );

        std::printf(
            "tamanho do registro em disco: %zu bytes\n",
            sizeof(DiskArticle)
        );

        std::printf(
            "tamanho do bloco: %zu bytes\n",
            blockio::kBlockSize
        );

        std::printf(
            "registros por bloco: %zu\n",
            hashfile::RECORDS_PER_BLOCK
        );

        std::printf(
            "numero de buckets: %llu\n",
            static_cast<unsigned long long>(
                hashfile::NUM_BUCKETS
            )
        );

        std::printf(
            "blocos do arquivo de dados: %llu\n",
            static_cast<unsigned long long>(
                blocos_dados
            )
        );

        std::printf(
            "blocos do indice B+Tree por ID: %llu\n",
            static_cast<unsigned long long>(
                blocos_indice_id
            )
        );

        std::printf(
            "blocos do indice B+Tree por titulo: %llu\n",
            static_cast<unsigned long long>(
                blocos_indice_titulo
            )
        );

        std::printf(
            "blocos escritos no arquivo hash: %llu\n",
            static_cast<unsigned long long>(
                hash.stats().writes
            )
        );

        std::printf(
            "blocos lidos no arquivo hash: %llu\n",
            static_cast<unsigned long long>(
                hash.stats().reads
            )
        );

        std::printf(
            "blocos escritos no indice por ID: %llu\n",
            static_cast<unsigned long long>(
                index_id.stats().writes
            )
        );

        std::printf(
            "blocos lidos no indice por ID: %llu\n",
            static_cast<unsigned long long>(
                index_id.stats().reads
            )
        );

        std::printf(
            "blocos escritos no indice por titulo: %llu\n",
            static_cast<unsigned long long>(
                index_title.stats().writes
            )
        );

        std::printf(
            "blocos lidos no indice por titulo: %llu\n",
            static_cast<unsigned long long>(
                index_title.stats().reads
            )
        );

        std::printf(
            "taxa aproximada de ocupacao: %.2f%%\n",
            ocupacao
        );

        std::printf(
            "arquivo de dados: %s\n",
            data_path.c_str()
        );

        std::printf(
            "indice por ID: %s\n",
            index_id_path.c_str()
        );

        std::printf(
            "indice por titulo: %s\n",
            index_title_path.c_str()
        );

        std::printf(
            "========================================\n"
        );

        index_title.close();
        index_id.close();
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