#pragma once

#include <cmath>

namespace non_std
{
namespace fibonacci
{

namespace
{
template <typename TValue>
struct Node
{
    TValue val;

    Node* next = nullptr;
    Node* prev = nullptr;
    Node* top = nullptr;
    Node* child = nullptr;
    bool marked = false;
    unsigned int rank = 0;

public:
    Node(const TValue& val);
};

template<typename TValue>
Node<TValue>::Node(const TValue& val)
    : val(val)
    , next(this)
    , prev(this)
{
}

template <typename T>
void swap(T& lhs, T& rhs)
{
    T buf = rhs;
    rhs = lhs;
    lhs = buf;
}

template<typename TNode>
void swap_position(TNode& lhs, TNode& rhs)
{
    swap(lhs.next, rhs.next);
    swap(lhs.prev, rhs.prev);
}

}  // namespace

template <typename T>
class Heap;

template <typename T>
class Finger
{
public:
    Finger(Node<T>*, Heap<T>*);
    const T& getVal() const;
    void relax(const T& newVal);
private:
    Heap<T>* heap;
    Node<T>* node;
};

template <typename T>
Finger<T>::Finger(Node<T>* n, Heap<T>* h)
    : heap(h)
    , node(n)
{
}

template <typename T>
const T& Finger<T>::getVal() const
{
    return node->val;
}

template <typename T>
class Heap
{
public:
    Heap();
    ~Heap();

    using value_type = T;
    using finger_type = Finger<value_type>;

    const value_type& top() const;
    void pop();
    bool empty() const;
    finger_type insert(const value_type&);

private:
    void relax(Node<value_type>*, int value);
    friend Finger<value_type>;
private:
    void moveHeadsChildrensToTop();
    void consolidate();
    void linkNodes(Node<value_type>* as_parent, Node<value_type>* as_child);
    void detachRoot(Node<value_type>* root);
    void insert(Node<value_type>*);
    void cleanChildList(Node<value_type>* node);
    int allocateA();
    void cut(Node<value_type>* as_parent, Node<value_type>* as_child);
    void cascading(Node<value_type>*);

    Node<value_type>** A = nullptr;
    int real_size = 0;

