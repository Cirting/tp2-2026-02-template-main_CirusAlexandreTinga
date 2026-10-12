#include "bplustree.hpp"

#include <cstring>
#include <stdexcept>
#include <vector>

namespace bptree {

void BPlusTree::save_header() {
    std::vector<char> buffer(
        blockio::kBlockSize,
        0
    );

    TreeHeader header{};
    header.root = root_;

    std::memcpy(
        buffer.data(),
        &header,
        sizeof(TreeHeader)
    );

    file_.write_block(
        0,
        buffer.data()
    );
}

void BPlusTree::load_header() {
    std::vector<char> buffer(
        blockio::kBlockSize,
        0
    );

    file_.read_block(
        0,
        buffer.data()
    );

    TreeHeader header{};

    std::memcpy(
        &header,
        buffer.data(),
        sizeof(TreeHeader)
    );

    root_ = header.root;
}

void BPlusTree::create(
    const std::string& path
) {
    file_.open(
        path,
        blockio::Mode::kTruncate
    );

    file_.alloc_block();

    root_ = create_leaf();

    save_header();
}

void BPlusTree::open(
    const std::string& path
) {
    file_.open(
        path,
        blockio::Mode::kRead
    );

    if (file_.block_count() == 0) {
        throw std::runtime_error(
            "arquivo de indice B+Tree vazio"
        );
    }

    load_header();
}

void BPlusTree::close() {
    file_.close();
}

void BPlusTree::read_node(
    std::uint64_t index,
    BPlusNode& node
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
        &node,
        buffer.data(),
        sizeof(BPlusNode)
    );
}

void BPlusTree::write_node(
    std::uint64_t index,
    const BPlusNode& node
) {
    std::vector<char> buffer(
        blockio::kBlockSize,
        0
    );

    std::memcpy(
        buffer.data(),
        &node,
        sizeof(BPlusNode)
    );

    file_.write_block(
        index,
        buffer.data()
    );
}

std::uint64_t BPlusTree::create_leaf() {
    const std::uint64_t index =
        file_.alloc_block();

    BPlusNode node{};

    node.header.is_leaf = 1;
    node.header.key_count = 0;
    node.header.parent = INVALID_BLOCK;
    node.header.next_leaf = INVALID_BLOCK;

    write_node(
        index,
        node
    );

    return index;
}

std::uint64_t BPlusTree::find_leaf(
    int key
) {
    if (root_ == INVALID_BLOCK) {
        throw std::runtime_error(
            "B+Tree nao possui raiz"
        );
    }

    std::uint64_t current = root_;

    while (true) {
        BPlusNode node{};

        read_node(
            current,
            node
        );

        if (node.header.is_leaf) {
            return current;
        }

        std::size_t i = 0;

        while (
            i < node.header.key_count &&
            key >= node.keys[i]
        ) {
            ++i;
        }

        current = node.children[i];
    }
}

bool BPlusTree::search(
    int key,
    RecordAddress& result
) {
    const std::uint64_t leaf_index =
        find_leaf(key);

    BPlusNode leaf{};

    read_node(
        leaf_index,
        leaf
    );

    for (
        std::size_t i = 0;
        i < leaf.header.key_count;
        ++i
    ) {
        if (leaf.keys[i] == key) {
            result =
                leaf.addresses[i];

            return true;
        }
    }

    return false;
}

