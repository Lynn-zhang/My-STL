#pragma once
#include<iostream>
using namespace std;

#include<new.h>  //想要使用 placement new，需要包含此文件
#include"TypeTraits.hpp"

// Construct 函数接受一个指针p和一个初值value，该函数的用途就是将初值设定到指针所指的空间上。
// C++的 placement new 运算子可用来完成这一任务
template <class T1, class T2>
inline void Construct(T1* p, const T2& value)
{
	new(p)T1(value);	//调用T1:T1(value)
}

// 以下是destory的第一个版本，接受一个指针，将该指针所指之物析构掉（直接调用该对象的析构函数）
// 构造和释放单个对象
template <class T>
inline void Destroy(T* pointer)
{
	pointer->~T();
}

//以下是destory()第二版本，接受两个迭代器，将[first,last)范围内的所有对象析构掉!
//此函数设法找出元素的数值型别，进而利用__TypeTraits<> 求取最适当措施
template <class ForwardIterator>
inline void Destroy(ForwardIterator first, ForwardIterator last)
{
	__Destroy(first, last, ValueType(first));
}

//判断元素的数值型别（value type）是否有 TrivialDestructor
template <class ForwardIterator, class T>
inline void __Destroy(ForwardIterator first, ForwardIterator last, T*)
{
	typedef typename __TypeTraits<T>::HasTrivialDestructor TrivialDestructor;
	__DestroyAux(first, last, TrivialDestructor());
}

//如果元素的数值型别（ValueType）有 NonTrivialDestructor
// 释放一组区间对象
template <class ForwardIterator>
inline void __DestroyAux(ForwardIterator first, ForwardIterator last, __FalseType)
{
	for (; first < last; ++first)
		Destroy(&*first);
}

//如果元素的数值型别（ValueType）有 TrivialDestructor
template <class ForwardIterator>
inline void __DestroyAux(ForwardIterator, ForwardIterator, __TrueType) {}

inline void Destroy(char*, char*){}
inline void Destroy(wchar_t*, wchar_t*){}
