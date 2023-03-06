#pragma once

#include <optional>
#include <atomic>
#include <memory>
#include <utility>

namespace non_std::containers::thread_safe::lock_free
{

// Low trafic stack 
template <typename T>
class StackLT
{
	struct Node
	{
		Node* next = nullptr;
		std::shared_ptr<T> data_;
	};

public:
	void push(const T& t);
	std::shared_ptr<T> pop();
private:
	std::atomic<Node*> head_ = nullptr;

private:
	void deleteNodes(Node*);
	void tryReclaim(Node* node);
	void chainNode(Node* node);
	void chainNodes(Node* node);
	void chainNodes(Node* first, Node* last);
	std::atomic<unsigned> threadsInPop_ = 0;
	std::atomic<Node*> postponedDeletes_;
};

template <typename T>
void StackLT<T>::push(const T& t)
{
	auto* nodeToPush = new Node(this->head_, std::make_shared<T>(t));
	while(!head_.compare_exchange_weak(nodeToPush->next, nodeToPush));
}

template <typename T>
std::shared_ptr<T> StackLT<T>::pop()
{
	++threadsInPop_;
	Node* oldhead = head_.load();
	while (oldhead && !head_.compare_exchange_weak(oldhead, oldhead->next));

	std::shared_ptr<T> ret;
	if (oldhead)
	{
		ret.swap(oldhead->data_);
	}
	tryReclaim(oldhead);
	return ret;
}

template <typename T>
void StackLT<T>::deleteNodes(Node* node)
{
	while (node != nullptr)
	{
		Node* next = node->next;
		delete node;
		node = next;
	}
}

template <typename T>
void StackLT<T>::tryReclaim(Node* node)
{
	if (1 == threadsInPop_)
	{
		auto* toBeDeleted = postponedDeletes_.exchange(nullptr);
		if (!--threadsInPop_)
		{
			deleteNodes(toBeDeleted);
		}
		else if (toBeDeleted)
		{
			chainNodes(toBeDeleted);
		}
		delete node;
	}
	else
	{
		chainNode(node);
		--threadsInPop_;
	}
}

template <typename T>
void StackLT<T>::chainNodes(Node* node)
{
	auto* last = node;
	while (Node* const next = last->next)
	{
		last = next;
	}
	chainNodes(node, last);
}

template <typename T>
void StackLT<T>::chainNodes(Node* first, Node* last)
{
	last->next = postponedDeletes_.load();
	while (!postponedDeletes_.compare_exchange_weak(last->next, first));
}

template <typename T>
void StackLT<T>::chainNode(Node* node)
{
	chainNodes(node, node);
}
}  // namespace non_std::containers::thread_safe::lock_free
