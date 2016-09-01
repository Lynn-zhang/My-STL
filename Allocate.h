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
//���ڵ���׷�ݵ�trace log
inline static void __trace_debug(const char* function, const char* filename, int line, char* format, ...)
{
#ifdef __DEBUG__
	//������ú�������Ϣ
	fprintf(stdout, "��%s:%d��%s", GetFileName(filename).c_str(), line, function);

	//����û����trace��Ϣ
	va_list args;
	va_start(args, format);
	vfprintf(stdout, format, args);
	va_end(args);
#endif
}

#define __TRACE_DEBUG(...)  __trace_debug(__FUNCTION__, __FILE__, __LINE__, __VA_ARGS__);

//-----------------  SimpleAllocͳһ��װ���ڴ����Ľӿ� -------------------------//
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

//----------------------** һ���ռ���������malloc/realloc/free�� **----------------//  
//�ڴ����ʧ���Ժ���ľ��handler����
typedef void(*FUNC_HANDLER)();

template <int inst>
class __MallocAllocTemplate
{
	/////////////////////////////////////////////////////////////////////////////
	// 1. Allocate()ֱ�ӵ���malloc()
	//		Deallocate()ֱ�ӵ���free()
	// 2. ģ��C++��set_new_handler()�Դ����ڴ治���״��
	/////////////////////////////////////////////////////////////////////////////
private:
	//���º������������ڴ治������
	// oom  -> out of memory
	static void * OomMalloc(size_t n)
	{
		// 1�������ڴ�ɹ�����ֱ�ӷ���
		// 2��������ʧ�ܣ������Ƿ����ô����handler��
		// ��������Ժ��ٷ��䡣�����ظ�������̣�ֱ������ɹ�Ϊֹ��
		// û�����ô����handler����ֱ�ӽ�������
		//
		void *result;

		for (;;)	//���ϳ����ͷš����ã����ͷš�������...
		{
			if (0 == __MallocAllocOomHandler)
			{	
				//�Զ����ڴ治�㴦�����̲�δ���Ͷ��趨��
				//throw bad_alloc("�����ڴ�ʧ��");
				cerr << "out of memory" << endl;
				exit(1);
			}
			(*__MallocAllocOomHandler)();	//�����Զ��崦�����̣���ͼ�ͷ��ڴ�
			result = malloc(n);	//�ٴγ��������ڴ�
			if (result)
				return(result);
		}
	}
	static void * OomRealloc(void *p, size_t n)
	{
		void *result;

		for (;;)	//���ϳ����ͷš����ã����ͷš�������...
		{
			if (0 == __MallocAllocOomHandler)
			{
				//�Զ����ڴ治�㴦�����̲�δ���Ͷ��趨��
				//throw bad_alloc("�����ڴ�ʧ��");
				cerr << "out of memory" << endl;
			}
			(*__MallocAllocOomHandler)();		//���ô������̣���ͼ�ͷ��ڴ�
			result = realloc(p, n);		//�ٴγ��������ڴ�
			if (result)
				return(result);
		}
	}
	static FUNC_HANDLER __MallocAllocOomHandler;
	//static void(*__MallocAllocOomHandler)();

public:
	static void * Allocate(size_t n)
	{
		//��һ��������ֱ��ʹ��malloc
		void *result = malloc(n);
		//�޷���������ʱ����OomMalloc()
		if (0 == result)
			result = OomMalloc(n);

		__TRACE_DEBUG("  ����ɹ���\n\n");
		return result;
	}

	static void Deallocate(void *p, size_t /* n */)
	{	//��һ��������ֱ��ʹ��free()
		free(p);
	}

	static void * Reallocate(void *p, size_t /* old_sz */, size_t new_sz)
	{
		//��һ��������ֱ��ʹ��realloc()
		void * result = realloc(p, new_sz);
		//�޷���������ʱ������OomRealloc()
		if (0 == result) result = OomRealloc(p, new_sz);
		return result;
	}

	//ģ��C++��set_new_handler()�Դ����ڴ治���״��
	//ʹ���Զ��庯������������ڴ治������
	//static void(*SetMallocHandler(void(*f)()))()
	static FUNC_HANDLER SetMallocHandler(FUNC_HANDLER f)
	{
		//void(*old)() = __MallocAllocOomHandler;
		FUNC_HANDLER old = __MallocAllocOomHandler;
		__MallocAllocOomHandler = f;
		return(old);
	}

};

