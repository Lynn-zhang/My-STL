#pragma once 
#include <stdarg.h>
#include <string>

#define __DEBUG__
static string GetFileName(const string& path)
{
	char ch = '/';

#ifdef _WIN32
	ch = '\\';
#endif

	size_t pos = path.rfind(ch);
	if (pos == string::npos)
		return path;
	else
		return path.substr(pos + 1);
}
//用于调试追溯的trace log
inline static void __trace_debug(const char* function, const char* filename, int line, char* format, ...)
{
#ifdef __DEBUG__
	//输出调用函数的信息
	fprintf(stdout, "【%s:%d】%s", GetFileName(filename).c_str(), line, function);

	//输出用户打的trace信息
	va_list args;
	va_start(args, format);
	vfprintf(stdout, format, args);
	va_end(args);
#endif
}

#define __TRACE_DEBUG(...)  __trace_debug(__FUNCTION__, __FILE__, __LINE__, __VA_ARGS__);

//-----------------  SimpleAlloc统一封装的内存分配的接口 -------------------------//
template<class T, class Alloc>
class SimpleAlloc
{

public:
	static T* Allocate(size_t n)
	{
		return 0 == n ? 0 : (T*)Alloc::Allocate(n * sizeof (T));
	}

	static T* Allocate(void)
	{
		return (T*)Alloc::Allocate(sizeof (T));
	}

	static void Deallocate(T *p, size_t n)
	{
		if (0 != n)
			Alloc::Deallocate(p, n * sizeof (T));
	}

	static void Deallocate(T *p)
	{
		Alloc::Deallocate(p, sizeof (T));
	}
};

//----------------------** 一级空间配置器（malloc/realloc/free） **----------------//  
//内存分配失败以后处理的句柄handler类型
typedef void(*FUNC_HANDLER)();

template <int inst>
class __MallocAllocTemplate
{
	/////////////////////////////////////////////////////////////////////////////
	// 1. Allocate()直接调用malloc()
	//		Deallocate()直接调用free()
	// 2. 模拟C++的set_new_handler()以处理内存不足的状况
	/////////////////////////////////////////////////////////////////////////////
private:
	//以下函数用来处理内存不足的情况
	// oom  -> out of memory
	static void * OomMalloc(size_t n)
	{
		// 1：分配内存成功，则直接返回
		// 2：若分配失败，则检查是否设置处理的handler，
		// 有则调用以后再分配。不断重复这个过程，直到分配成功为止。
		// 没有设置处理的handler，则直接结束程序。
		//
		void *result;

		for (;;)	//不断尝试释放、配置，再释放、再配置...
		{
			if (0 == __MallocAllocOomHandler)
			{	
				//自定义内存不足处理例程并未被客端设定！
				//throw bad_alloc("分配内存失败");
				cerr << "out of memory" << endl;
				exit(1);
			}
			(*__MallocAllocOomHandler)();	//调用自定义处理例程，企图释放内存
			result = malloc(n);	//再次尝试配置内存
			if (result)
				return(result);
		}
	}
	static void * OomRealloc(void *p, size_t n)
	{
		void *result;

		for (;;)	//不断尝试释放、配置，再释放、再配置...
		{
			if (0 == __MallocAllocOomHandler)
			{
				//自定义内存不足处理例程并未被客端设定！
				//throw bad_alloc("分配内存失败");
				cerr << "out of memory" << endl;
			}
			(*__MallocAllocOomHandler)();		//调用处理例程，企图释放内存
			result = realloc(p, n);		//再次尝试配置内存
			if (result)
				return(result);
		}
	}
	static FUNC_HANDLER __MallocAllocOomHandler;
	//static void(*__MallocAllocOomHandler)();

public:
	static void * Allocate(size_t n)
	{
		//第一级配置器直接使用malloc
		void *result = malloc(n);
		//无法满足需求时调用OomMalloc()
		if (0 == result)
			result = OomMalloc(n);

		__TRACE_DEBUG("  分配成功！\n\n");
		return result;
	}

	static void Deallocate(void *p, size_t /* n */)
	{	//第一级配置器直接使用free()
		free(p);
	}

	static void * Reallocate(void *p, size_t /* old_sz */, size_t new_sz)
	{
		//第一级配置器直接使用realloc()
		void * result = realloc(p, new_sz);
		//无法满足需求时，改用OomRealloc()
		if (0 == result) result = OomRealloc(p, new_sz);
		return result;
	}

	//模拟C++的set_new_handler()以处理内存不足的状况
	//使用自定义函数句柄来处理内存不足的情况
	//static void(*SetMallocHandler(void(*f)()))()
	static FUNC_HANDLER SetMallocHandler(FUNC_HANDLER f)
	{
		//void(*old)() = __MallocAllocOomHandler;
		FUNC_HANDLER old = __MallocAllocOomHandler;
		__MallocAllocOomHandler = f;
		return(old);
	}

};

