#pragma once

#include <optional>
#include <atomic>
#include <memory>
#include <utility>

namespace non_std::containers::thread_safe::lock_free
{
template <typename T>
class Stack
{

struct InternalNode;

struct node_ptr
{
	int count = 0;
	InternalNode* internalPtr = nullptr;
};

struct InternalNode
{
	std::atomic<int> count;
	std::shared_ptr<T> data_;
	node_ptr next;
	InternalNode(const T& data)
		: count(0)
		, data_(std::make_shared<T>(data))
	{
	}
};

public:
	void push(const T& data)
	{
		node_ptr toPush{};
		toPush.internalPtr = new InternalNode(data);
		toPush.count = 1;
		toPush.internalPtr->next = head_.load(std::memory_order_relaxed);

		while (!head_.compare_exchange_weak(toPush.internalPtr->next, toPush,
			std::memory_order_release, std::memory_order_relaxed));
	}

	std::shared_ptr<T> pop()
	{
		node_ptr oldHead = head_.load(std::memory_order_relaxed);
		while (true)
		{
			increaseHeadCounter(oldHead);
			InternalNode* internalNode = oldHead.internalPtr;
			if (not internalNode)
			{
				return std::shared_ptr<T>{};
			}
			if (head_.compare_exchange_strong(oldHead, internalNode->next, std::memory_order_relaxed))
			{
				std::shared_ptr<T> res;
				res.swap(internalNode->data_);

				auto toAdd = oldHead.count - 2;
				if (internalNode->count.fetch_add(toAdd, std::memory_order_release) == -toAdd)
				{
					delete internalNode;
				}
				return res;
			} else if (internalNode->count.fetch_sub(1, std::memory_order_relaxed) == 1)
			{
				delete internalNode;
			}
		}
	}
private:
	void increaseHeadCounter(node_ptr& oldHead)
	{
		node_ptr newHead;
		do
		{
			newHead = oldHead;
			++newHead.count;
		} while (!head_.compare_exchange_weak(oldHead, newHead,
			std::memory_order_acquire, std::memory_order_relaxed));
		oldHead.count = newHead.count;
	}

	std::atomic<node_ptr> head_;
};

}  // non_std::containers::thread_safe::lock_free