//�����ڴ�ʧ�ܴ������ľ������ָ��
//��ֵΪ0���д��Ͷ��趨
template <int inst>
//void(*__MallocAllocTemplate<inst>::__MallocAllocOomHandler)() = 0;
FUNC_HANDLER __MallocAllocTemplate<inst>::__MallocAllocOomHandler = 0;

//ֱ�ӽ�����instָ��Ϊ0
typedef __MallocAllocTemplate<0> MallocAlloc;

//------------------------** �����ռ�������	��������   **-------------------------//  
#ifdef __USE_MALLOC
typedef MallocAlloc Alloc;

#else

template <bool threads, int inst>
class __DefaultAllocTemplate
{
	/////////////////////////////////////////////////////////////////////////////
	// 1. ά��16���������� _freeList
	//		����16��С������Ĵ���������.
	//		�ڴ�� memory pool ��malloc()���ö��á�
	//		����ڴ治�㣬ת���õ�һ�����������Ƕ��д������
	// 2. ����������128bytes����ת���õ�һ����������
	/////////////////////////////////////////////////////////////////////////////

	//--------------------| ���ݳ�Ա |-------------------------//
private:
	enum { __ALIGN = 8 };		// ���л�׼ֵ�����м����8��
	enum { __MAX_BYTES = 128 };		// ���ֵ  �������������ڴ��
	enum { __NFREELISTS = __MAX_BYTES / __ALIGN };	// ��������С 0-15 ��16��

	// free-lists�Ľڵ㹹��
	union Obj
	{
		union Obj* _freeListLink;	// ָ����һ���ڴ���ָ��
		char _clientData[1];
	};
	// �������� 16��free-lists
	static Obj* volatile _freeList[__NFREELISTS];	

	//�ڴ��
	static char* _startFree;		// �ڴ��ˮλ�߿�ʼ��ֻ��ChunkAlloc()�б仯
	static char* _endFree;		// �ڴ��ˮλ�߽�����ֻ��ChunkAlloc()�б仯
	static size_t _heapSize;	// ��ϵͳ�ѷ�����ܴ�С

public:
	//--------------------| ��Ա���� |-------------------------//

	//��bytes�ϵ���8�ı���
	// bytes == 9
	// bytes == 8
	// bytes == 7
	static size_t ROUND_UP(size_t bytes)
	{
		return (((bytes)+__ALIGN - 1) & ~(__ALIGN - 1));
	}

	//Ѱ���ֽ�����������������������±�
	//������ʹ�õ�n��free-list��n��0����
	// bytes == 9
	// bytes == 8
	// bytes == 7
	static  size_t FREELIST_INDEX(size_t bytes)
	{ 
		return (((bytes)+__ALIGN - 1) / __ALIGN - 1);
	}

