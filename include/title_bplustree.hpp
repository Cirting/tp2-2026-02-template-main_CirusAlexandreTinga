#ifndef TITLE_BPLUSTREE_HPP
#define TITLE_BPLUSTREE_HPP

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

#include "blockio.hpp"

namespace titlebptree {

constexpr std::uint64_t INVALID_BLOCK =
    static_cast<std::uint64_t>(-1);

constexpr std::size_t TITLE_KEY_SIZE = 301;



struct RecordAddress {
    std::uint64_t block;
    std::uint32_t slot;
};

constexpr std::size_t NODE_OVERHEAD = 64;

constexpr std::size_t MAX_KEYS =
    (blockio::kBlockSize - NODE_OVERHEAD) /
    (
        TITLE_KEY_SIZE +
        sizeof(RecordAddress) +
        sizeof(std::uint64_t)
    );

struct SearchResult {
    RecordAddress address;
};

struct TreeHeader {
    std::uint64_t root;
};

struct NodeHeader {
    std::uint8_t is_leaf;
    std::uint16_t key_count;
    std::uint64_t parent;
    std::uint64_t next_leaf;
};

struct TitleNode {
    NodeHeader header;

    char keys[MAX_KEYS][TITLE_KEY_SIZE];

    // Usado pelas folhas.
    RecordAddress addresses[MAX_KEYS];

    // Usado pelos nós internos.
    std::uint64_t children[MAX_KEYS + 1];
};

static_assert(
    sizeof(TitleNode) <= blockio::kBlockSize,
    "TitleNode nao cabe dentro de um bloco."
);

class TitleBPlusTree {
  public:
    TitleBPlusTree() = default;

    void create(const std::string& path);

    void open(const std::string& path);

    void close();

    void insert(
        const std::string& title,
        const RecordAddress& address
    );

    void search(
        const std::string& title,
        std::vector<SearchResult>& results
    );

    std::uint64_t block_count() const {
        return file_.block_count();
    }

    const blockio::Stats& stats() const {
        return file_.stats();
    }

    void reset_stats() {
        file_.reset_stats();
    }

  private:
    blockio::BlockFile file_;
    std::uint64_t root_ = INVALID_BLOCK;

    void save_header();

    void load_header();

    void read_node(
        std::uint64_t index,
        TitleNode& node
    );

    void write_node(
        std::uint64_t index,
        const TitleNode& node
    );

    std::uint64_t create_leaf();

    std::uint64_t find_leaf(
        const std::string& title
    );

    void insert_into_parent(
        std::uint64_t left_index,
        const std::string& separator,
        std::uint64_t right_index
    );
};

}  // namespace titlebptree

#endif // TITLE_BPLUSTREE_HPP