#pragma once

#include <vector>
#include <utility>
#include <optional>

namespace non_std::containers
{

template<typename TKey,
        typename TValue,
        typename Hash = std::hash<TKey>>
class FixedSizeHashTableOpenHashingWithAge
{
    struct Node
    {
        enum class Occupancy
        {
            free,
            deleted,
            occupied,
        } occupancy_ = Occupancy::free;
        unsigned long long age_ = 0u;
        TKey key_;
        TValue value_;
    };
public:
    HashTableOpenHashing(const HashTableWithRehashing &) = delete;
    HashTableOpenHashing&operator=(const HashTableOpenHashing&in) = delete;

    HashTableOpenHashing()
        :data_(1 << TBucketBitWidth)
    {
    }

    TValue* get(const TKey& key) const noexcept
    {
        auto hash = hashFunction_(key);
        auto bucket = hash & hashMask;

        for (int i = 0; i < 3; ++i)
        {
            if (nodes_[bucket].occupancy_ == Node::Occupancy::occupied && nodes_[bucket].key_ == key)
            {
                nodes_[bucket].age_ = ++age_;
                return &nodes_[bucket].value;
            }
            if (nodes_[bucket].occupancy_ == Node::Occupancy::free)
            {
                return nullptr;
            }
            ++bucket;
        }
        return nullptr;
    }

    void store(const TKey& key, const TValue& val)
    {
        auto hash = hashFunction_(key);
        auto bucket = hash & hashMask;

        auto oldestElem = nullptr;
        auto oldestAge = 0u;

        for (int i = 0; i < 3; ++i)
        {
            if (nodes_[bucket].occupancy_ == Node::Occupancy::occupied && nodes_[bucket].key_ == key)
            {
                nodes_[bucket].value = val;
                nodes_[bucket].age_ = ++age_;
                return;
            }
            if (nodes_[bucket].occupancy_ == Node::Occupancy::free)
            {
                create(nodes_[bucket], key, value);
                return;
            }
            if (nodes_[bucket].occupancy_ == Node::Occupancy::deleted)
            {
                oldestElem = &nodes_[bucket];
                oldestAge = age_;
                ++bucket;
                continue;
            }
            if (nodes_[bucket].age_ > oldestAge)
            {
                oldestAge = nodes_[bucket].age_;
                oldestElem = &nodes_[bucket];
            }
            ++bucket;
        }
        create(*oldestElem, key, value);
    }

    TValue* operator[](const TKey& key)
    {
        auto hash = hashFunction_(key);
        auto bucket = hash & hashMask;

        auto oldestElem = nullptr;
        auto oldestAge = 0u;

        for (int i = 0; i < 3; ++i)
        {
            if (nodes_[bucket].occupancy_ == Node::Occupancy::occupied && nodes_[bucket].key_ == key)
            {
                return &nodes_[bucket].value;
            }
            if (nodes_[bucket].occupancy_ == Node::Occupancy::free)
            {
                create(nodes_[bucket], key, value);
                return &nodes_[bucket].value;
            }
            if (nodes_[bucket].occupancy_ == Node::Occupancy::deleted)
            {
                oldestElem = &nodes_[bucket];
                oldestAge = age_;
                ++bucket;
                continue;
            }
            if (nodes_[bucket].age_ > oldestAge)
            {
                oldestAge = nodes_[bucket].age_;
                oldestElem = &nodes_[bucket];
            }
            ++bucket;
        }
        create(*oldestElem, key, value);
        return oldestElem;
    }
private:
    void create(Node& node, const TKey& key, const TValue& val)
    {
        node.key = key;
        node.value = val;
        node.age_ = ++age_;
        node.occupancy_ = Node::Occupancy::occupied;
    }

    std::decay_t<decltype(Hash())> hashFunction_;
    std::array<TValue, (20u << 1u) > nodes_;
    unsigned long long age_ = 0u;
    unsigned long long hashMask = (20u << 1u) - 1;
};

}  // namespace non_std::containers
