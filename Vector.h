//#define _SCL_SECURE_NO_WARNINGS
#pragma once

//#include"Allocate.h"
//#include"Construct.h"
//#include"Uninitialized.h"

template<class T>
class Vector
{
public:
	typedef T ValueType;
	typedef ValueType* Iterator;  //����ģ�����͵�ָ��Ϊ����������
	typedef const ValueType* ConstIterator;  //const���͵ĵ�����
	typedef ValueType& Reference;    //ģ�������ΪReference����
	typedef ReverseIterator<Iterator>	ReverseIterator; //���������
	typedef SimpleAlloc<ValueType, Alloc> DataAllocator;

	Vector()
		:_start(0)
		, _finish(0)
		, _endOfStorage(0)
	{}

	//*********************//
	~Vector()
	{
		//delete[] _start;
		if (_start)
		{
			Destroy(_start, _finish);
			DataAllocator::Deallocate(_start, _endOfStorage - _start);
		}

		_start = _finish = _endOfStorage = 0;
	}
	//*********************//

	Reference operator[](size_t index)
	{
		assert(_start + index < _finish);
		return _start[index];
	}

	Iterator Begin()
	{		return _start;	}

	ConstIterator Begin() const
	{		return _start;	}

	Iterator End()
	{		return _finish;	}

	ConstIterator End() const
	{		return _finish;	}

	ReverseIterator RBegin()
	{		return ReverseIterator(End());		}

	ReverseIterator REnd()
	{		return ReverseIterator(Begin());	}

	size_t Size()
	{		return _finish - _start;	}

	size_t Capacity()   //��ȡ����
	{		return _endOfStorage - _start;	}
	//β��
	void PushBack(const T&x)
	{
		//��������ǰ��������������֤��Խ��
		_CheckStorage();
		*_finish = x;
		++_finish;
	}
	//βɾ
	void PopBack()
	{
		assert(_finish != _start);
		--_finish;
	}
	//--------------- ���� ------------------//
	void _CheckStorage()
	{
		////��ָ�����ݿ��β���±��Ѿ��ﵽ��������ʱ��Ҫ��������
		//if (_finish == _endOfStorage)
		//{
		//	size_t size = _finish - _start;
		//	size_t newSize = size * 2 + 3;
		//	//1.�½�һ���ڴ棬��_start�е����ݶ����ƹ���
		//	ValueType* tmp = new ValueType[newSize];
		//	for (size_t i = 0; i < size; ++i)
		//	{
		//		tmp[i] = _start[i];
		//	}
		//	//2.�ڽ�tmp����_start
		//	delete[] _start;
		//	_start = tmp;
		//	_finish = _start + size;
		//	_endOfStorage = _start + newSize;
		//}

		if (_finish == _endOfStorage)
		{
			size_t size = _finish - _start;
			size_t newSize = size * 2 + 3;
			ValueType* tmp = DataAllocator::Allocate(newSize);
			
			// ���������ű��뾭�����������á�Ԫ���ƶ����ͷ�ԭ�ռ䡱
			// ������������ �����ÿռ��������ͷſռ�
			if (_start)
			{
				// ��ɿ���
				UninitializedCopy(_start, _finish, tmp);
				Destroy(_start, _finish);
				DataAllocator::Deallocate(_start);
			}

			_start = tmp;
			_finish = _start + size;
			_endOfStorage = _start + newSize;
		}
	}
private:
	Iterator _start;		//ָ�����ݿ�Ŀ�ʼ(�±�)
	Iterator _finish;		//ָ�����ݿ�Ľ�β
	Iterator _endOfStorage;	//ָ��洢������β
};

//******************************��������***************************//
//	1��2��3��4��5
void PrintVector1(const Vector<int>& v)
{ //��ͷ��β��ӡvector
	Vector<int>::ConstIterator it = v.Begin();
	while (it != v.End())
	{//��������������ָ����һ��λ��
		cout << *it << " ";
		++it;
	}
	cout << endl;
}
//2��3��4��5��6
void PrintVector2(Vector<int>& v)
{
	Vector<int>::Iterator it = v.Begin();
	while (it != v.End())
	{ //������ָ�����������1�������������ָ����һ��λ��
		(*it)++;
		cout << *it << " ";
		++it;
	}
	cout << endl;
}
//�����±����vector
// 3��4��5��6��7
void PrintVector3(Vector<int>& v)
{
	for (size_t i = 0; i < v.Size(); ++i)
	{//vector�е����ݱ�������
		v[i]++;
		cout << v[i] << " ";
	}
	cout << endl;
}
//���÷���������������
// 7��6��5��4��3
void PrintVector4(Vector<int>& v)
{
	Vector<int>::ReverseIterator it = v.RBegin();
	while (it != v.REnd())
	{
		cout << *it << " ";
		++it;
	}
	cout << endl;
}

void TestVector()
{
	Vector<int> v;
	v.PushBack(1);
	v.PushBack(2);
	v.PushBack(3);
	v.PushBack(4);
	v.PushBack(5);

	PrintVector1(v);
	PrintVector2(v);
	PrintVector3(v); 
	PrintVector4(v);
}