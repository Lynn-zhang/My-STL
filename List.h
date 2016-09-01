#pragma once
#include<iostream>
using namespace std;
//#include "Iterator.h"
//#include "Allocate.h"
//#include "Construct.h"

// //////////**************	�ڵ���	 **************/////////////////
template<class T>
struct __ListNode
{
	__ListNode<T>* _next;
	__ListNode<T>* _prev;
	T _data;

	__ListNode(const T& x = T())
		:_data(x)
		, _next(NULL)
		, _prev(NULL)
	{}
};

// /////////************** ��������	***************/////////////////
// T	�� T& �� T*
// T��const T&��const T*
template<class T, class Ref, class Ptr>
struct __ListIterator
{
	typedef __ListIterator<T, T&, T*> Iterator;
	typedef __ListIterator<T, const T&, const T*> ConstIterator;
	typedef __ListIterator<T, Ref, Ptr>  Self;

	typedef __ListNode<T> ListNode;
	typedef T ValueType;
	typedef Ref Reference;
	typedef Ptr Pointer;

	typedef BidirectionalIteratorTag IteratorCategory;
	typedef ptrdiff_t DifferenceType;

	__ListIterator() {}
	__ListIterator(ListNode* x) : _node(x)	{}
	__ListIterator(const Iterator& x): _node(x._node)	{}

	//��const������Ե���const���͵ĺ���
	//const�����ܵ��÷�const���͵ĺ���
	bool operator==(const Self& x) const
	{
		return _node == x._node;
	}
	bool operator!=(const Self& x) const
	{
		return _node != x._node;
	}
	// T&
	Reference operator*() const
	{
		//return _node->_data;
		return (*_node)._data;
	}
	// T*
	Pointer operator->() const
	{
		//return &_node->_data;
		return &(operator*());
	}

	Self& operator++()
	{
		//_node = (LinkType)((*_node)._next);
		_node = _node->_next;
		return *this;
	}
	Self operator++(int)
	{
		Self tmp = *this;
		++*this;
		return tmp;
	}
	Self& operator--()
	{
		//	_node = (LinkType)((*_node)._prev);
		_node = _node->_prev;
		return *this;
	}
	Self operator--(int)
	{
		Self tmp = *this;
		--*this;
		return tmp;
	}

	ListNode* _node;
};

// //////////**************	 ������	 **************/////////////////
template <class T, class alloc = Alloc>
class List
{
protected:
	typedef __ListNode<T> ListNode;    //�ڵ�
	typedef SimpleAlloc<ListNode, alloc> ListNodeAllocator;
public:
	typedef T ValueType;  //ģ��������� T
	typedef ValueType* Pointer;  // T*
	typedef const ValueType* ConstPointer;	  // const T*
	typedef ValueType& Reference;	// T&
	typedef const ValueType& ConstReference;	// const T&
	//typedef ptrdiff_t difference_type;

	typedef __ListIterator<T, T&, T*> Iterator;	//����������
	typedef __ListIterator<T, const T&, const T*> ConstIterator; //const ������

	//���������
	//typedef ReverseIterator<ConstIterator> ConstReverseRterator;
	typedef ReverseIterator<Iterator> ReverseIterator;
	//----------------------------------** �ָ���  **------------------------------------//
public:
	//������ĳ�ʼ������
	List()
	{
		_head = GetNode();
		_head->_next = _head;
		_head->_prev = _head;
	}

	~List()
	{
		Clear();
		//delete _head;
		DestroyNode(_head);
	}

	ListNode* GetNode(const T& x = T())
	{
		ListNode* node = ListNodeAllocator::Allocate();
		Construct(node, x);
		return node;
	}

	void DestroyNode(ListNode* node)
	{
		Destroy(node);
		ListNodeAllocator::Deallocate(node);
	}
	void Clear()
	{
		Iterator it = Begin();
		while (it != End())
		{
			Iterator del = it;
			++it;
			DestroyNode(del._node);
		}
		_head->_next = _head;
		_head->_prev = _head;
	}

