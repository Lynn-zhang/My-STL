//迭代器的5种型别
//
//1.只读迭代器
struct InputIteratorTag {};
//2.只写迭代器
struct OutputIteratorTag {};
//3.前向迭代器
struct ForwardIteratorTag : public InputIteratorTag {};
//4.双向迭代器
struct BidirectionalIteratorTag : public ForwardIteratorTag {};
//5.随机访问迭代器	->vector
struct RandomAccessIteratorTag : public BidirectionalIteratorTag {};


template <class T, class Distance>
struct InputIterator	//只读迭代器
{
	typedef InputIteratorTag  IteratorCategory;
	typedef T						    ValueType;
	typedef Distance             DifferenceType;
	typedef T*					    Pointer;
	typedef T&					    Reference;
};

template <class T, class Distance>
struct OutputIterator		//只写迭代器
{
	typedef OutputIteratorTag  IteratorCategory;
	typedef void						   ValueType;
	typedef void						   DifferenceType;
	typedef void						   Pointer;
	typedef void						   Reference;
};

template <class T, class Distance>
struct ForwardIterator	//前向迭代器
{
	typedef ForwardIteratorTag	 IteratorCategory;
	typedef T                    ValueType;
	typedef Distance             DifferenceType;
	typedef T*                   Pointer;
	typedef T&                   Reference;
};

template <class T, class Distance>
struct BidirectionalIterator	//双向迭代器
{
	typedef BidirectionalIteratorTag IteratorCategory;
	typedef T						 ValueType;
	typedef Distance				 DifferenceType;
	typedef T*                       Pointer;
	typedef T&                       Reference;
};

template <class T, class Distance>
struct RandomAccessIterator		//随机访问迭代器
{
	typedef RandomAccessIteratorTag IteratorCategory;
	typedef T                       ValueType;
	typedef Distance                DifferenceType;
	typedef T*                      Pointer;
	typedef T&                      Reference;
};

//-----------------------------------------------------------------------------
// 迭代器内嵌包含的5种相应的型别
// Iterator Category、Value Type、Difference Type、Pointer、Reference
// 这5种内嵌的型别定义，确保了能够更方便的跟STL融合。
// 且方便Iterator Traits的类型萃取
//
template <class Category, class T, class Distance = ptrdiff_t,class Pointer = T*, class Reference = T&>
struct Iterator
{
	typedef Category  IteratorCategory;		// 迭代器类型
	typedef T	  	  ValueType;			// 迭代器所指对象类型
	typedef Distance  DifferenceType;		// 两个迭代器之间的距离 
	typedef Pointer	  Pointer;				// 迭代器所指对象类型的指针
	typedef Reference Reference;			// 迭代器所指对象类型的引用
};
//-------------------------------------------------------------------------------
// Traits就像一台“特性萃取机”，榨取各个迭代器的特性（对应的型别）
template <class Iterator>
struct IteratorTraits
{
	typedef typename Iterator::IteratorCategory  IteratorCategory;
	typedef typename Iterator::ValueType         ValueType;
	typedef typename Iterator::DifferenceType    DifferenceType;
	typedef typename Iterator::Pointer           Pointer;
	typedef typename Iterator::Reference         Reference;
};

//-------------------------------------------------------------------------------
// 针对原生指针而设计的traits偏特化版
//  vector
template <class T>
struct IteratorTraits<T*>
{
	typedef RandomAccessIteratorTag    IteratorCategory;
	typedef T                          ValueType;
	typedef ptrdiff_t                  DifferenceType;
	typedef T*                         Pointer;
	typedef T&                         Reference;
};

//-------------------------------------------------------------------------------
// 偏特化const原生指针类型
//
template <class T>
struct IteratorTraits<const T*>
{
	typedef RandomAccessIteratorTag		IteratorCategory;
	typedef T							ValueType;
	typedef ptrdiff_t					DifferenceType;
	typedef const T*					Pointer;
	typedef const T&					Reference;
};
///////////////////////////////////////////////////////////////////////////////

template <class T, class Distance>
inline T* ValueType(const InputIterator<T, Distance>&)
{
	return (T*)(0);
}

template <class T, class Distance>
inline T* ValueType(const ForwardIterator<T, Distance>&)
{
	return (T*)(0);
}

template <class T, class Distance>
inline T* ValueType(const BidirectionalIterator<T, Distance>&)
{
	return (T*)(0);
}

template <class T, class Distance>
inline T* ValueType(const RandomAccessIterator<T, Distance>&)
{
	return (T*)(0);
}

template <class T>
inline T* ValueType(const T*)
{
	return (T*)(0);
}

// ---------------------------------------------------------------------------- 
// Distance的实现		用来结算两个迭代器的距离
//1. n不做参数传递 直接将计算好的n作为返回值返回

