/**
@file:		PerformanceTimer.hpp
@author:	Robert Clarke
@date:		2019-11-08
@note:		Developed for the INFO-5104 course at Fanshawe College
@brief:		Declaration of the ThreadPool, AdapterThread and parent classes
*/
#pragma once
#include <functional>
#include <thread>
#include <mutex>
#include <atomic>
#include <vector>
#include <queue>

#define THREADPOOL_WIN32_THREADING_ALLOWED false
#if THREADPOOL_WIN32_THREADING_ALLOWED
#include <Windows.h>
#endif

namespace threading {
	using lguard = std::lock_guard<std::mutex>;
	using ulock = std::unique_lock<std::mutex>;

	//Forward declaration of the ThreadPool class
	template<class task_t>
	class ThreadPool;


	/********************************
	 Concrete Thread Implementations
	********************************/

	//Interface for the concrete thread classes
	template <class task_t>
	class ConcreteThreadInterface {
	public:
		using procedure_t = typename ThreadPool<task_t>::procedure_t;
		using threadpool_ptr_t = ThreadPool<task_t>*;
		procedure_t p_func;
		threadpool_ptr_t p_threadPool;

		ConcreteThreadInterface(threadpool_ptr_t tp_ptr, procedure_t procedure) : p_threadPool(tp_ptr), p_func(procedure) {}
		virtual ~ConcreteThreadInterface() {}

		virtual void doJoin() = 0;
	protected:
		//Receive tasks from the threadpool and process them
		void doTaskProcessing() {
			while (!p_threadPool->finishedProcessing) {
				{	ulock lk(p_threadPool->mxConditionVar);
					p_threadPool->cvNotify.wait(lk);
				}
				if (p_threadPool->finishedProcessing)
					break;

				//Use double-check locking to ensure there is a task to be processed
				task_t unprocessedTask;
				bool hasTask = false;
				{	lguard lk(p_threadPool->mxTaskQueue);
					if (!p_threadPool->taskQueue.empty()) {
						unprocessedTask = p_threadPool->taskQueue.front();
						p_threadPool->taskQueue.pop();
						hasTask = true;
					}
				}

				if (hasTask)
					p_func(unprocessedTask);
			}
		}
	};

#if THREADPOOL_WIN32_THREADING_ALLOWED
	//Win32 concrete thread class
	template <class task_t>
	class ConcreteThreadWin32 : public ConcreteThreadInterface<task_t> {
		using threadpool_ptr_t = typename ConcreteThreadInterface<task_t>::threadpool_ptr_t;
		using procedure_t = typename ConcreteThreadInterface<task_t>::procedure_t;
		DWORD threadId;
		HANDLE hThread;

		static DWORD WINAPI win32Wrapper(LPVOID parameter) {
			ConcreteThreadWin32<task_t>* pointerToSelf = (ConcreteThreadWin32<task_t>*)parameter;
			pointerToSelf->doTaskProcessing();
			return 0;
		}
	protected:
		inline void doJoin() override { WaitForSingleObject(hThread, INFINITE); }
	public:
		ConcreteThreadWin32(threadpool_ptr_t tp_ptr, procedure_t procedure) :
			ConcreteThreadInterface<task_t>(tp_ptr, procedure), threadId(0) {
			hThread = CreateThread(NULL, 0, win32Wrapper, this, 0, &threadId);
		}
		~ConcreteThreadWin32() { CloseHandle(hThread); }
	};
#endif

	//C++11 concrete thread
	template <class task_t>
	class ConcreteThreadCPP11 {
		using procedure_t = typename ThreadPool<task_t>::procedure_t;

		std::thread* m_thread;
		ThreadPool<task_t>* p_threadPool;
		procedure_t p_func;

	protected:
		//Receive tasks from the threadpool and process them
		void doTaskProcessing() {
			while (!p_threadPool->finishedProcessing) {
				{	ulock lk(p_threadPool->mxConditionVar);
					p_threadPool->cvNotify.wait(lk);
				}
				if (p_threadPool->finishedProcessing)
					break;

				//Use double-check locking to ensure there is a task to be processed
				task_t unprocessedTask;
				bool hasTask = false;
				{	lguard lk(p_threadPool->mxTaskQueue);
					if (!p_threadPool->taskQueue.empty()) {
						unprocessedTask = p_threadPool->taskQueue.front();
						p_threadPool->taskQueue.pop();
						hasTask = true;
					}
				}
				if (hasTask)
					p_func(unprocessedTask);
			}
		}
		inline void doJoin() { m_thread->join(); }
	public:
		ConcreteThreadCPP11(typename ThreadPool<task_t>::ptr_t tp_ptr, procedure_t procedure) : p_threadPool(tp_ptr), p_func(procedure),
			m_thread(new std::thread(&ConcreteThreadCPP11::doTaskProcessing, this)) {};
		~ConcreteThreadCPP11() { delete m_thread; }
	};