	// ������� free list ���µĿռ�ȡ���ڴ�أ�����ChunkAlloc()��ɣ�
	// ȱʡȡ��20�������飬����һ�ڴ�ؿռ䲻�㣬��õ����������ܻ�����20 
	// ����һ����СΪn�Ķ��󣬲����ܻ�Ϊ�ʵ��� freeList ��ӽڵ�
	static void* Refill(size_t n)
	{
		// ����ʹ��ChunkAllocȡ��nobjs��������Ϊfree list���½ڵ�
		// ����20��n bytes���ڴ�
		// ����������ܷ�����ٷ������
		//
		size_t nobjs = 20;
		char* chunk = ChunkAlloc(n, nobjs);

		__TRACE_DEBUG(" �ڴ�ط������<n:%d��nobjs:%d>\n", n, nobjs);
		
		// ���ֻ���䵽һ�飬���������ͷ����������ʹ�ã�free list���½ڵ�
		if (1 == nobjs)
			return chunk;
		// ����׼������ free list�������½ڵ�
		size_t index = FREELIST_INDEX(n);

		// ��ʣ��Ŀ�ҵ���������
		Obj* cur = (Obj*)(chunk + n);
		_freeList[index] = cur;
		// ���½� free list �ĸ��ڵ㴮������
		// ��1��ʼ����Ϊ��0�������ظ��Ͷ�
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

	///************************* ��Ҫ **************************//
	//����һ���ռ䣬������nobjs����СΪsize������
	//�������nobjs�������������㣬nobjs�ں����ڲ����ܱ�����
	static char* ChunkAlloc(size_t size, size_t& nobjs)
	{
		__TRACE_DEBUG(" �ƻ����ڴ��ȡ%d��%u bytes��С�����飨%d bytes��\n",nobjs,size,nobjs*size);

		char* result;
		size_t bytesNeed = size*nobjs;
		size_t bytesLeft = _endFree - _startFree;	   //�ڴ���е�ʣ��ռ�

		//
		// 1.�ڴ���е��ڴ��㹻��bytesLeft>=bytesNeed����ֱ�Ӵ��ڴ����ȡ��
		// 2.�ڴ���е��ڴ治�㣬���ǹ�һ��bytesLeft >= size����ֱ��ȡ�ܹ�ȡ������
		// 3.�ڴ���е��ڴ治�㣬���ϵͳ�ѷ������ڴ浽�ڴ���С�
		//
		if (bytesLeft >= bytesNeed)
		{	//�ڴ��ʣ��ռ���ȫ���������� 
			__TRACE_DEBUG("  �ڴ�����ڴ��㹻����%d������\n", nobjs);

			result = _startFree;
			_startFree += bytesNeed;
		}
		else if (bytesLeft >= size)
		{	//�ڴ��ʣ��ռ䲻����ȫ���������������㹻��Ӧһ������)���ϵ�����
			__TRACE_DEBUG(" �ڴ�����ڴ治������%d������ֻ�ܷ���%d������\n", nobjs, bytesLeft / size);
			result = _startFree;
			nobjs = bytesLeft / size;
			_startFree += nobjs*size;
		}
		else
		{
			////////////////////////////////////////////////////////////////////////////////////////////////////////
			//	�ڴ����һ������ռ䶼�޷���Ӧ����ʱ������malloc()��heap������2 * bytesNeed + n��С�ڴ档
			//	����һ����������19������free listά�������µ������ڴ�ء�
			//	���malloc()ʧ�ܣ���ȥfree list��Ѱ��δʹ�����㹻���free list���ҵ�����һ�齻�����Ҳ����͵��õ�һ����������
			//	��һ��������Ҳ��ʹ��malloc()�������ڴ棬�������Զ���Ĵ������̣������л����ͷ��������ڴ������˴�ʹ�á�
			//	������Ծͳɹ������򷢳�bad_alloc�쳣��
			////////////////////////////////////////////////////////////////////////////////////////////////////////////

			if (bytesLeft > 0)
			{// ԭ���ڴ���л���С��ʣ���ڴ棬����ͷ�嵽���ʵ���������
				size_t index = FREELIST_INDEX(bytesLeft);
				((Obj*)_startFree)->_freeListLink = _freeList[index];
				_freeList[index] = (Obj*)_startFree;
				_startFree = NULL;

				__TRACE_DEBUG("  ���ڴ����ʣ��Ŀռ䣬�����freeList[%d]\n", index);
			}
			//��ˮ���Ĵ�СΪ���������������ټ���һ���������ô���������������ĸ�������
			// ��ϵͳ�ѷ�������+�ѷ����heapSize/8���ڴ浽�ڴ���С�
			size_t bytesToGet = 2 * bytesNeed + ROUND_UP(_heapSize >> 4);
			
			//����heap�ռ䣬���������ڴ��
			_startFree = (char*)malloc(bytesToGet);
			__TRACE_DEBUG("  �ڴ�ؿռ䲻�㣬��ϵͳ�ѷ�����������+������=%u bytes�ڴ�\n", bytesToGet);

			// ������֮�١�
			// �����ϵͳ�����ڴ����ʧ�ܣ����Ե����������и���Ľڵ��з���
			//
			if (_startFree == NULL)
			{//heap�ռ䲻�㣬malloc()ʧ��
				__TRACE_DEBUG("  ϵͳ�������㹻������֮�£�ֻ�ܵ����������п���\n");

				for (int i = size; i <= __MAX_BYTES; i += __ALIGN)
				{
					Obj* head = _freeList[FREELIST_INDEX(size)];
					if (head)
					{//free list ������δ�����㹻�������
						//ͷɾ �ͷų���δ������
						_startFree = (char*)head;
						_freeList[FREELIST_INDEX(size)] = head->_freeListLink;
						_endFree = _startFree + i;
						//�ݹ�����Լ���Ϊ������ nobjs
						return ChunkAlloc(size, nobjs);
						//�κβ�����ͷ�ս��������ʵ���free list�б���
					}
				}

				///////////////////////////////////////////////////////////
				// �����һ�����ݡ�
				// ����������Ҳû�з��䵽�ڴ棬���ٵ�һ���������з����ڴ棬
				// һ���������п��������õĴ����ڴ棬�����ܷ��䵽�ڴ档
				
				__TRACE_DEBUG("  ϵͳ�Ѻ��������������ڴ棬һ�������������һ������\n");
				_startFree = (char*)MallocAlloc::Allocate(bytesToGet);
			}

			// ��ϵͳ�ѷ�������ֽ��������������´η���ʱ���е��ڣ�
			_heapSize += bytesToGet;
			_endFree = _startFree + bytesToGet;

			// �ݹ�����Լ���Ϊ������nobjs
			return ChunkAlloc(size, nobjs);
		}

		return result;
	}

	//�ռ����ú���
	/* n must be > 0 */
	static void* Allocate(size_t n)
	{
		//cout << endl;
		__TRACE_DEBUG("  ��ʼ�����ڴ�����n=%d bytes\n", n);
		//
		// �� n > __MAX_BYTES��ֱ����һ���������л�ȡ
		// �����ڶ����������л�ȡ
		// 
		if (n > 128)
		{
			return MallocAlloc::Allocate(n);
		}

		// ������Ҫ���ڴ�������������е�λ�ã�Ѱ��16��free list���ʵ���һ��
		void* ret = 0;
		size_t index = FREELIST_INDEX(n);
		size_t byte = ROUND_UP(n);
		//
		// 1.�������������û���ڴ���ͨ��Refill�������
		// 2.�����������������ֱ�ӷ���һ���ڵ���ڴ�
		// ps:���̻߳�����Ҫ���Ǽ���
		//
		if (_freeList[index] == 0)
		{
			// û�ҵ����õ�free list�����ڴ�ؽ��з������
			ret = Refill(byte);
		}
		else
		{
			// ͷɾ������ͷ�ڴ��
			ret = _freeList[index];
			_freeList[index] = ((Obj*)ret)->_freeListLink;
		}
		__TRACE_DEBUG(" <%d bytes> ����ɹ���\n\n",byte);
		return ret;
	}

	/* p �������� 0 */
	static void Deallocate(void* p, size_t n)
	{
		__TRACE_DEBUG("  �ͷ��ڴ�����%p, %d\n", p, n);
		//����128��ֱ�ӹ黹��һ��������
		if (n > 128)
		{
			MallocAlloc::Deallocate(p, n);
			return;
		}
		//Ѱ��16��free list���ʵ���һ��
		size_t index = FREELIST_INDEX(n);
		
		// ps:���̻߳�����Ҫ���Ǽ���
		// acquire lock
		//lock lock_instance;
	
		// ͷ�����������
		((Obj*)p)->_freeListLink = _freeList[index];
		_freeList[index] = (Obj*)p;
	
		// lock is released here
	}

	static void * Reallocate(void *p, size_t old_sz, size_t new_sz);

};
//����ȫ�־�̬����Ķ������ֵ�趨
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


//�����ڴ�ص�һ������������������
void TestAllocate1()
{
	//���Ե���һ�������������ڴ�
	cout << "	���Ե���һ�������������ڴ�" << endl;
	char*p1 = SimpleAlloc<char, Alloc>::Allocate(129);
	SimpleAlloc<char, Alloc>::Deallocate(p1, 129);

	//���Ե��ö��������������ڴ�
	cout << "	���Ե��ö��������������ڴ�" << endl;
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
		printf("���Ե�%d�η���\n", i + 1);
		char*p = SimpleAlloc<char, Alloc>::Allocate(128);
	}
}

// �׺в���
//�������ⳡ��
void TestAllocate2()
{
	cout << "�����ڴ�ؿռ䲻�㣬��ϵͳ�ѽ��з���" << endl;
	// 8*20->8*2->320
	char*p1 = SimpleAlloc<char, Alloc>::Allocate(8);
	char*p2 = SimpleAlloc<char, Alloc>::Allocate(8);

	cout << "�����ڴ�ؿռ䲻�㣬�������䲻��20����������" << endl;
	// 16*20
	char*p3 = SimpleAlloc<char, Alloc>::Allocate(12);
}

//����ϵͳ���ڴ�ľ��ĳ���
void TestAllocate3()
{
	cout << "����ϵͳ���ڴ�ľ�" << endl;

	SimpleAlloc<char, Alloc>::Allocate(1024 * 1024 * 1024);
	//SimpleAlloc<char, Alloc>::Allocate(1024*1024*1024);
	SimpleAlloc<char, Alloc>::Allocate(1024 * 1024);

	//���ò��ԣ�˵��ϵͳ����С���ڴ���������Ǻ�ǿ�ġ�
	for (int i = 0; i< 100000; ++i)
	{
		char*p1 = SimpleAlloc<char, Alloc>::Allocate(128);
	}
}