#pragma once
#include <thread>
#include <mutex>
#include <vector>
#include <queue>
#include <atomic>
#include <functional>
#include <condition_variable>
#include <map>
#include <future>
using namespace std;

class ThreadPool
{
public:
	ThreadPool(int min = 4, int max = thread::hardware_concurrency());
	~ThreadPool();

	template<typename F, typename... Args>
	/*future 是C++标准库中的一个模板类，用于表示异步操作的结果。它提供了一种机制，允许你在某个时间点获取异步操作的结果，无论是成功还是失败。
	F(Args...) 这部分表示一个可调用类型 F 和它的参数包 Args...。F 可以是一个函数、lambda表达式、函数对象等。
	Args... 是一个参数包，表示 F 可以接受任意数量和类型的参数。::type 用于访问 result_of 模板推导出的类型。
	result_of<F(Args...)>::type 就是调用 F 类型（带有 Args... 参数）的函数时返回的结果类型。*/
	auto addTask(F&& f, Args&&... args)->future<typename result_of<F(Args...)>::type>
		/*F&& f, Args&&... args: 函数模板参数，f 是要执行的任务函数，args 是该函数的参数。
		->future<typename result_of<F(Args...)>::type> : 返回值是一个future对象，其类型由f函数和args参数共同决定。*/
	{
		using returnType = typename result_of<F(Args...)>::type;
		//使用 result_of<F(Args...)>::type 来获取任务函数 F 执行后的返回类型，并将其命名为 returnType。

		auto task = make_shared<packaged_task<returnType()>>(
			bind(forward<F>(f), forward<Args>(args)...)
		);
		/*使用make_shared创建一个packaged_task对象，该对象包装了f函数及其参数args。
		packaged_task是一个模板类，用于封装任何可以调用的目标（如函数、lambda表达式、绑定表达式或其他函数对象），使得它的返回值或异常能够被异步地获取。*/

		future<returnType> res = task->get_future();
		{
			unique_lock<mutex> lock(m_queueMutex);
			m_tasks.emplace([task]() {(*task)(); });
		}
		m_condition.notify_one();
		return res;
	}

private:
	void manager();
	void worker();
private:
	thread* m_manager;
	map<thread::id, thread> m_workers;
	vector<thread::id> m_ids;
	int m_minThreads;
	int m_maxThreads;
	atomic<bool> m_stop;
	atomic<int> m_curThreads;
	atomic<int> m_idleThreads;
	atomic<int> m_exitNumber;
	queue<function<void()>> m_tasks;
	mutex m_idsMutex;
	mutex m_queueMutex;
	condition_variable m_condition;
};