#pragma once

#include <vector>
#include <cstdint>
#include <non_std/PoolAlocator.hpp>
#include "Traits.hpp"

namespace non_std::containers
{

template<typename TValue, typename TKey, unsigned char THashWidth>
class HashMap
{
public:
    constexpr static NeverOverwriteTag OverwriteCategory {};
    using pointer = TValue*;
private:
    struct Node
    {
        TKey key;
        TValue val;
        Node *next;

        Node(const TKey &keyIn, const TValue &valIn, Node *nextIn) noexcept
                : key(keyIn), val(valIn), next(nextIn)
        {}

        Node(const TKey &keyIn, TValue &&valIn, Node *nextIn) noexcept
                : key(keyIn), val(std::move(valIn)), next(nextIn)
        {}

        ~Node() noexcept
        {
            delete next;
        }
    };

public:
    HashMap()
        : table(1 << THashWidth, nullptr)
    {
    }

    HashMap(const HashMap &) = delete;
    HashMap &operator=(const HashMap &in) = delete;

    HashMap &operator=(HashMap &&in)
    {
        clear();
        this->table = in.table;
        in.table = {};
        return *this;
    }

    HashMap(HashMap &&in)
    {
        this->table = in.table;
        in.table = {};
    }

    void clear() noexcept
    {
        for (auto *&elem: table) {
            if (elem != nullptr) {
                elem = nullptr;
            }
        }
        allocator_.clearAll();
    }

    ~HashMap()
    {
        // No need to clear POD types. Allocator does it for us.
    }

    TValue* get(const TKey key) noexcept
    {
        auto index = key & hashMask;
        Node *node = table[index];
        while (node != nullptr) {
            if (node->key == key) {
                return &(node->val);
            }
            node = node->next;
        }
        return nullptr;
    }

    TValue* operator[](const TKey key)
    {
        auto index = key & hashMask;
        Node *node = table[index];
        while (node != nullptr) {
            if (node->key == key) {
                return &(node->val);
            }
            node = node->next;
        }
        node = allocator_.allocate();
        std::construct_at(node, key, TValue{}, table[index]);
        table[index] = node;
        return &(table[index]->val);
    }

    TValue *store(const TKey key, const TValue val)
    {
        auto index = key & hashMask;
        auto *node = allocator_.allocate();
        std::construct_at(node, key, std::move(val), table[index]);
        table[index] = node;
        return &(table[index]->val);
    }

private:
    PoolAllocator <Node> allocator_;
    std::vector<Node *> table;
    static constexpr uint64_t hashMask = (1 << THashWidth) - 1;
};

}  // namespace non_std::containers