//分配内存失败处理函数的句柄函数指针
//初值为0，有待客端设定
template <int inst>
//void(*__MallocAllocTemplate<inst>::__MallocAllocOomHandler)() = 0;
FUNC_HANDLER __MallocAllocTemplate<inst>::__MallocAllocOomHandler = 0;

//直接将参数inst指定为0
typedef __MallocAllocTemplate<0> MallocAlloc;

//------------------------** 二级空间配置器	自由链表   **-------------------------//  
#ifdef __USE_MALLOC
typedef MallocAlloc Alloc;

#else

template <bool threads, int inst>
class __DefaultAllocTemplate
{
	/////////////////////////////////////////////////////////////////////////////
	// 1. 维护16个自由链表 _freeList
	//		负责16种小型区块的次配置能力.
	//		内存池 memory pool 以malloc()配置而得。
	//		如果内存不足，转调用第一级配置器（那儿有处理程序）
	// 2. 如果需求大于128bytes，就转调用第一级配置器。
	/////////////////////////////////////////////////////////////////////////////

	//--------------------| 数据成员 |-------------------------//
private:
	enum { __ALIGN = 8 };		// 排列基准值（排列间隔是8）
	enum { __MAX_BYTES = 128 };		// 最大值  自由链表最大的内存块
	enum { __NFREELISTS = __MAX_BYTES / __ALIGN };	// 排列链大小 0-15 共16个

	// free-lists的节点构造
	union Obj
	{
		union Obj* _freeListLink;	// 指向下一个内存块的指针
		char _clientData[1];
	};
	// 自由链表 16个free-lists
	static Obj* volatile _freeList[__NFREELISTS];	

	//内存池
	static char* _startFree;		// 内存池水位线开始，只在ChunkAlloc()中变化
	static char* _endFree;		// 内存池水位线结束，只在ChunkAlloc()中变化
	static size_t _heapSize;	// 从系统堆分配的总大小

public:
	//--------------------| 成员函数 |-------------------------//

	//将bytes上调至8的倍数
	// bytes == 9
	// bytes == 8
	// bytes == 7
	static size_t ROUND_UP(size_t bytes)
	{
		return (((bytes)+__ALIGN - 1) & ~(__ALIGN - 1));
	}

	//寻找字节数在自由链表所在数组的下标
	//即决定使用第n号free-list，n从0算起
	// bytes == 9
	// bytes == 8
	// bytes == 7
	static  size_t FREELIST_INDEX(size_t bytes)
	{ 
		return (((bytes)+__ALIGN - 1) / __ALIGN - 1);
	}

	// 重新填充 free list ，新的空间取自内存池（经由ChunkAlloc()完成）
	// 缺省取得20个新区块，但万一内存池空间不足，获得的区块数可能会少于20 
	// 返回一个大小为n的对象，并可能会为适当的 freeList 添加节点
	static void* Refill(size_t n)
	{
		// 尝试使用ChunkAlloc取得nobjs个区块作为free list的新节点
		// 分配20个n bytes的内存
		// 如果不够则能分配多少分配多少
		//
		size_t nobjs = 20;
		char* chunk = ChunkAlloc(n, nobjs);

		__TRACE_DEBUG(" 内存池分配对象：<n:%d，nobjs:%d>\n", n, nobjs);
		
		// 如果只分配到一块，则这个区块就分配给调用者使用，free list无新节点
		if (1 == nobjs)
			return chunk;
		// 否则准备调整 free list，纳入新节点
		size_t index = FREELIST_INDEX(n);

		// 将剩余的块挂到自由链表
		Obj* cur = (Obj*)(chunk + n);
		_freeList[index] = cur;
		// 以下将 free list 的各节点串接起来
		// 从1开始，因为第0个将返回给客端
		for (size_t i = 1; i < nobjs; ++i)
		{
			Obj* next = (Obj*)(chunk + n*i);
			cur->_freeListLink = next;
			cur = next;
		}

		cur->_freeListLink = NULL;
		//cout << endl;
		return chunk;
		
	}

