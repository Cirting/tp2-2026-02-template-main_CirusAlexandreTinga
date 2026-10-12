#ifndef HASH_FILE_HPP
#define HASH_FILE_HPP
#include <vector>

#include <cstddef>
#include <cstdint>
#include <limits>
#include <string>

#include "blockio.hpp"
#include "disk_record.hpp"

namespace hashfile {

constexpr std::uint64_t INVALID_BLOCK =
    std::numeric_limits<std::uint64_t>::max();

struct HashBlockHeader {
    std::uint64_t next_block;
    std::uint32_t record_count;
    std::uint32_t reserved;
};

constexpr std::size_t HEADER_SIZE =
    sizeof(HashBlockHeader);

constexpr std::size_t RECORDS_PER_BLOCK =
    (blockio::kBlockSize - HEADER_SIZE) /
    sizeof(DiskArticle);

struct HashBlock {
    HashBlockHeader header;
    DiskArticle records[RECORDS_PER_BLOCK];
};

static_assert(
    sizeof(HashBlock) <= blockio::kBlockSize,
    "HashBlock nao cabe dentro de um bloco."
);

constexpr std::uint64_t NUM_BUCKETS = 50000;

inline std::uint64_t hash_id(int id) {
    return static_cast<std::uint64_t>(id) %
           NUM_BUCKETS;
}

// Endereco fisico de um registro dentro de artigos.dat
struct RecordAddress {
    std::uint64_t block;
    std::uint32_t slot;
};
class HashFile {
 
     public:
    HashFile() = default;

    void create(const std::string& path);

    void open(const std::string& path);

    void close();

    RecordAddress insert(const DiskArticle& article);

    bool find_by_id(
        int id,
        DiskArticle& result
    );

    bool find_by_title(
        const std::string& titulo,
        DiskArticle& result
    );

    std::uint64_t find_all_by_title(
        const std::string& titulo,
        std::vector<DiskArticle>& results
    );

    bool read_at(
        const RecordAddress& address,
        DiskArticle& result
    );

    std::uint64_t find_range(
        int inicio,
        int fim
    );

    std::uint64_t block_count() const;

    const blockio::Stats& stats() const {
        return file_.stats();
    }

    void reset_stats() {
        file_.reset_stats();
    }
    

  private:
    blockio::BlockFile file_;

    void initialize_buckets();

    void read_hash_block(
        std::uint64_t index,
        HashBlock& block
    );

    void write_hash_block(
        std::uint64_t index,
        const HashBlock& block
    );

    
};

}  // namespace hashfile

#endif