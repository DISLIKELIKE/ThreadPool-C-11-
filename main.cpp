#include<iostream>
#include"ThreadPool.h"


int calc(int x, int y)
{
	int res = x + y;
	/*cout << "res = " << res << endl;*/
	this_thread::sleep_for(chrono::seconds(2));
	return res;
}

int main()
{	
	ThreadPool pool(4);
	vector<future<int>> results;

	for (int i = 0; i < 10; ++i)
	{
		auto func = bind(calc, i, i * 2);
		pool.addTask(func);
		results.emplace_back(pool.addTask(calc, i, i * 2));
	}

	//�ȴ�����ӡ���
	for (auto&& res : results)
	{
		cout << "�̺߳�������ֵ��" << res.get() << endl;
	}
	
	return 0;
}