#pragma once

#include <optional>
#include <atomic>
#include <memory>
#include <utility>

namespace non_std::containers::thread_safe::lock_free
{

template <typename T>
class Queue
{
	struct InternalNode;
	struct Node
	{
		InternalNode* internalNode_ = nullptr;
		int externalCount_;
	};

	struct NodeCounter
	{
		unsigned internalCount : 30;
		unsigned externalCount : 2;
	};

	struct InternalNode
	{
		std::atomic<T*> data_ = {};
		std::atomic<NodeCounter> count_;
		Node next_;

		InternalNode()
		{
			NodeCounter counter;
			counter.internalCount = 0;
			counter.externalCount = 2;
			count_.store(counter);

			next_.externalCount_ = 2;
			next_.internalNode_ = nullptr;
		}

		void releaseRef()
		{
			NodeCounter oldCounter = count_.load();
			NodeCounter newCounter;
			do
			{
				newCounter = oldCounter;
				--newCounter.internalCount;
			} while (!count_.compare_exchange_strong(oldCounter, newCounter));

			if (newCounter.internalCount == 0 && newCounter.externalCount == 0)
			{
				delete this;
			}
		}
	};
public:
	Queue()
	{
		Node head;
		head.externalCount_ = 1;
		head.internalNode_ = new InternalNode();
		head_.store(head);
		tail_.store(head);
	}

	~Queue()
	{
		while (pop()); // not really effincient, but working ;)
		delete tail_.load().internalNode_;
	}

	void push(T data)
	{
		std::unique_ptr<T> newData(new T(data));
		Node newNext;
		newNext.internalNode_ = new InternalNode;
		newNext.externalCount_ = 1;

		Node oldTail = tail_.load();

		while (true)
		{
			increaseExternalCount(tail_, oldTail);
			T* oldData = nullptr;
			if (oldTail.internalNode_->data_.compare_exchange_strong(oldData, newData.get()))
			{
				oldTail.internalNode_->next_ = newNext;
				oldTail = tail_.exchange(newNext);
				freeExternalCounter(oldTail);
				newData.release();
				break;
			}
			oldTail.internalNode_->releaseRef();
		}
	}

	std::unique_ptr<T> pop()
	{
		Node oldHead = head_.load();

		while (true)
		{
			increaseExternalCount(head_, oldHead);
			InternalNode* ptr = oldHead.internalNode_;
			if (ptr == tail_.load().internalNode_)
			{
				ptr->releaseRef();
				return std::unique_ptr<T>();
			}
			if (head_.compare_exchange_strong(oldHead, ptr->next_))
			{
				T* const res = ptr->data_.exchange(nullptr);
				freeExternalCounter(oldHead);
				return std::unique_ptr<T>(res);
			}
			ptr->releaseRef();
		}
	}
private:
	void increaseExternalCount(std::atomic<Node>& counter,
		Node& oldCounter)
	{
		Node newCounter;

		do
		{
			newCounter = oldCounter;
			++newCounter.externalCount_;
		} while (!counter.compare_exchange_strong(oldCounter, newCounter));
		oldCounter.externalCount_ = newCounter.externalCount_;
	}

	void freeExternalCounter(Node& oldNode)
	{
		InternalNode* ptr = oldNode.internalNode_;
		int const count_increase = oldNode.externalCount_ - 2;
		NodeCounter oldCounter = ptr->count_.load();
		NodeCounter newCounter;
		do
		{
			newCounter = oldCounter;
			--newCounter.externalCount;
			newCounter.internalCount += count_increase;
		} while (!ptr->count_.compare_exchange_strong(oldCounter, newCounter));

		if (!newCounter.internalCount && !newCounter.externalCount)
		{
			delete ptr;
		}
	}
private:
	std::atomic<Node> head_;
	std::atomic<Node> tail_;
};

}
