#pragma once

#include <utility>
#include <memory>
#include <mutex>

namespace non_std::containers::thread_safe
{

template <typename T>
class Queue
{
	struct Node
	{
		std::shared_ptr<T> data_ = {};
		Node* next_ = nullptr;
		Node() {}
		Node(T&& t)
			: data_(std::make_shared<T>(std::move(t))) {}
	};
public:
	Queue()
		: head_(new Node())
		, tail_(head_)
	{
	}

	~Queue()
	{
		while (pop()); // not really effincient, but working ;)
	}

	void push(T data)
	{
		auto* toAdd = new Node();
		auto dataToPush = std::make_shared<T>(std::move(data));

		std::lock_guard lk(tailMutex_);

		tail_->data_ = dataToPush;
		tail_->next_ = toAdd;
		tail_ = toAdd;
	}

	std::shared_ptr<T> pop()
	{
		decltype(head_) oldHead;
		std::lock_guard lk(headMutex_);
		{
			{
				std::lock_guard lk(tailMutex_);
				if (head_ == tail_) return {};
			}

			oldHead = head_;
			head_ = oldHead->next_;
		}

		std::shared_ptr<T> returnVal;
		returnVal.swap(oldHead->data_);
		
		delete oldHead;
		return returnVal;
	}

	bool empty()
	{
		std::lock_guard headLock(headMutex_);
		std::lock_guard tailLock(tailMutex_);
		return head_ == nullptr;
	}

private:
	std::mutex headMutex_;
	Node* head_ = nullptr;
	std::mutex tailMutex_;
	Node* tail_ = nullptr;
};

}  // namespace non_std::containers::thread_safe
