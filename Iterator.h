//��������5���ͱ�
//
//1.ֻ��������
struct InputIteratorTag {};
//2.ֻд������
struct OutputIteratorTag {};
//3.ǰ�������
struct ForwardIteratorTag : public InputIteratorTag {};
//4.˫�������
struct BidirectionalIteratorTag : public ForwardIteratorTag {};
//5.������ʵ�����	->vector
struct RandomAccessIteratorTag : public BidirectionalIteratorTag {};


template <class T, class Distance>
struct InputIterator	//ֻ��������
{
	typedef InputIteratorTag  IteratorCategory;
	typedef T						    ValueType;
	typedef Distance             DifferenceType;
	typedef T*					    Pointer;
	typedef T&					    Reference;
};

template <class T, class Distance>
struct OutputIterator		//ֻд������
{
	typedef OutputIteratorTag  IteratorCategory;
	typedef void						   ValueType;
	typedef void						   DifferenceType;
	typedef void						   Pointer;
	typedef void						   Reference;
};

template <class T, class Distance>
struct ForwardIterator	//ǰ�������
{
	typedef ForwardIteratorTag	 IteratorCategory;
	typedef T                    ValueType;
	typedef Distance             DifferenceType;
	typedef T*                   Pointer;
	typedef T&                   Reference;
};

template <class T, class Distance>
struct BidirectionalIterator	//˫�������
{
	typedef BidirectionalIteratorTag IteratorCategory;
	typedef T						 ValueType;
	typedef Distance				 DifferenceType;
	typedef T*                       Pointer;
	typedef T&                       Reference;
};

template <class T, class Distance>
struct RandomAccessIterator		//������ʵ�����
{
	typedef RandomAccessIteratorTag IteratorCategory;
	typedef T                       ValueType;
	typedef Distance                DifferenceType;
	typedef T*                      Pointer;
	typedef T&                      Reference;
};

//-----------------------------------------------------------------------------
// ��������Ƕ������5����Ӧ���ͱ�
// Iterator Category��Value Type��Difference Type��Pointer��Reference
// ��5����Ƕ���ͱ��壬ȷ�����ܹ�������ĸ�STL�ںϡ�
// �ҷ���Iterator Traits��������ȡ
//
template <class Category, class T, class Distance = ptrdiff_t,class Pointer = T*, class Reference = T&>
struct Iterator
{
	typedef Category  IteratorCategory;		// ����������
	typedef T	  	  ValueType;			// ��������ָ��������
	typedef Distance  DifferenceType;		// ����������֮��ľ��� 
	typedef Pointer	  Pointer;				// ��������ָ�������͵�ָ��
	typedef Reference Reference;			// ��������ָ�������͵�����
};
//-------------------------------------------------------------------------------
// Traits����һ̨��������ȡ������եȡ���������������ԣ���Ӧ���ͱ�
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
// ���ԭ��ָ�����Ƶ�traitsƫ�ػ���
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
// ƫ�ػ�constԭ��ָ������
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
// Distance��ʵ��		�������������������ľ���
//1. n������������ ֱ�ӽ�����õ�n��Ϊ����ֵ����

template <class InputIterator>
inline typename IteratorTraits<InputIterator>::DifferenceType __Distance(InputIterator first, InputIterator last, InputIteratorTag)
{
	IteratorTraits<InputIterator>::DifferenceType n = 0;
	//��һ�ۼƾ���
	while (first != last)
	{
		++first; ++n;
	}
	return n;
}

template <class RandomAccessIterator>
inline typename IteratorTraits<RandomAccessIterator>::DifferenceType __Distance(RandomAccessIterator first, RandomAccessIterator last, RandomAccessIteratorTag)
{	//ֱ�Ӽ�����
	//������ʵ����� vector֧��ָ��ֱ�ӽ���++��--
	return last - first;
}

template <class InputIterator>
inline typename IteratorTraits<InputIterator>::DifferenceType Distance(InputIterator first, InputIterator last)
{
	return __Distance(first, last, IteratorTraits<InputIterator>::IteratorCategory());
}

////2. n��Ϊ���ò������ݣ�ֱ�Ӽ���ò���Ҫ����
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
// Advance��ʵ��

template <class InputIterator, class Distance>
inline void __Advance(InputIterator& i, Distance n, InputIteratorTag)
{ //�������������һǰ��
	while (n--) ++i;
}

template <class BidirectionalIterator, class Distance>
inline void __Advance(BidirectionalIterator& i, Distance n, BidirectionalIteratorTag)
{ // ˫�����������һǰ��
	if (n >= 0)
	while (n--) ++i;
	else
	while (n++) --i;
}

template <class RandomAccessIterator, class Distance>
inline void __Advance(RandomAccessIterator& i, Distance n, RandomAccessIteratorTag)
{ //������ʵ�����	˫����Ծǰ��
	i += n;
}

template <class InputIterator, class Distance>
inline void Advance(InputIterator& i, Distance n)
{
	__Advance(i, n, IteratorTraits<InputIterator>::IteratorCategory());
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ���������	
// ���õ������Ķ��壬������������������һ���װ

template <class Iterator>
class ReverseIterator
{
public:
	// ͨ����������ȡ������ȡ������������ж���Ļ�������	
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
		// ע�����������ʱȡ���ǵ�ǰλ�õ�ǰһ�����ݡ�
		// ��ΪRBegin()==End() REnd()==Begin() 
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

