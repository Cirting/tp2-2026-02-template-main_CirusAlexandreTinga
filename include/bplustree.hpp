#ifndef BPLUSTREE_HPP
#define BPLUSTREE_HPP

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

#include "blockio.hpp"

namespace bptree {

constexpr std::uint64_t INVALID_BLOCK =
    static_cast<std::uint64_t>(-1);

struct RecordAddress {
    std::uint64_t block;
    std::uint32_t slot;
};

struct RangeEntry {
    int key;
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

constexpr std::size_t NODE_OVERHEAD = 64;

 constexpr std::size_t MAX_KEYS = (blockio::kBlockSize - NODE_OVERHEAD) /(sizeof(int) + sizeof(RecordAddress) + sizeof(std::uint64_t));

struct BPlusNode {
    NodeHeader header;

    int keys[MAX_KEYS];

    RecordAddress addresses[MAX_KEYS];

    std::uint64_t children[MAX_KEYS + 1];
};

static_assert(
    sizeof(BPlusNode) <= blockio::kBlockSize,
    "BPlusNode nao cabe dentro de um bloco."
);

class BPlusTree {
  public:
    BPlusTree() = default;

    void create(const std::string& path);
    void open(const std::string& path);
    void close();

    void insert(
        int key,
        const RecordAddress& address
    );

    bool search(
        int key,
        RecordAddress& result
    );

    void range_search( int inicio, int fim, std::vector<RangeEntry>& results );

    std::uint64_t root() const {return root_;}

    std::uint64_t block_count() const {return file_.block_count();}

    const blockio::Stats& stats() const {return file_.stats();}

    void reset_stats() {file_.reset_stats();}

  private:
    blockio::BlockFile file_;
    std::uint64_t root_ = INVALID_BLOCK;

    void save_header();
    void load_header();

    void read_node(
        std::uint64_t index,
        BPlusNode& node
    );

    void write_node(
        std::uint64_t index,
        const BPlusNode& node
    );

    std::uint64_t create_leaf();

    std::uint64_t find_leaf(int key);

    void insert_into_parent(
        std::uint64_t left_index,
        int separator_key,
        std::uint64_t right_index
    );

    void split_internal(
        std::uint64_t node_index,
        BPlusNode& node
    );
};

}  // namespace bptree

#endif