#pragma once
#include<iostream>
using namespace std;

#include<new.h>  //��Ҫʹ�� placement new����Ҫ�������ļ�
#include"TypeTraits.hpp"

// Construct ��������һ��ָ��p��һ����ֵvalue���ú�������;���ǽ���ֵ�趨��ָ����ָ�Ŀռ��ϡ�
// C++�� placement new �����ӿ����������һ����
template <class T1, class T2>
inline void Construct(T1* p, const T2& value)
{
	new(p)T1(value);	//����T1:T1(value)
}

// ������destory�ĵ�һ���汾������һ��ָ�룬����ָ����ָ֮����������ֱ�ӵ��øö��������������
// ������ͷŵ�������
template <class T>
inline void Destroy(T* pointer)
{
	pointer->~T();
}

//������destory()�ڶ��汾��������������������[first,last)��Χ�ڵ����ж���������!
//�˺����跨�ҳ�Ԫ�ص���ֵ�ͱ𣬽�������__TypeTraits<> ��ȡ���ʵ���ʩ
template <class ForwardIterator>
inline void Destroy(ForwardIterator first, ForwardIterator last)
{
	__Destroy(first, last, ValueType(first));
}

//�ж�Ԫ�ص���ֵ�ͱ�value type���Ƿ��� TrivialDestructor
template <class ForwardIterator, class T>
inline void __Destroy(ForwardIterator first, ForwardIterator last, T*)
{
	typedef typename __TypeTraits<T>::HasTrivialDestructor TrivialDestructor;
	__DestroyAux(first, last, TrivialDestructor());
}

//���Ԫ�ص���ֵ�ͱ�ValueType���� NonTrivialDestructor
// �ͷ�һ���������
template <class ForwardIterator>
inline void __DestroyAux(ForwardIterator first, ForwardIterator last, __FalseType)
{
	for (; first < last; ++first)
		Destroy(&*first);
}

//���Ԫ�ص���ֵ�ͱ�ValueType���� TrivialDestructor
template <class ForwardIterator>
inline void __DestroyAux(ForwardIterator, ForwardIterator, __TrueType) {}

inline void Destroy(char*, char*){}
inline void Destroy(wchar_t*, wchar_t*){}
