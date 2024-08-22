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
	/*future ��C++��׼���е�һ��ģ���࣬���ڱ�ʾ�첽�����Ľ�������ṩ��һ�ֻ��ƣ���������ĳ��ʱ����ȡ�첽�����Ľ���������ǳɹ�����ʧ�ܡ�
	F(Args...) �ⲿ�ֱ�ʾһ���ɵ������� F �����Ĳ����� Args...��F ������һ��������lambda���ʽ����������ȡ�
	Args... ��һ������������ʾ F ���Խ����������������͵Ĳ�����::type ���ڷ��� result_of ģ���Ƶ��������͡�
	result_of<F(Args...)>::type ���ǵ��� F ���ͣ����� Args... �������ĺ���ʱ���صĽ�����͡�*/
	auto addTask(F&& f, Args&&... args)->future<typename result_of<F(Args...)>::type>
		/*F&& f, Args&&... args: ����ģ�������f ��Ҫִ�е���������args �Ǹú����Ĳ�����
		->future<typename result_of<F(Args...)>::type> : ����ֵ��һ��future������������f������args������ͬ������*/
	{
		using returnType = typename result_of<F(Args...)>::type;
		//ʹ�� result_of<F(Args...)>::type ����ȡ������ F ִ�к�ķ������ͣ�����������Ϊ returnType��

		auto task = make_shared<packaged_task<returnType()>>(
			bind(forward<F>(f), forward<Args>(args)...)
		);
		/*ʹ��make_shared����һ��packaged_task���󣬸ö����װ��f�����������args��
		packaged_task��һ��ģ���࣬���ڷ�װ�κο��Ե��õ�Ŀ�꣨�纯����lambda���ʽ���󶨱��ʽ�������������󣩣�ʹ�����ķ���ֵ���쳣�ܹ����첽�ػ�ȡ��*/

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