	///************************* 重要 **************************//
	//配置一大块空间，可容纳nobjs个大小为size的区块
	//如果配置nobjs个区块有所不便，nobjs在函数内部可能被降低
	static char* ChunkAlloc(size_t size, size_t& nobjs)
	{
		__TRACE_DEBUG(" 计划从内存池取%d个%u bytes大小的区块（%d bytes）\n",nobjs,size,nobjs*size);

		char* result;
		size_t bytesNeed = size*nobjs;
		size_t bytesLeft = _endFree - _startFree;	   //内存池中的剩余空间

		//
		// 1.内存池中的内存足够，bytesLeft>=bytesNeed，则直接从内存池中取。
		// 2.内存池中的内存不足，但是够一个bytesLeft >= size，则直接取能够取出来。
		// 3.内存池中的内存不足，则从系统堆分配大块内存到内存池中。
		//
		if (bytesLeft >= bytesNeed)
		{	//内存池剩余空间完全满足需求量 
			__TRACE_DEBUG("  内存池中内存足够分配%d个对象\n", nobjs);

			result = _startFree;
			_startFree += bytesNeed;
		}
		else if (bytesLeft >= size)
		{	//内存池剩余空间不能完全满足需求量，但足够供应一个（含)以上的区块
			__TRACE_DEBUG(" 内存池中内存不够分配%d个对象，只能分配%d个对象\n", nobjs, bytesLeft / size);
			result = _startFree;
			nobjs = bytesLeft / size;
			_startFree += nobjs*size;
		}
		else
		{
			////////////////////////////////////////////////////////////////////////////////////////////////////////
			//	内存池连一块区域空间都无法供应，此时需利用malloc()从heap中配置2 * bytesNeed + n大小内存。
			//	其中一个交出，另19个交给free list维护，余下的留给内存池。
			//	如果malloc()失败，就去free list中寻找未使用且足够大的free list，找到就挖一块交出，找不到就调用第一级配置器。
			//	第一级配置器也是使用malloc()来配置内存，但它有自定义的处理例程，或许有机会释放其他的内存拿来此处使用。
			//	如果可以就成功，否则发出bad_alloc异常。
			////////////////////////////////////////////////////////////////////////////////////////////////////////////

			if (bytesLeft > 0)
			{// 原本内存池中还有小块剩余内存，则将它头插到合适的自由链表
				size_t index = FREELIST_INDEX(bytesLeft);
				((Obj*)_startFree)->_freeListLink = _freeList[index];
				_freeList[index] = (Obj*)_startFree;
				_startFree = NULL;

				__TRACE_DEBUG("  将内存池中剩余的空间，分配给freeList[%d]\n", index);
			}
			//新水量的大小为需求量的两倍，再加上一个随着配置次数增加愈来愈大的附加量。
			// 从系统堆分配两倍+已分配的heapSize/8的内存到内存池中。
			size_t bytesToGet = 2 * bytesNeed + ROUND_UP(_heapSize >> 4);
			
			//配置heap空间，用来补充内存池
			_startFree = (char*)malloc(bytesToGet);
			__TRACE_DEBUG("  内存池空间不足，从系统堆分配两倍需求+附加量=%u bytes内存\n", bytesToGet);

			// 【无奈之举】
			// 如果在系统堆中内存分配失败，则尝试到自由链表中更大的节点中分配
			//
			if (_startFree == NULL)
			{//heap空间不足，malloc()失败
				__TRACE_DEBUG("  系统堆已无足够，无奈之下，只能到自由链表中看看\n");

				for (int i = size; i <= __MAX_BYTES; i += __ALIGN)
				{
					Obj* head = _freeList[FREELIST_INDEX(size)];
					if (head)
					{//free list 内尚有未用且足够大的区块
						//头删 释放出该未用区块
						_startFree = (char*)head;
						_freeList[FREELIST_INDEX(size)] = head->_freeListLink;
						_endFree = _startFree + i;
						//递归调用自己，为了修正 nobjs
						return ChunkAlloc(size, nobjs);
						//任何残余零头终将被编入适当的free list中备用
					}
				}

				///////////////////////////////////////////////////////////
				// 【最后一根稻草】
				// 自由链表中也没有分配到内存，则再到一级配置器中分配内存，
				// 一级配置器中可能有设置的处理内存，或许能分配到内存。
				
				__TRACE_DEBUG("  系统堆和自由链表都已无内存，一级配置器做最后一根稻草\n");
				_startFree = (char*)MallocAlloc::Allocate(bytesToGet);
			}

			// 从系统堆分配的总字节数。（可用于下次分配时进行调节）
			_heapSize += bytesToGet;
			_endFree = _startFree + bytesToGet;

			// 递归调用自己，为了修正nobjs
			return ChunkAlloc(size, nobjs);
		}

		return result;
	}

