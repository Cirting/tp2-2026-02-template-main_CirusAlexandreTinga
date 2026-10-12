#include "title_bplustree.hpp"

#include <algorithm>
#include <cstring>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace titlebptree {

namespace {

std::string get_key(
    const char key[TITLE_KEY_SIZE]
) {
    return std::string(key);
}

void set_key(
    char destination[TITLE_KEY_SIZE],
    const std::string& value
) {
    std::memset(
        destination,
        0,
        TITLE_KEY_SIZE
    );

    std::strncpy(
        destination,
        value.c_str(),
        TITLE_KEY_SIZE - 1
    );

    destination[TITLE_KEY_SIZE - 1] = '\0';
}

}  // namespace

void TitleBPlusTree::save_header() {
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

void TitleBPlusTree::load_header() {
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

void TitleBPlusTree::create(
    const std::string& path
) {
    file_.open(
        path,
        blockio::Mode::kTruncate
    );

    // Bloco 0 reservado para o cabecalho.
    file_.alloc_block();

    root_ = create_leaf();

    save_header();
}

void TitleBPlusTree::open(
    const std::string& path
) {
    file_.open(
        path,
        blockio::Mode::kRead
    );

    if (file_.block_count() == 0) {
        throw std::runtime_error(
            "arquivo de indice B+Tree por titulo vazio"
        );
    }

    load_header();
}

void TitleBPlusTree::close() {
    file_.close();
}

void TitleBPlusTree::read_node(
    std::uint64_t index,
    TitleNode& node
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
        sizeof(TitleNode)
    );
}

void TitleBPlusTree::write_node(
    std::uint64_t index,
    const TitleNode& node
) {
    std::vector<char> buffer(
        blockio::kBlockSize,
        0
    );

    std::memcpy(
        buffer.data(),
        &node,
        sizeof(TitleNode)
    );

    file_.write_block(
        index,
        buffer.data()
    );
}

std::uint64_t TitleBPlusTree::create_leaf() {
    const std::uint64_t index =
        file_.alloc_block();

    TitleNode node{};

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

std::uint64_t TitleBPlusTree::find_leaf(
    const std::string& title
) {
    if (root_ == INVALID_BLOCK) {
        throw std::runtime_error(
            "B+Tree de titulo nao possui raiz"
        );
    }

    std::uint64_t current = root_;

    while (true) {
        TitleNode node{};

        read_node(
            current,
            node
        );

        if (node.header.is_leaf) {
            return current;
        }

        std::size_t i = 0;

        /*
         * Usamos > e nao >=.
         *
         * Assim, quando existem titulos iguais divididos
         * entre duas folhas, a busca comeca pela folha
         * mais a esquerda possivel e depois segue
         * next_leaf.
         */
        while (
            i < node.header.key_count &&
            title > get_key(node.keys[i])
        ) {
            ++i;
        }

        current =
            node.children[i];
    }
}

void TitleBPlusTree::search(
    const std::string& title,
    std::vector<SearchResult>& results
) {
    results.clear();

    std::uint64_t leaf_index =
        find_leaf(title);

    while (
        leaf_index != INVALID_BLOCK
    ) {
        TitleNode leaf{};

        read_node(
            leaf_index,
            leaf
        );

        for (
            std::size_t i = 0;
            i < leaf.header.key_count;
            ++i
        ) {
            const std::string current =
                get_key(leaf.keys[i]);

            if (current < title) {
                continue;
            }

            if (current > title) {
                return;
            }

            SearchResult result{};

            result.address =
                leaf.addresses[i];

            results.push_back(
                result
            );
        }

        leaf_index =
            leaf.header.next_leaf;
    }
}

void TitleBPlusTree::insert_into_parent(
    std::uint64_t left_index,
    const std::string& separator,
    std::uint64_t right_index
) {
    TitleNode left{};
    TitleNode right{};

    read_node(
        left_index,
        left
    );

    read_node(
        right_index,
        right
    );

    // Caso em que a raiz antiga foi dividida.
    if (
        left.header.parent ==
        INVALID_BLOCK
    ) {
        const std::uint64_t new_root_index =
            file_.alloc_block();

        TitleNode root{};

        root.header.is_leaf = 0;
        root.header.key_count = 1;
        root.header.parent = INVALID_BLOCK;
        root.header.next_leaf = INVALID_BLOCK;

        set_key(
            root.keys[0],
            separator
        );

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

    TitleNode parent{};

    read_node(
        parent_index,
        parent
    );

    std::size_t left_pos = 0;

    while (
        left_pos <= parent.header.key_count &&
        parent.children[left_pos] != left_index
    ) {
        ++left_pos;
    }

    if (
        left_pos >
        parent.header.key_count
    ) {
        throw std::runtime_error(
            "filho esquerdo nao encontrado no pai"
        );
    }

    // Pai ainda possui espaco.
    if (
        parent.header.key_count <
        MAX_KEYS
    ) {
        for (
            std::size_t i =
                parent.header.key_count;
            i > left_pos;
            --i
        ) {
            set_key(
                parent.keys[i],
                get_key(
                    parent.keys[i - 1]
                )
            );

            parent.children[i + 1] =
                parent.children[i];
        }

        set_key(
            parent.keys[left_pos],
            separator
        );

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

    /*
     * Pai cheio.
     * Criamos vetores temporarios com MAX_KEYS + 1.
     */

    const std::size_t total_keys =
        MAX_KEYS + 1;

    std::vector<std::string> temp_keys(
        total_keys
    );

    std::vector<std::uint64_t> temp_children(
        total_keys + 1
    );

    for (
        std::size_t i = 0;
        i < parent.header.key_count;
        ++i
    ) {
        temp_keys[i] =
            get_key(parent.keys[i]);
    }

    for (
        std::size_t i = 0;
        i <= parent.header.key_count;
        ++i
    ) {
        temp_children[i] =
            parent.children[i];
    }

    for (
        std::size_t i =
            total_keys - 1;
        i > left_pos;
        --i
    ) {
        temp_keys[i] =
            temp_keys[i - 1];
    }

    for (
        std::size_t i =
            total_keys;
        i > left_pos + 1;
        --i
    ) {
        temp_children[i] =
            temp_children[i - 1];
    }

    temp_keys[left_pos] =
        separator;

    temp_children[left_pos + 1] =
        right_index;

    const std::size_t middle =
        total_keys / 2;

    const std::string promoted_key =
        temp_keys[middle];

    // Reconstrói metade esquerda no pai antigo.
    parent.header.key_count =
        static_cast<std::uint16_t>(
            middle
        );

    for (
        std::size_t i = 0;
        i < middle;
        ++i
    ) {
        set_key(
            parent.keys[i],
            temp_keys[i]
        );
    }

    for (
        std::size_t i = 0;
        i <= middle;
        ++i
    ) {
        parent.children[i] =
            temp_children[i];
    }

    // Nova metade direita.
    const std::uint64_t new_internal_index =
        file_.alloc_block();

    TitleNode new_internal{};

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
        set_key(
            new_internal.keys[i],
            temp_keys[
                middle + 1 + i
            ]
        );
    }

    for (
        std::size_t i = 0;
        i <= right_key_count;
        ++i
    ) {
        new_internal.children[i] =
            temp_children[
                middle + 1 + i
            ];
    }

    // Atualiza o pai dos filhos movidos.
    for (
        std::size_t i = 0;
        i <= right_key_count;
        ++i
    ) {
        TitleNode child{};

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

void TitleBPlusTree::insert(
    const std::string& title,
    const RecordAddress& address
) {
    const std::uint64_t leaf_index =
        find_leaf(title);

    TitleNode leaf{};

    read_node(
        leaf_index,
        leaf
    );

    /*
     * Diferente do indice por ID:
     * titulo repetido NAO substitui entrada.
     */

    if (
        leaf.header.key_count <
        MAX_KEYS
    ) {
        std::size_t pos =
            leaf.header.key_count;

        /*
         * Inserimos depois das chaves menores ou iguais.
         * Assim titulos iguais permanecem agrupados.
         */
        while (
            pos > 0 &&
            get_key(
                leaf.keys[pos - 1]
            ) > title
        ) {
            set_key(
                leaf.keys[pos],
                get_key(
                    leaf.keys[pos - 1]
                )
            );

            leaf.addresses[pos] =
                leaf.addresses[pos - 1];

            --pos;
        }

        set_key(
            leaf.keys[pos],
            title
        );

        leaf.addresses[pos] =
            address;

        ++leaf.header.key_count;

        write_node(
            leaf_index,
            leaf
        );

        return;
    }

    /*
     * Folha cheia:
     * cria lista temporaria com 11 entradas.
     */

    std::vector<
        std::pair<
            std::string,
            RecordAddress
        >
    > entries;

    entries.reserve(
        MAX_KEYS + 1
    );

    for (
        std::size_t i = 0;
        i < leaf.header.key_count;
        ++i
    ) {
        entries.push_back({
            get_key(
                leaf.keys[i]
            ),
            leaf.addresses[i]
        });
    }

    entries.push_back({
        title,
        address
    });

    std::stable_sort(
        entries.begin(),
        entries.end(),
        [](
            const auto& a,
            const auto& b
        ) {
            return a.first < b.first;
        }
    );

    const std::size_t total =
        entries.size();

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
        set_key(
            leaf.keys[i],
            entries[i].first
        );

        leaf.addresses[i] =
            entries[i].second;
    }

    const std::uint64_t new_leaf_index =
        file_.alloc_block();

    TitleNode new_leaf{};

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
        set_key(
            new_leaf.keys[j],
            entries[i].first
        );

        new_leaf.addresses[j] =
            entries[i].second;
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

    const std::string separator =
        get_key(
            new_leaf.keys[0]
        );

    insert_into_parent(
        leaf_index,
        separator,
        new_leaf_index
    );
}

}  // namespace titlebptree