	Iterator Begin()
	{
		return _head->_next;
	}
	ConstIterator Begin() const
	{
		return (ListNode*)((*_head)._next);
	}
	Iterator End()
	{
		return _head;
	}
	ConstIterator End() const
	{
		return _head;
	}
	ReverseIterator RBegin()
	{
		return ReverseIterator(End());
	}

	ReverseIterator REnd()
	{
		return ReverseIterator(Begin());
	}

	bool Empty() const
	{
		return _head->_next == _head;
	}
	size_t Size() const
	{
		size_t result = 0;
		Distance(Begin(), End(), result);
		return result;
	}
	size_t MaxSize() const
	{
		return size_t(-1);
	}
	Reference Front()
	{
		return *Begin();
	}
	ConstReference Front() const
	{
		return *begin();
	}
	Reference Back()
	{
		return *(--End());
	}
	ConstReference Back() const
	{
		return *(--End());
	}

	void PushFront(const T& x)
	{
		Insert(Begin(), x);
	}
	void PushBack(const T& x)
	{
		Insert(End(), x);
	}
	void PopFront()
	{
		Erase(Begin());
	}
	void PopBack()
	{
		Iterator tmp = End();
		Erase(--tmp);
	}

	//��posλ��ǰ����һ���ڵ�
	Iterator Insert(Iterator pos, const T& x)
	{
		ListNode* tmp = GetNode(x);

		ListNode* cur = pos._node;
		ListNode* prev = pos._node->_prev;
		prev->_next = tmp;
		tmp->_next = cur;
		cur->_prev = tmp;
		tmp->_prev = prev;

		return tmp;
	}
	//��posλ��ǰ����n��x
	void Insert(Iterator pos, size_t n, const T& x)
	{
		for (; n > 0; --n)
			Insert(pos, x);
	}

	//ɾ��posλ�õ�����
	Iterator Erase(Iterator pos)
	{
		ListNode* next = pos._node->_next;
		ListNode* prev = pos._node->_prev;
		prev->_next = next;
		next->_prev = prev;
		DestroyNode(pos._node);
		return Iterator(next);
	}
	//ɾ����first��last������
	Iterator Erase(Iterator first, Iterator last)
	{
		while (first != last)
			Erase(first++);
		return last;
	}
	//ɾ������������ֵΪvalue�Ľڵ�
	void Remove(const T& value)
	{
		Iterator first = begin();
		Iterator last = end();
		while (first != last) {
			Iterator next = first;
			++next;
			if (*first == value) Erase(first);
			first = next;
		}
	}
	//ɾ�����ڵ��ظ�Ԫ��
	void Unique()
	{
		Iterator first = Begin();
		Iterator last = End();
		if (first == last) return;
		Iterator next = first;
		while (++next != last) {
			if (*first == *next)
				Erase(next);
			else
				first = next;
			next = first;
		}
	}

protected:
	ListNode* _head;
};


// const���͵ĵ�����
void PrintList1(const List<int>& l)
{
	List<int>::ConstIterator it = l.Begin();
	while (it != l.End())
	{
		//	(*it)++;
		cout << *it << " ";
		++it;
	}
	cout << endl;
}

void PrintList2(List<int>& l)
{
	List<int>::Iterator it = l.Begin();
	while (it != l.End())
	{
		(*it)++;
		cout << *it << " ";
		++it;
	}
	cout << endl;
}

void TestReverseIterator()
{
	List<int> l;
	l.PushBack(1);
	l.PushBack(2);
	l.PushBack(3);
	l.PushBack(4);

	List<int>::ReverseIterator it = l.RBegin();
	while (it != l.REnd())
	{
		cout << *it << " ";
		++it;
	}
	cout << endl;
}


void TestList()
{
	List<int> l;
	l.PushBack(1);
	l.PushBack(2);
	l.PushBack(3);
	l.PushBack(4);
	cout << "----------------------------------------" << endl;

	PrintList1(l);
	PrintList2(l);
	cout << "----------------------------------------" << endl;
	l.PopBack();
	l.PopBack();
	cout << "----------------------------------------" << endl;

	PrintList1(l);
	PrintList2(l);
	cout << "----------------------------------------" << endl;

	TestReverseIterator();
}