	/****************************************************************************************
	 Adapter Implementations, uses the Class Adapter since C++ supports multiple inheritance
	****************************************************************************************/

	//Abstract target thread for adapters
	class AbstractThread {
	public:
		using ptr_t = AbstractThread*;
		virtual ~AbstractThread() {}
		virtual void join() = 0;
	};

#if THREADPOOL_WIN32_THREADING_ALLOWED
	//Win32 adapter
	template <class task_t>
	class AdapterThreadWin32 : public AbstractThread, private ConcreteThreadWin32<task_t> {
	public:
		using procedure_t = typename ThreadPool<task_t>::procedure_t;
		using threadpool_ptr_t = ThreadPool<task_t>*;
	private:
		threadpool_ptr_t p_threadPool;
		procedure_t p_func;

	public:
		AdapterThreadWin32(threadpool_ptr_t tp_ptr, procedure_t procedure) :
			ConcreteThreadWin32<task_t>(tp_ptr, procedure) {}
		inline virtual void join() override { this->doJoin(); }
	};
#endif

	//C++11 Adapter thread
	template <class task_t>
	class AdapterThreadCPP11 : public AbstractThread, private ConcreteThreadCPP11<task_t> {
	public:
		using procedure_t = typename ThreadPool<task_t>::procedure_t;
		AdapterThreadCPP11(typename ThreadPool<task_t>::ptr_t tp_ptr, procedure_t procedure) :
			ConcreteThreadCPP11<task_t>(tp_ptr, procedure) {}
		inline virtual void join() override { this->doJoin(); }
	};


	/**************************
	 ThreadPool implementation
	**************************/
	template <class task_t>
	class ThreadPool {
	public:
		using ptr_t = ThreadPool<task_t>*;
		using procedure_t = std::function<void(task_t)>;
		using task_queue = std::queue<task_t>;
		using task_queue_ptr = task_queue*;

	protected:
		//thread construction
		std::vector<AbstractThread::ptr_t> threadContainer;
		std::atomic<unsigned> uninitializedThreads;

		//synchronization attributes
		std::mutex mxConditionVar, mxTaskQueue;
		std::condition_variable cvNotify;

		//task distribution to spawned threads
		task_queue taskQueue;
		bool finishedProcessing;

#if THREADPOOL_WIN32_THREADING_ALLOWED
		friend class ConcreteThreadWin32<task_t>;
#endif
		friend class ConcreteThreadCPP11<task_t>;
		friend class ConcreteThreadInterface<task_t>;
	public:
		ThreadPool(procedure_t procedure) {
			using namespace std;
			finishedProcessing = false;

			//Spawn threads equal to the number of logical cores
			uninitializedThreads = thread::hardware_concurrency();
			for (unsigned x = 0; x < uninitializedThreads; ++x)
				threadContainer.push_back(new AdapterThreadCPP11<task_t>(this, procedure));
		}

		~ThreadPool() {
			for (AbstractThread::ptr_t element : threadContainer)
				delete element;
		}

		void enqueue(task_t task) {
			{	lguard lk(mxTaskQueue);
				taskQueue.emplace(task);
			}
			{	ulock lk(mxConditionVar);
				cvNotify.notify_one();
			}
		}

		void wrapup() {
			//finish up any remaining tasks that might be in the taskqueue
			for (;;) {
				{	lguard lk(mxTaskQueue);
					if (taskQueue.empty()) break;
				}
				{	ulock lk(mxConditionVar);
					cvNotify.notify_one();
				}
			}
			finishedProcessing = true; //flip flag
			//let threads continue past the barrier and break out of processing loops
			{	ulock lk(mxConditionVar);
				cvNotify.notify_all();
			}
			//join all threads before handing control back to caller
			for (AbstractThread* element : threadContainer) {
				element->join();
			}
		}
	};

}