#pragma once

#include <vector>
#include <utility>
#include <optional>

namespace non_std::containers
{

/*
 * Assumptions:
 *  * Elements cannot be cleared. It is to do not consider situation when get returns empty,
 *      but elements can be find in rehashed.
 * */
template<typename TKey,
        typename TValue,
        unsigned int TBucketBitWidth,
        typename Hash = std::hash<TKey>,
        typename Rehash = std::hash<std::decay_t<decltype(Hash()(std::declval<TKey>()))>>,
        unsigned int HashRetries = 3ul>
class HashTableWithRehashing
{
    struct Node;
public:
    HashTableWithRehashing(const HashTableWithRehashing &) = delete;
    HashTableWithRehashing &operator=(const HashTableWithRehashing &in) = delete;

    HashTableWithRehashing()
        :data_(1 << TBucketBitWidth)
    {
    }

    TValue* operator[](const TKey key) noexcept
    {
        auto hash = hashFunction_(key);
        auto bucket = hash & hashMask;

        for (unsigned int i = 0ul; i < HashRetries + 1; ++i)
        {
            if (not data_[bucket])
            {
                data_[bucket] = {key, {}};
                return &(data_[bucket]->val);
            }
            else if (data_[bucket]->key == key)
            {
                return &(data_[bucket]->val);
            }

            hash = rehashFunction_(hash);
            bucket = hash & hashMask;
        }
        return nullptr;
    }

    TValue* get(const TKey key) const noexcept
    {
        auto hash = hashFunction_(key);
        auto bucket = hash & hashMask;

        for (unsigned int i = 0ul; i < HashRetries + 1; ++i)
        {
            if (data_[bucket] && data_[bucket]->key == key)
            {
                return &(data_[bucket]->val);
            }

            hash = rehashFunction_(hash);
            bucket = hash & hashMask;
        }
        return nullptr;
    }

    void store(const TKey key, TValue val)
    {
        auto hash = hashFunction_(key);
        auto bucket = hash & hashMask;

        for (unsigned int i = 0ul; i < HashRetries + 1; ++i)
        {
            if (not data_[bucket] || data_[bucket]->key == key)
            {
                data_[bucket] = {key, std::move(val)};
            }

            hash = rehashFunction_(hash);
            bucket = hash & hashMask;
        }
    }
private:
    struct Node
    {
        TKey key;
        TValue val;
    };

    std::decay_t<decltype(Hash())> hashFunction_;
    std::decay_t<decltype(Rehash())> rehashFunction_;
    std::vector<std::optional<Node>> data_;
    static constexpr uint64_t hashMask = (1 << TBucketBitWidth) - 1;
};

}  // namespace non_std::containers
