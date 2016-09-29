#include <muduo/base/CurrentThread.h>

#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>

namespace // 一旦扯上了namespace，一般就是全局可见啦！
{
	__thread int x = 0;
}

void print()
{
	printf("pid=%d tid=%d x=%d\n", getpid(), muduo::CurrentThread::tid(), x);
}

int main()
{
	printf("parent %d\n", getpid());
	print();
	x = 1;
	print();
	pid_t p = fork();

	if (p == 0)
	{
		printf("chlid %d\n", getpid());
		// child
		print();
		x = 2;
		print();

		if (fork() == 0)
		{
			printf("grandchlid %d\n", getpid());
			print();
			x = 3;
			print();
		}
	}
	else
	{
	  // parent
		sleep(1);
		print();
	}
}