void BPlusTree::insert_into_parent(
    std::uint64_t left_index,
    int separator_key,
    std::uint64_t right_index
) {
    BPlusNode left{};
    BPlusNode right{};

    read_node(
        left_index,
        left
    );

    read_node(
        right_index,
        right
    );

    // Caso especial: left era a raiz.
    if (left.header.parent == INVALID_BLOCK) {
        const std::uint64_t new_root_index =
            file_.alloc_block();

        BPlusNode root{};

        root.header.is_leaf = 0;
        root.header.key_count = 1;
        root.header.parent = INVALID_BLOCK;
        root.header.next_leaf = INVALID_BLOCK;

        root.keys[0] =
            separator_key;

        root.children[0] =
            left_index;

        root.children[1] =
            right_index;

        left.header.parent =
            new_root_index;

        right.header.parent =
            new_root_index;

        write_node(
            left_index,
            left
        );

        write_node(
            right_index,
            right
        );

        write_node(
            new_root_index,
            root
        );

        root_ =
            new_root_index;

        save_header();

        return;
    }

    const std::uint64_t parent_index =
        left.header.parent;

    BPlusNode parent{};

    read_node(
        parent_index,
        parent
    );

    // Descobre onde esta o filho esquerdo.
    std::size_t left_pos = 0;

    while (
        left_pos <= parent.header.key_count &&
        parent.children[left_pos] != left_index
    ) {
        ++left_pos;
    }

    if (left_pos > parent.header.key_count) {
        throw std::runtime_error(
            "filho esquerdo nao encontrado no pai"
        );
    }

    // Se o pai ainda tem espaco.
    if (parent.header.key_count < MAX_KEYS) {

        for (
            std::size_t i = parent.header.key_count;
            i > left_pos;
            --i
        ) {
            parent.keys[i] =
                parent.keys[i - 1];

            parent.children[i + 1] =
                parent.children[i];
        }

        parent.keys[left_pos] =
            separator_key;

        parent.children[left_pos + 1] =
            right_index;

        ++parent.header.key_count;

        right.header.parent =
            parent_index;

        write_node(
            right_index,
            right
        );

        write_node(
            parent_index,
            parent
        );

        return;
    }

    // Pai cheio: cria estrutura temporaria.
    const std::size_t total_keys =
        MAX_KEYS + 1;

    std::vector<int> temp_keys(
        total_keys
    );

    std::vector<std::uint64_t> temp_children(
        total_keys + 1
    );

    for (
        std::size_t i = 0;
        i <= parent.header.key_count;
        ++i
    ) {
        temp_children[i] =
            parent.children[i];
    }

    for (
        std::size_t i = 0;
        i < parent.header.key_count;
        ++i
    ) {
        temp_keys[i] =
            parent.keys[i];
    }

    // Move para abrir espaco.
    for (
        std::size_t i = total_keys - 1;
        i > left_pos;
        --i
    ) {
        temp_keys[i] =
            temp_keys[i - 1];
    }

    for (
        std::size_t i = total_keys;
        i > left_pos + 1;
        --i
    ) {
        temp_children[i] =
            temp_children[i - 1];
    }

    temp_keys[left_pos] =
        separator_key;

    temp_children[left_pos + 1] =
        right_index;

    const std::size_t middle =
        total_keys / 2;

    const int promoted_key =
        temp_keys[middle];

    // Reconstrói o pai antigo como metade esquerda.
    parent.header.key_count =
        static_cast<std::uint16_t>(
            middle
        );

    for (
        std::size_t i = 0;
        i < middle;
        ++i
    ) {
        parent.keys[i] =
            temp_keys[i];
    }

    for (
        std::size_t i = 0;
        i <= middle;
        ++i
    ) {
        parent.children[i] =
            temp_children[i];
    }

    // Cria novo no interno.
    const std::uint64_t new_internal_index =
        file_.alloc_block();

    BPlusNode new_internal{};

    new_internal.header.is_leaf = 0;
    new_internal.header.parent =
        parent.header.parent;
    new_internal.header.next_leaf =
        INVALID_BLOCK;

    const std::size_t right_key_count =
        total_keys - middle - 1;

    new_internal.header.key_count =
        static_cast<std::uint16_t>(
            right_key_count
        );

    for (
        std::size_t i = 0;
        i < right_key_count;
        ++i
    ) {
        new_internal.keys[i] =
            temp_keys[middle + 1 + i];
    }

    for (
        std::size_t i = 0;
        i <= right_key_count;
        ++i
    ) {
        new_internal.children[i] =
            temp_children[middle + 1 + i];
    }

    // Atualiza o parent dos filhos do novo no.
    for (
        std::size_t i = 0;
        i <= right_key_count;
        ++i
    ) {
        BPlusNode child{};

        read_node(
            new_internal.children[i],
            child
        );

        child.header.parent =
            new_internal_index;

        write_node(
            new_internal.children[i],
            child
        );
    }

    write_node(
        parent_index,
        parent
    );

    write_node(
        new_internal_index,
        new_internal
    );

    insert_into_parent(
        parent_index,
        promoted_key,
        new_internal_index
    );
}

