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
	typedef ValueType* Iterator;  //定义模板类型的指针为迭代器类型
	typedef const ValueType* ConstIterator;  //const类型的迭代器
	typedef ValueType& Reference;    //模板的引用为Reference类型
	typedef ReverseIterator<Iterator>	ReverseIterator; //反向迭代器
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

	size_t Capacity()   //获取容量
	{		return _endOfStorage - _start;	}
	//尾插
	void PushBack(const T&x)
	{
		//增添数据前必须检查容量，保证不越界
		_CheckStorage();
		*_finish = x;
		++_finish;
	}
	//尾删
	void PopBack()
	{
		assert(_finish != _start);
		--_finish;
	}
	//--------------- 测容 ------------------//
	void _CheckStorage()
	{
		////当指向数据块结尾的下标已经达到容量限制时，要进行扩容
		//if (_finish == _endOfStorage)
		//{
		//	size_t size = _finish - _start;
		//	size_t newSize = size * 2 + 3;
		//	//1.新建一块内存，将_start中的数据都复制过来
		//	ValueType* tmp = new ValueType[newSize];
		//	for (size_t i = 0; i < size; ++i)
		//	{
		//		tmp[i] = _start[i];
		//	}
		//	//2.在将tmp赋给_start
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
			
			// 容量的扩张必须经历“重新配置、元素移动、释放原空间”
			// 调用析构函数 并调用空间配置器释放空间
			if (_start)
			{
				// 完成拷贝
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
	Iterator _start;		//指向数据块的开始(下标)
	Iterator _finish;		//指向数据块的结尾
	Iterator _endOfStorage;	//指向存储容量的尾
};

//******************************测试用例***************************//
//	1、2、3、4、5
void PrintVector1(const Vector<int>& v)
{ //从头至尾打印vector
	Vector<int>::ConstIterator it = v.Begin();
	while (it != v.End())
	{//先输出，后迭代器指向下一个位置
		cout << *it << " ";
		++it;
	}
	cout << endl;
}
//2，3，4，5，6
void PrintVector2(Vector<int>& v)
{
	Vector<int>::Iterator it = v.Begin();
	while (it != v.End())
	{ //迭代器指向的数据自增1，输出，迭代器指向下一个位置
		(*it)++;
		cout << *it << " ";
		++it;
	}
	cout << endl;
}
//按照下标遍历vector
// 3、4、5、6、7
void PrintVector3(Vector<int>& v)
{
	for (size_t i = 0; i < v.Size(); ++i)
	{//vector中的数据本身自增
		v[i]++;
		cout << v[i] << " ";
	}
	cout << endl;
}
//利用反向迭代器依次输出
// 7、6、5、4、3
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