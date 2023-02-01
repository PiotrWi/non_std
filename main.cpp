#include <iostream>
#include "containers/HashTableWithRehashing.hpp"
#include "containers/HashMap.hpp"

int main()
{
    non_std::containers::HashTableWithRehashing<uint64_t, uint64_t, 3ul> table;
    non_std::containers::HashMap<uint64_t, uint64_t, 3ul> map;

    *(table[1ul]) = 1;
    *(map[1ul]) = 1;

    return 0;
}