void BPlusTree::split_internal(
    std::uint64_t,
    BPlusNode&
) {
    // O split interno ja e tratado
    // diretamente em insert_into_parent().
}

void BPlusTree::insert(
    int key,
    const RecordAddress& address
) {
    const std::uint64_t leaf_index =
        find_leaf(key);

    BPlusNode leaf{};

    read_node(
        leaf_index,
        leaf
    );

    // Atualiza caso a chave ja exista.
    for (
        std::size_t i = 0;
        i < leaf.header.key_count;
        ++i
    ) {
        if (leaf.keys[i] == key) {
            leaf.addresses[i] =
                address;

            write_node(
                leaf_index,
                leaf
            );

            return;
        }
    }

    // Folha ainda tem espaco.
    if (leaf.header.key_count < MAX_KEYS) {
        std::size_t pos =
            leaf.header.key_count;

        while (
            pos > 0 &&
            leaf.keys[pos - 1] > key
        ) {
            leaf.keys[pos] =
                leaf.keys[pos - 1];

            leaf.addresses[pos] =
                leaf.addresses[pos - 1];

            --pos;
        }

        leaf.keys[pos] =
            key;

        leaf.addresses[pos] =
            address;

        ++leaf.header.key_count;

        write_node(
            leaf_index,
            leaf
        );

        return;
    }

    // Folha cheia.
    const std::size_t total =
        MAX_KEYS + 1;

    std::vector<int> temp_keys(
        total
    );

    std::vector<RecordAddress> temp_addresses(
        total
    );

    std::size_t pos = 0;
    bool inserted = false;

    for (
        std::size_t i = 0;
        i < leaf.header.key_count;
        ++i
    ) {
        if (
            !inserted &&
            key < leaf.keys[i]
        ) {
            temp_keys[pos] =
                key;

            temp_addresses[pos] =
                address;

            ++pos;
            inserted = true;
        }

        temp_keys[pos] =
            leaf.keys[i];

        temp_addresses[pos] =
            leaf.addresses[i];

        ++pos;
    }

    if (!inserted) {
        temp_keys[pos] =
            key;

        temp_addresses[pos] =
            address;
    }

    const std::size_t split =
        total / 2;

    leaf.header.key_count =
        static_cast<std::uint16_t>(
            split
        );

    for (
        std::size_t i = 0;
        i < split;
        ++i
    ) {
        leaf.keys[i] =
            temp_keys[i];

        leaf.addresses[i] =
            temp_addresses[i];
    }

    const std::uint64_t new_leaf_index =
        file_.alloc_block();

    BPlusNode new_leaf{};

    new_leaf.header.is_leaf = 1;
    new_leaf.header.parent =
        leaf.header.parent;

    new_leaf.header.next_leaf =
        leaf.header.next_leaf;

    new_leaf.header.key_count =
        static_cast<std::uint16_t>(
            total - split
        );

    for (
        std::size_t i = split, j = 0;
        i < total;
        ++i, ++j
    ) {
        new_leaf.keys[j] =
            temp_keys[i];

        new_leaf.addresses[j] =
            temp_addresses[i];
    }

    leaf.header.next_leaf =
        new_leaf_index;

    write_node(
        leaf_index,
        leaf
    );

    write_node(
        new_leaf_index,
        new_leaf
    );

    const int separator_key =
        new_leaf.keys[0];

    insert_into_parent(
        leaf_index,
        separator_key,
        new_leaf_index
    );
}

void BPlusTree::range_search(
    int inicio,
    int fim,
    std::vector<RangeEntry>& results
) {
    results.clear();

    if (inicio > fim) {
        return;
    }

    std::uint64_t leaf_index =
        find_leaf(inicio);

    while (leaf_index != INVALID_BLOCK) {
        BPlusNode leaf{};

        read_node(
            leaf_index,
            leaf
        );

        for (
            std::size_t i = 0;
            i < leaf.header.key_count;
            ++i
        ) {
            const int key =
                leaf.keys[i];

            if (key < inicio) {
                continue;
            }

            if (key > fim) {
                return;
            }

            RangeEntry entry{};

            entry.key = key;
            entry.address =
                leaf.addresses[i];

            results.push_back(entry);
        }

        leaf_index =
            leaf.header.next_leaf;
    }
}

}  // namespace bptree