// #include "tests/lockFreeQueuesTests.hpp"
#include "tests/FixedSizeHashTableOpenHashingWIthAgeTests.hpp"
int main()
{
    // gcc linker errors
    // test::lock_free_structures::test();
    test::fixed_size_hash_table_open_hashing_with_age::test();
    return 0;
}
