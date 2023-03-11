#include "FixedSizeHashTableOpenHashingWIthAgeTests.hpp"

#include <non_std/containers/FixedSizeHashTableOpenHashingWithAge.hpp>

#include <cassert>
#include <iostream>
#include <stdint.h>

namespace test::fixed_size_hash_table_open_hashing_with_age
{

struct PassTrhoughtHash
{
    uint64_t operator()(uint64_t in) {return in;}
};

struct ValueType
{
    float field1;
    double field2;
    int field3;
};

non_std::containers::FixedSizeHashTableOpenHashingWithAge<uint64_t, ValueType, PassTrhoughtHash> hastTable;

void testGivenKey(uint64_t key)
{
    auto val1 = hastTable[key];
    val1->field3 = 3;

    assert(hastTable.get(key)->field3 == 3 && "hash table shall remember previously set value");
}

void testOverrideOlderElements()
{
    std::vector<std::pair<unsigned int, uint64_t>> hashes;
    uint64_t basePart = 31;
    for (unsigned int i = 0; i < decltype(hastTable)::HashTries; ++i)
    {
        basePart += (1ull << 60);
        hashes.push_back(std::make_pair(i, basePart));
        hastTable[basePart]->field3 = i;
    }

    // check all still in collection
    for (const auto& [expectedVal, hash]: hashes)
    {
        auto hashedValue = hastTable.get(hash);
        assert(hastTable.get(hash) != nullptr
            && "Value shall not be overwritten  because, it does not reach age limit");
        assert(hastTable.get(hash)->field3 == expectedVal && "Value");
    }

    basePart += (1ull << 60);
    hastTable[basePart]->field3 = 1001;

    assert(hastTable.get(hashes.front().second).operator ValueType *() == nullptr && "The oldest element shall be removed right now");

}

void test()
{
    for (uint64_t key = 1; key < 1000ull; ++key)
    {
        testGivenKey(key);
    }

    testOverrideOlderElements();

    std::cout << "fixed_size_hash_table_open_hashing_with_age passed" << std::endl;
}

}  // test::fixed_size_hash_table_open_hashing_with_age

