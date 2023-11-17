#pragma once

#include <stdint.h>

#include <non_std/BitOperations/Intrincts.hpp>

/*
Some assumptions:
1. Not thread safe.
2. Does not perform object construction/deletion. It just allocates the memory.
3. Does not free memory until destructor is called.
*/
template <typename T>
struct PoolAllocator
{
private:
    struct AllocationPool
    {
        uint64_t allocationMask[4] = {0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff};
        AllocationPool* prev = nullptr;
        AllocationPool* next = nullptr;

        struct Node
        {
            unsigned char payload_[sizeof(T)];
            AllocationPool* poolParrent_;
        } nodes_[256];
        using ptr_type = Node*;
    };
    AllocationPool* availablePools_ = nullptr;
    AllocationPool* fullyAllocatedPools_ = nullptr;
public:
    using value_type = T;

    PoolAllocator() noexcept = default;

    T* allocate()
    {
        if (availablePools_ == nullptr)
        {
            availablePools_ = new AllocationPool();
        }
        for (AllocationPool* current = availablePools_; current != nullptr; current = current->next)
        {
            for (auto i = 0; i < 4; ++i)
            {
                if (current->allocationMask[i] != 0x0)
                {
                    unsigned char freeBit = bit_operations::intrincs::findFirstSet(current->allocationMask[i]) - 1;
                    current->allocationMask[i] ^= (1ull <<freeBit);

                    auto freeField = i * 64 + freeBit;
                    current->nodes_[freeField].poolParrent_ = current;

                    if (isFull(current))
                    {
                        moveTofullyAllocated(current);
                    }
                    return (T*)(&(current->nodes_[freeField]));
                }
            }
        }
        return nullptr;
    }
    void dealocate(T* in)
    {
        AllocationPool* toBeRemoved = (((typename AllocationPool::ptr_type)(in))->poolParrent_);
        if (isFull(toBeRemoved))
        {
            moveToAvaiable(toBeRemoved);
        }
        unsigned index = ((typename AllocationPool::ptr_type)(in)) - toBeRemoved->nodes_;
        auto wordIndex = (index >> 6);
        auto bit = index & 0b111111;

        toBeRemoved->allocationMask[wordIndex] ^= (1ull << bit);
    }
/*
    void clearAll()
    {
        deleteNext(availablePools_);
        deleteNext(fullyAllocatedPools_);
        availablePools_ = nullptr;
        fullyAllocatedPools_ = nullptr;
    }
*/
    void clearAll()
    {
        while (fullyAllocatedPools_ != nullptr)
        {
            moveToAvaiable(fullyAllocatedPools_);
        }
        auto* it = availablePools_;
        while (it != nullptr)
        {
            it->allocationMask[0] = 0xffffffffffffffffull;
            it->allocationMask[1] = 0xffffffffffffffffull;
            it->allocationMask[2] = 0xffffffffffffffffull;
            it->allocationMask[3] = 0xffffffffffffffffull;
            it = it->next;
        }
    }

    ~PoolAllocator()
    {
        deleteNext(availablePools_);
        deleteNext(fullyAllocatedPools_);
    }
private:
    bool isFull(AllocationPool* in)
    {
        return (in->allocationMask[0] == 0x0) & (in->allocationMask[1] == 0x0)
               & (in->allocationMask[2] == 0x0) & (in->allocationMask[3] == 0x0);
    }

    void moveTofullyAllocated(AllocationPool* in)
    {
        availablePools_ = in->next;
        if (fullyAllocatedPools_ != nullptr)
        {
            fullyAllocatedPools_->prev = in;
        }
        in->next = fullyAllocatedPools_;
        fullyAllocatedPools_ = in;
    }

    void moveToAvaiable(AllocationPool* in)
    {
        fullyAllocatedPools_ = in->next;
        if (availablePools_ != nullptr)
        {
            availablePools_->prev = in;
        }
        in->next = availablePools_;
        availablePools_ = in;
    }

    void deleteNext(AllocationPool* in)
    {
        if (in != nullptr)
        {
            deleteNext(in->next);
            delete in;
        }
    }

};
