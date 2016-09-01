#pragma once
#include<string.h>
#include "TypeTraits.hpp"
#include "Iterator.h"

// 是POD型别
// 拷贝一段未初始化的数据
template <class InputIterator, class ForwardIterator>
inline ForwardIterator __UninitializedCopyAux(InputIterator first, InputIterator last, ForwardIterator result, __TrueType)
{
	// 这里实际是调用的C++库里的copy
	// 内部要通过模板推演出数据的类型，再调用memcpy
	return copy(first, last, result);
}

// 是non-POD型别
template <class InputIterator, class ForwardIterator>
ForwardIterator __UninitializedCopyAux(InputIterator first, InputIterator last, ForwardIterator result, __FalseType)
{
	ForwardIterator cur = result;
	try
	{
		for (; first != last; ++first, ++cur)
			Construct(&*cur, *first);	//必须一个一个元素地构造，无法批量进行
	}
	catch (...)
	{
		Destroy(result, cur);
		throw;
	}

	return cur;
}

//-------------------------------------------------------------------------------------------------//
template <class InputIterator, class ForwardIterator, class T>
inline ForwardIterator	__UninitializedCopy(InputIterator first, InputIterator last, ForwardIterator result, T*)
{
	//企图利用IsPODType()所获得的结果，让编译器做参数推导
	return __UninitializedCopyAux(first, last, result, __TypeTraits<T>::IsPODType());
}

///------------------------  该函数能够将内存的配置与对象的析构行为分离开来	--------------------////
//		迭代器first指向输入端的起始位置
//		迭代器last指向输入端的结束为止（前闭后开区间）
//		迭代器result指向输出端（欲初始化空间）的起始处
template <class InputIterator, class ForwardIterator>
inline ForwardIterator UninitializedCopy(InputIterator first, InputIterator last, ForwardIterator result)
{
	//首先萃取出迭代器result的型别，然后判断该型别是否为POD型别：
	return __UninitializedCopy(first, last, result, ValueType(result));
}

////------------------	针对char* 和 wchar_t* 的特化版本    --------------------/////
inline char* UninitializedCopy(const char* first, const char* last, char* result)
{
	memmove(result, first, last - first);
	return result + (last - first);
}
inline wchar_t*	UninitializedCopy(const wchar_t* first, const wchar_t* last, wchar_t* result)
{
	memmove(result, first, sizeof(wchar_t)* (last - first));
	return result + (last - first);
}