    Node<value_type>* head = nullptr;
    int n = 0;
};

template <typename T>
void Finger<T>::relax(const T& newVal)
{
    throw "Not implemented yet";
    heap->relax(node, newVal);
}

template <typename T>
Heap<T>::Heap() {}

template <typename T>
Heap<T>::~Heap()
{
    if (head != nullptr)
        cleanChildList(head);
    if (A != nullptr)
        delete[] A;
}

template <typename T>
void Heap<T>::cleanChildList(Node<T>* node)
{
    node->prev->next = nullptr;
    for (auto* it = node; it != nullptr;)
    {
        if (it->child != nullptr)
            cleanChildList(it->child);
          auto to_del = it;
          it = it->next;
          delete to_del;
    }
}

template <typename T>
const T& Heap<T>::top() const
{
    return head->val;
}

template <typename T>
bool Heap<T>::empty() const
{
    return head == nullptr;
}

template <typename T>
Finger<T> Heap<T>::insert(const T& val)
{
    auto* nodeToInsert = new Node<T>(val);
    ++n;
    insert(nodeToInsert);
    return Finger<T>(nodeToInsert, this);
}

template <typename T>
void Heap<T>::insert(Node<T>* nodeToInsert)
{
    if (head == nullptr)
    {
        head = nodeToInsert;
        return;
    }

    auto* leftNode = head;
    auto* rightNode = head->next;
    nodeToInsert ->prev = leftNode;
    nodeToInsert->next = rightNode;

    leftNode->next = nodeToInsert;
    rightNode->prev = nodeToInsert;

    if (head->val < nodeToInsert->val)
    {
        head = nodeToInsert;
    }
}

template <typename T>
void Heap<T>::moveHeadsChildrensToTop()
{
    auto* firstChild = head->child;
    if (firstChild == nullptr)
        return;
    auto* lastChild = firstChild->prev;

    for (auto* it = firstChild; it->top != nullptr; it = it->next)
        it->top = nullptr;

    auto* rightNode = head->next;
    firstChild->prev = head;
    head->next = firstChild;

    rightNode->prev = lastChild;
    lastChild->next = rightNode;
}

template <typename T>
void Heap<T>::pop()
{
    moveHeadsChildrensToTop();
    if (head->next == head)
    {
        delete head;
        head = nullptr;
    }
    else
    {
        auto* oldHead = head;
        head = oldHead->next;

        detachRoot(oldHead);
        delete oldHead;

        consolidate();
    }
    --n;
}

template <typename T>
void Heap<T>::detachRoot(Node<T>* root)
{
    auto* left = root->prev;
    auto* right = root->next;
    left->next = right;
    right->prev = left;
}

template <typename T>
int Heap<T>::allocateA()
{
    int desired_size = ((log(n)) / (log(2))) + 1;

    if (A == nullptr)
    {
        real_size = desired_size + 5;
        A = new Node<T>*[real_size]();
    }
    else if (desired_size > real_size)
    {
        delete[] A;
        real_size = desired_size+5;
        A = new Node<T>*[real_size]();
    }
    return desired_size;
}

template <typename T>
void Heap<T>::consolidate()
{
    const auto A_size = allocateA();
    for (int i = 0; i < A_size; ++i)
        A[i] = nullptr;

    auto it = head;
    head->prev->next = nullptr;
    while (it != nullptr)
    {
        auto* next = it->next;
        {
            it->prev = it;
            it->next = it;

            while (A[it->rank] != nullptr)
            {
                auto* other = A[it->rank];
                A[it->rank] = nullptr;

                if (other->val > it->val)
                    swap (it, other);

                linkNodes(it, other);
            }
        }
        A[it->rank] = it;
        it = next;
    }

    head = nullptr;
    for (auto i = 0; i < A_size; ++i)
    {
        if (A[i] != nullptr)
        {
            insert(A[i]);
        }
    }
}

template <typename T>
void Heap<T>::linkNodes(Node<T>* as_parent, Node<T>* as_child)
{
    detachRoot(as_child);
    as_child->marked = false;

    auto* childHead = as_parent->child;
    if (childHead != nullptr)
    {
        auto* leftNode = childHead;
        auto* rightNode = childHead->next;
        as_child->prev = leftNode;
        as_child->next = rightNode;
        leftNode->next = as_child;
        rightNode->prev = as_child;
    }
    else
    {
        as_child->next = as_child;
        as_child->prev = as_child;
    }
    as_child->top = as_parent;
    as_parent->child = as_child;
    ++as_parent->rank;
}

template <typename T>
void Heap<T>::relax(Node<T>* node, int value)
{
    node->val = value;
    auto* parrent = node->top;
    if (parrent != nullptr && node->val > parrent->val)
    {
        cut(parrent, node);
        cascading(parrent);
    }
    if (node->val < head->val)
    {
        head = node;
    }
}

template <typename T>
void Heap<T>::cut(Node<T>* as_parent, Node<T>* as_child)
{
    --as_parent->rank;
    if (as_child->next!=as_child)
    {
        auto* left = as_child->prev;
        auto* right = as_child->next;
        as_parent->child = left;
        left->next = right;
        right->prev = left;
    }
    else
    {
        as_parent->child = nullptr;
    }

    auto* left = as_parent;
    auto* right = as_parent->next;
    left->next = as_child;
    right->prev = as_child;
    as_child->prev = left;
    as_child->next = right;

    as_child->top = nullptr;
    as_child->marked = false;
}

template <typename T>
void Heap<T>::cascading(Node<T>* toCut)
{
    auto* parrent = toCut->top;
    if (parrent != nullptr)
    {
        if (toCut->marked == false)
        {
            toCut->marked =true;
        }
        else
        {
            cut(parrent, toCut);
            cascading(parrent);
        }
    }
}

} //namespace fibonacci
} //namespace non_std