	//空间配置函数
	/* n must be > 0 */
	static void* Allocate(size_t n)
	{
		//cout << endl;
		__TRACE_DEBUG("  开始分配内存块对象：n=%d bytes\n", n);
		//
		// 若 n > __MAX_BYTES则直接在一级配置器中获取
		// 否则在二级配置器中获取
		// 
		if (n > 128)
		{
			return MallocAlloc::Allocate(n);
		}

		// 计算需要的内存块在自由链表中的位置，寻找16个free list中适当的一个
		void* ret = 0;
		size_t index = FREELIST_INDEX(n);
		size_t byte = ROUND_UP(n);
		//
		// 1.如果自由链表中没有内存则通过Refill进行填充
		// 2.如果自由链表中有则直接返回一个节点块内存
		// ps:多线程环境需要考虑加锁
		//
		if (_freeList[index] == 0)
		{
			// 没找到可用的free list，找内存池进行分配填充
			ret = Refill(byte);
		}
		else
		{
			// 头删，返回头内存块
			ret = _freeList[index];
			_freeList[index] = ((Obj*)ret)->_freeListLink;
		}
		__TRACE_DEBUG(" <%d bytes> 分配成功！\n\n",byte);
		return ret;
	}

	/* p 不可以是 0 */
	static void Deallocate(void* p, size_t n)
	{
		__TRACE_DEBUG("  释放内存块对象：%p, %d\n", p, n);
		//大于128则直接归还给一级配置器
		if (n > 128)
		{
			MallocAlloc::Deallocate(p, n);
			return;
		}
		//寻找16个free list中适当的一个
		size_t index = FREELIST_INDEX(n);
		
		// ps:多线程环境需要考虑加锁
		// acquire lock
		//lock lock_instance;
	
		// 头插回自由链表
		((Obj*)p)->_freeListLink = _freeList[index];
		_freeList[index] = (Obj*)p;
	
		// lock is released here
	}

	static void * Reallocate(void *p, size_t old_sz, size_t new_sz);

};
//以下全局静态对象的定义与初值设定
template <bool threads, int inst>
char* __DefaultAllocTemplate<threads, inst>::_startFree = 0;

template <bool threads, int inst>
char* __DefaultAllocTemplate<threads, inst>::_endFree = 0;

template <bool threads, int inst>
size_t __DefaultAllocTemplate<threads, inst>::_heapSize = 0;

template <bool threads, int inst>
typename __DefaultAllocTemplate<threads, inst>::Obj* volatile __DefaultAllocTemplate<threads, inst>::_freeList[__DefaultAllocTemplate<threads, inst>::__NFREELISTS];

typedef __DefaultAllocTemplate<1, 0> Alloc;

#endif //__USE_MALLOC


//测试内存池的一级、二级配置器功能
void TestAllocate1()
{
	//测试调用一级配置器分配内存
	cout << "	测试调用一级配置器分配内存" << endl;
	char*p1 = SimpleAlloc<char, Alloc>::Allocate(129);
	SimpleAlloc<char, Alloc>::Deallocate(p1, 129);

	//测试调用二级配置器分配内存
	cout << "	测试调用二级配置器分配内存" << endl;
	char*p2 = SimpleAlloc<char, Alloc>::Allocate(128);
	char*p3 = SimpleAlloc<char, Alloc>::Allocate(128);
	char*p4 = SimpleAlloc<char, Alloc>::Allocate(128);
	char*p5 = SimpleAlloc<char, Alloc>::Allocate(128);
	SimpleAlloc<char, Alloc>::Deallocate(p2, 128);
	SimpleAlloc<char, Alloc>::Deallocate(p3, 128);
	SimpleAlloc<char, Alloc>::Deallocate(p4, 128);
	SimpleAlloc<char, Alloc>::Deallocate(p5, 128);

	for (int i = 0; i< 21; ++i)
	{
		printf("测试第%d次分配\n", i + 1);
		char*p = SimpleAlloc<char, Alloc>::Allocate(128);
	}
}

// 白盒测试
//测试特殊场景
void TestAllocate2()
{
	cout << "测试内存池空间不足，从系统堆进行分配" << endl;
	// 8*20->8*2->320
	char*p1 = SimpleAlloc<char, Alloc>::Allocate(8);
	char*p2 = SimpleAlloc<char, Alloc>::Allocate(8);

	cout << "测试内存池空间不足，仅够分配不足20个区块的情况" << endl;
	// 16*20
	char*p3 = SimpleAlloc<char, Alloc>::Allocate(12);
}

//测试系统堆内存耗尽的场景
void TestAllocate3()
{
	cout << "测试系统堆内存耗尽" << endl;

	SimpleAlloc<char, Alloc>::Allocate(1024 * 1024 * 1024);
	//SimpleAlloc<char, Alloc>::Allocate(1024*1024*1024);
	SimpleAlloc<char, Alloc>::Allocate(1024 * 1024);

	//不好测试，说明系统管理小块内存的能力还是很强的。
	for (int i = 0; i< 100000; ++i)
	{
		char*p1 = SimpleAlloc<char, Alloc>::Allocate(128);
	}
}