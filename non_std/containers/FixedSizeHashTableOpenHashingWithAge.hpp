#pragma once

#include <array>
#include <vector>
#include <utility>
#include <optional>
#include <stdint.h>

#include "Traits.hpp"

#include <non_std/internal/Logger.hpp>
#include <unordered_map>

namespace non_std::containers
{

template<typename TKey,
        typename TValue,
        typename Hash = std::hash<TKey>>
class FixedSizeHashTableOpenHashingWithAge
{
struct Node;
    enum class Occupancy
    {
        free = 0,
        deleted,
        occupied,
        locked,
    };

public:
    /* This is problematic */
    /* It may crash operator[] in case all slots visited there are locked */
    /* Possibly detached node is better idea. In such scenario, pointer maybe out of collection. */
    struct PersistPointer
    {
        explicit PersistPointer(Node* in)
        {
            node_ = in;
            if (node_ != nullptr)
                in->occupancy_ = Occupancy::locked;
        }
        ~PersistPointer ()
        {
            if (node_ != nullptr) node_->occupancy_ = Occupancy::occupied;
        }
        TValue* operator->()
        {
            return &node_->value_;
        }
        operator TValue*()
        {
            if (node_ != nullptr)
                return &node_->value_;
            return nullptr;
        }

        Node* node_;
    };

/* Traits section */
public:
    constexpr static OverwriteOlderElements_Taq OverwriteCategory {};
    constexpr static unsigned int HashTries = 5;
    using pointer = PersistPointer;
/* Internal types section */
private:
    struct Node
    {
        Occupancy occupancy_ = Occupancy::free;
        uint64_t age_ = 0u;
        TKey key_ = {};
        TValue value_ = {};
    };

public:
    FixedSizeHashTableOpenHashingWithAge()
        : nodes_(std::vector<Node>((1u  << 21u), Node{}))
    {
    }

    FixedSizeHashTableOpenHashingWithAge(const FixedSizeHashTableOpenHashingWithAge & other)
    {
        nodes_ = other.nodes_;
        age_ = other.age_;
    }

    FixedSizeHashTableOpenHashingWithAge& operator=(const FixedSizeHashTableOpenHashingWithAge& other)
    {
        if (this == &other)
        {
            return *this;
        }
        nodes_ = other.nodes_;
        age_ = other.age_;
        return *this;
    }

    pointer get(const TKey& key) noexcept
    {
        LOG ("get: " << key << std::endl);
        auto hash = hashFunction_(key);
        auto bucket = hash & hashMask;

        for (std::decay<decltype(HashTries)>::type i = 0; i < HashTries; ++i)
        {
            if ((nodes_[bucket].occupancy_ == Occupancy::occupied
                    || nodes_[bucket].occupancy_ == Occupancy::locked)
                && nodes_[bucket].key_ == key)
            {
                nodes_[bucket].age_ = ++age_;
                LOG ("get: key: " << key << " exist in exist in bucket: " << bucket << std::endl);
                return PersistPointer(&nodes_[bucket]);
            }
            if (nodes_[bucket].occupancy_ == Occupancy::free)
            {
                LOG("get: key: " << key << " not exist because bucket: " << bucket << " is empty" << std::endl);
                return PersistPointer(nullptr);
            }
            bucket = getBucket(bucket);
        }
        LOG("get: key: " << key << "Not exist - to many hash retries" << std::endl);
        return PersistPointer(nullptr);;
    }

    auto getBucket(uint64_t in)
    {
        return (1 + in) & hashMask;
    }

    void store(const TKey& key, const TValue& value)
    {
        auto hash = hashFunction_(key);
        auto bucket = hash & hashMask;

        Node* oldestElem = nullptr;
        uint64_t oldestAge = 0u;

        for (std::decay<decltype(HashTries)>::type  i = 0; i < HashTries; ++i)
        {
            if (nodes_[bucket].occupancy_ == Occupancy::occupied && nodes_[bucket].key_ == key)
            {
                nodes_[bucket].value = value;
                nodes_[bucket].age_ = ++age_;
                return;
            }
            if (nodes_[bucket].occupancy_ == Occupancy::free)
            {
                create(nodes_[bucket], key, value);
                return;
            }
            if (nodes_[bucket].occupancy_ == Occupancy::deleted)
            {
                oldestElem = &nodes_[bucket];
                oldestAge = age_;
                bucket = getBucket(bucket);
                continue;
            }
            if (nodes_[bucket].age_ > oldestAge)
            {
                oldestAge = nodes_[bucket].age_;
                oldestElem = &nodes_[bucket];
            }
            bucket = getBucket(bucket);
        }
        create(*oldestElem, key, value);
    }

    pointer operator[](const TKey& key)
    {
        LOG ("operator[]: " << key << std::endl);
        auto hash = hashFunction_(key);
        auto bucket = hash & hashMask;

        Node* oldestElem = nullptr;
        uint64_t oldestAge = 0ull - 1;

        for (std::decay<decltype(HashTries)>::type i = 0; i < HashTries; ++i)
        {
            if ((nodes_[bucket].occupancy_ == Occupancy::occupied
                 || nodes_[bucket].occupancy_ == Occupancy::locked) && nodes_[bucket].key_ == key)
            {
                LOG ("operator[]: key: " << key << " exist in exist in bucket: " << bucket << std::endl);
                return PersistPointer(&nodes_[bucket]);
            }
            if (nodes_[bucket].occupancy_ == Occupancy::free)
            {
                LOG ("operator[]: key: " << key << " will be put into bucket as this is free: " << bucket << std::endl);
                create(nodes_[bucket], key, TValue{});
                return PersistPointer(&nodes_[bucket]);
            }
            if (nodes_[bucket].occupancy_ == Occupancy::locked)
            {
                bucket = getBucket(bucket);
                continue;
            }
            if (nodes_[bucket].occupancy_ == Occupancy::deleted)
            {
                LOG ("operator[]: key: " << key << " conditionally may be put into bucket: " << bucket << " as this is free. " << std::endl);
                oldestElem = &nodes_[bucket];
                oldestAge = 0;
                bucket = getBucket(bucket);
                continue;
            }
            if (nodes_[bucket].age_ < oldestAge)
            {
                LOG ("operator[]: key: " << key << " conditionally may be put into bucket: " << bucket << " Its age is: " << nodes_[bucket].age_ << std::endl);
                oldestAge = nodes_[bucket].age_;
                oldestElem = &nodes_[bucket];
            }
            bucket = getBucket(bucket);
        }
        create(*oldestElem, key, TValue{});
        return PersistPointer(oldestElem);
    }
private:
    void create(Node& node, const TKey& key, const TValue& val)
    {
        node.key_ = key;
        node.value_ = val;
        node.age_ = ++age_;
        node.occupancy_ = Occupancy::occupied;
    }

    std::decay_t<decltype(Hash())> hashFunction_;
    std::vector<Node> nodes_;
    uint64_t age_ = 0u;
    uint64_t hashMask = (1u  << 21u) - 1;
};

}  // namespace non_std::containers