template <class InputIterator>
inline typename IteratorTraits<InputIterator>::DifferenceType __Distance(InputIterator first, InputIterator last, InputIteratorTag)
{
	IteratorTraits<InputIterator>::DifferenceType n = 0;
	//逐一累计距离
	while (first != last)
	{
		++first; ++n;
	}
	return n;
}

template <class RandomAccessIterator>
inline typename IteratorTraits<RandomAccessIterator>::DifferenceType __Distance(RandomAccessIterator first, RandomAccessIterator last, RandomAccessIteratorTag)
{	//直接计算差距
	//随机访问迭代器 vector支持指针直接进行++或--
	return last - first;
}

template <class InputIterator>
inline typename IteratorTraits<InputIterator>::DifferenceType Distance(InputIterator first, InputIterator last)
{
	return __Distance(first, last, IteratorTraits<InputIterator>::IteratorCategory());
}

////2. n作为引用参数传递，直接计算好不需要返回
//template <class InputIterator, class Distance>
//inline void __Distance(InputIterator first, InputIterator last, Distance& n,InputIteratorTag) 
//{
//	while (first != last)
//	{ 
//		++first; 
//		++n; 
//	}
//}
//
//template <class RandomAccessIterator, class Distance>
//inline void __Distance(RandomAccessIterator first, RandomAccessIterator last,Distance& n, RandomAccessIteratorTag)
//{
//	n += (last - first);
//}
//
//template <class InputIterator, class Distance>
//inline void Distance(InputIterator first, InputIterator last, Distance& n)
//{
//	//__Distance(first, last, n, IteratorCategory(first));
//	__Distance(first, last, n, IteratorTraits<InputIterator>::IteratorCategory());
//}

////---------------------------------------------------------------------------- 
// Advance的实现

template <class InputIterator, class Distance>
inline void __Advance(InputIterator& i, Distance n, InputIteratorTag)
{ //单向迭代器，逐一前进
	while (n--) ++i;
}

template <class BidirectionalIterator, class Distance>
inline void __Advance(BidirectionalIterator& i, Distance n, BidirectionalIteratorTag)
{ // 双向迭代器，逐一前进
	if (n >= 0)
	while (n--) ++i;
	else
	while (n++) --i;
}

template <class RandomAccessIterator, class Distance>
inline void __Advance(RandomAccessIterator& i, Distance n, RandomAccessIteratorTag)
{ //随机访问迭代器	双向，跳跃前进
	i += n;
}

template <class InputIterator, class Distance>
inline void Advance(InputIterator& i, Distance n)
{
	__Advance(i, n, IteratorTraits<InputIterator>::IteratorCategory());
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 反向迭代器	
// 逆置迭代器的定义，反向迭代器是正向迭代一层封装

template <class Iterator>
class ReverseIterator
{
public:
	// 通过迭代器萃取器，萃取出正向迭代器中定义的基本类型	
	typedef typename IteratorTraits<Iterator>::IteratorCategory
		IteratorCategory;
	typedef typename IteratorTraits<Iterator>::ValueType
		ValueType;
	typedef typename IteratorTraits<Iterator>::DifferenceType
		DifferenceType;
	typedef typename IteratorTraits<Iterator>::Pointer
		Pointer;
	typedef typename IteratorTraits<Iterator>::Reference
		Reference;

	typedef Iterator IteratorType;
	typedef ReverseIterator<Iterator> Self;

public:
	ReverseIterator()
	{}
	explicit ReverseIterator(IteratorType x)
		: _current(x)
	{}
	ReverseIterator(const Self& x)
		: _current(x._current)
	{}

	Reference operator*() const
	{
		// 注意这里解引用时取的是当前位置的前一个数据。
		// 以为RBegin()==End() REnd()==Begin() 
		Iterator tmp = _current;
		return *--tmp;
	}

	Pointer operator->() const
	{
		return &(operator*());
	}

	Self& operator++() {
		--_current;
		return *this;
	}
	Self operator++(int) {
		Self tmp = *this;
		--_current;
		return tmp;
	}
	Self& operator--()
	{
		++_current;
		return *this;
	}
	Self operator--(int)
	{
		Self tmp = *this;
		++_current;
		return tmp;
	}

	bool operator != (const Self& x) {
		return _current != x._current;
	}

	Self operator+(DifferenceType n) const
	{
		return Self(_current - n);
	}
	Self& operator+=(DifferenceType n)
	{
		_current -= n;
		return *this;
	}
	Self operator-(DifferenceType n) const
	{
		return Self(_current + n);
	}
	Self& operator-=(DifferenceType n)
	{
		_current += n;
		return *this;
	}
	Reference operator[](DifferenceType n) const
	{
		return *(*this + n);
	}
	IteratorType Base() const
	{
		return _current;
	}

protected:
	Iterator _current;
};


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

