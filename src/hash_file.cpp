#include "hash_file.hpp"

#include <cstdio>
#include <cstring>
#include <vector>

namespace hashfile {

void HashFile::create(
    const std::string& path
) {
    file_.open(
        path,
        blockio::Mode::kTruncate
    );

    initialize_buckets();
}

void HashFile::open(
    const std::string& path
) {
    file_.open(
        path,
        blockio::Mode::kRead
    );
}

void HashFile::close() {
    file_.close();
}

void HashFile::initialize_buckets() {
    std::vector<char> buffer(
        blockio::kBlockSize,
        0
    );

    for (
        std::uint64_t i = 0;
        i < NUM_BUCKETS;
        ++i
    ) {
        HashBlock block{};

        block.header.next_block =
            INVALID_BLOCK;

        block.header.record_count = 0;
        block.header.reserved = 0;

        std::memcpy(
            buffer.data(),
            &block,
            sizeof(HashBlock)
        );

        file_.write_block(
            i,
            buffer.data()
        );
    }
}

void HashFile::read_hash_block(
    std::uint64_t index,
    HashBlock& block
) {
    std::vector<char> buffer(
        blockio::kBlockSize,
        0
    );

    file_.read_block(
        index,
        buffer.data()
    );

    std::memcpy(
        &block,
        buffer.data(),
        sizeof(HashBlock)
    );
}

void HashFile::write_hash_block(
    std::uint64_t index,
    const HashBlock& block
) {
    std::vector<char> buffer(
        blockio::kBlockSize,
        0
    );

    std::memcpy(
        buffer.data(),
        &block,
        sizeof(HashBlock)
    );

    file_.write_block(
        index,
        buffer.data()
    );
}

RecordAddress HashFile::insert(
    const DiskArticle& article
) {
    std::uint64_t current =
        hash_id(article.id);

    while (true) {
        HashBlock block{};

        read_hash_block(
            current,
            block
        );

        // Ainda existe espaco neste bloco.
        if (
            block.header.record_count <
            RECORDS_PER_BLOCK
        ) {
            const std::uint32_t slot =
                block.header.record_count;

            block.records[slot] =
                article;

            ++block.header.record_count;

            write_hash_block(
                current,
                block
            );

            return RecordAddress{
                current,
                slot
            };
        }

        // Existe bloco de overflow.
        if (
            block.header.next_block !=
            INVALID_BLOCK
        ) {
            current =
                block.header.next_block;

            continue;
        }

        // Bloco cheio e sem overflow:
        // cria novo bloco.
        const std::uint64_t new_block_index =
            file_.alloc_block();

        HashBlock new_block{};

        new_block.header.next_block =
            INVALID_BLOCK;

        new_block.header.record_count = 1;
        new_block.header.reserved = 0;

        new_block.records[0] =
            article;

        write_hash_block(
            new_block_index,
            new_block
        );

        // Liga o bloco anterior ao overflow.
        block.header.next_block =
            new_block_index;

        write_hash_block(
            current,
            block
        );

        return RecordAddress{
            new_block_index,
            0
        };
    }
}

bool HashFile::find_by_id(
    int id,
    DiskArticle& result
) {
    std::uint64_t current =
        hash_id(id);

    while (
        current != INVALID_BLOCK
    ) {
        HashBlock block{};

        read_hash_block(
            current,
            block
        );

        for (
            std::uint32_t i = 0;
            i < block.header.record_count;
            ++i
        ) {
            if (
                block.records[i].id ==
                id
            ) {
                result =
                    block.records[i];

                return true;
            }
        }

        current =
            block.header.next_block;
    }

    return false;
}

bool HashFile::find_by_title(
    const std::string& titulo,
    DiskArticle& result
) {
    const std::uint64_t total_blocks =
        file_.block_count();

    for (
        std::uint64_t i = 0;
        i < total_blocks;
        ++i
    ) {
        HashBlock block{};

        read_hash_block(
            i,
            block
        );

        for (
            std::uint32_t j = 0;
            j < block.header.record_count;
            ++j
        ) {
            Article artigo =
                from_disk_article(
                    block.records[j]
                );

            if (
                artigo.titulo ==
                titulo
            ) {
                result =
                    block.records[j];

                return true;
            }
        }
    }

    return false;
}

std::uint64_t HashFile::find_range(
    int inicio,
    int fim
) {
    const std::uint64_t total_blocks =
        file_.block_count();

    std::uint64_t encontrados = 0;

    for (
        std::uint64_t i = 0;
        i < total_blocks;
        ++i
    ) {
        HashBlock block{};

        read_hash_block(
            i,
            block
        );

        for (
            std::uint32_t j = 0;
            j < block.header.record_count;
            ++j
        ) {
            const DiskArticle& disco =
                block.records[j];

            if (
                disco.id >= inicio &&
                disco.id <= fim
            ) {
                Article artigo =
                    from_disk_article(
                        disco
                    );

                std::printf(
                    "id: %d | titulo: %s\n",
                    artigo.id,
                    artigo.titulo.c_str()
                );

                ++encontrados;
            }
        }
    }

    return encontrados;
}


bool HashFile::read_at(
    const RecordAddress& address,
    DiskArticle& result
) {
    if (address.block >= file_.block_count()) {
        return false;
    }

    HashBlock block{};

    read_hash_block(
        address.block,
        block
    );

    if (address.slot >= block.header.record_count) {
        return false;
    }

    result = block.records[address.slot];

    return true;
}

std::uint64_t HashFile::find_all_by_title(
    const std::string& titulo,
    std::vector<DiskArticle>& results
) {
    results.clear();

    const std::uint64_t total_blocks =
        file_.block_count();

    for (
        std::uint64_t i = 0;
        i < total_blocks;
        ++i
    ) {
        HashBlock block{};

        read_hash_block(
            i,
            block
        );

        for (
            std::uint32_t j = 0;
            j < block.header.record_count;
            ++j
        ) {
            Article artigo =
                from_disk_article(
                    block.records[j]
                );

            if (artigo.titulo == titulo) {
                results.push_back(
                    block.records[j]
                );
            }
        }
    }

    return results.size();
}

std::uint64_t HashFile::block_count() const {
    return file_.block_count();
}

}  // namespace hashfile