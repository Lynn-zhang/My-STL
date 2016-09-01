#pragma once
#include<string.h>
#include "TypeTraits.hpp"
#include "Iterator.h"

// ��POD�ͱ�
// ����һ��δ��ʼ��������
template <class InputIterator, class ForwardIterator>
inline ForwardIterator __UninitializedCopyAux(InputIterator first, InputIterator last, ForwardIterator result, __TrueType)
{
	// ����ʵ���ǵ��õ�C++�����copy
	// �ڲ�Ҫͨ��ģ�����ݳ����ݵ����ͣ��ٵ���memcpy
	return copy(first, last, result);
}

// ��non-POD�ͱ�
template <class InputIterator, class ForwardIterator>
ForwardIterator __UninitializedCopyAux(InputIterator first, InputIterator last, ForwardIterator result, __FalseType)
{
	ForwardIterator cur = result;
	try
	{
		for (; first != last; ++first, ++cur)
			Construct(&*cur, *first);	//����һ��һ��Ԫ�صع��죬�޷���������
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
	//��ͼ����IsPODType()����õĽ�����ñ������������Ƶ�
	return __UninitializedCopyAux(first, last, result, __TypeTraits<T>::IsPODType());
}

///------------------------  �ú����ܹ����ڴ������������������Ϊ���뿪��	--------------------////
//		������firstָ������˵���ʼλ��
//		������lastָ������˵Ľ���Ϊֹ��ǰ�պ����䣩
//		������resultָ������ˣ�����ʼ���ռ䣩����ʼ��
template <class InputIterator, class ForwardIterator>
inline ForwardIterator UninitializedCopy(InputIterator first, InputIterator last, ForwardIterator result)
{
	//������ȡ��������result���ͱ�Ȼ���жϸ��ͱ��Ƿ�ΪPOD�ͱ�
	return __UninitializedCopy(first, last, result, ValueType(result));
}

////------------------	���char* �� wchar_t* ���ػ��汾    --------------------/////
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