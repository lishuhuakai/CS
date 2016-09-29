#include "tunnel.h"

#include <muduo/base/ThreadLocal.h>
#include <stdio.h>

using namespace muduo;
using namespace muduo::net;

std::vector<InetAddress> g_backends;
ThreadLocal<std::map<string, TunnelPtr> > t_tunnels;
MutexLock g_mutex;
size_t g_current = 0;

// 这个文件描述的应该是一个中介吧！
// 我们来看一下所谓的balancer究竟干了一些什么吧！
void onServerConnection(const TcpConnectionPtr& conn)
{
	LOG_DEBUG << (conn->connected() ? "UP" : "DOWN");
	std::map<string, TunnelPtr>& tunnels = t_tunnels.value();
	if (conn->connected())
	{
		conn->setTcpNoDelay(true); // setTcpNoDelay究竟是什么意思？
		size_t current = 0;
		{
			MutexLockGuard guard(g_mutex); // 加锁
			current = g_current; // 因为要修改全局的变量g_current
			g_current = (g_current + 1) % g_backends.size();
		}
		// 好吧，至少让我看一下，所谓的tunnel究竟是什么东西吧！
		InetAddress backend = g_backends[current];
		TunnelPtr tunnel(new Tunnel(conn->getLoop(), backend, conn));  // 新构建了一个tunnel，这个tunnel要连接backend所代表的地址
		tunnel->setup();
		tunnel->connect();

		tunnels[conn->name()] = tunnel; // 然后放入一个map里面
	}
	else
	{
		assert(tunnels.find(conn->name()) != tunnels.end());
		tunnels[conn->name()]->disconnect();
		tunnels.erase(conn->name());
	}
}

void onServerMessage(const TcpConnectionPtr& conn, Buffer* buf, Timestamp)
{
	if (!conn->getContext().empty())
	{
		const TcpConnectionPtr& clientConn
			= boost::any_cast<const TcpConnectionPtr&>(conn->getContext());
		clientConn->send(buf); // 向客户端发送该消息
	}
}

int main(int argc, char* argv[])
{
	if (argc < 3)
	{
		fprintf(stderr, "Usage: %s listen_port backend_ip:port [backend_ip:port]\n", argv[0]);
	}
	else
	{
		for (int i = 2; i < argc; ++i)
		{
			string hostport = argv[i]; // 端口的信息
			size_t colon = hostport.find(':');
			if (colon != string::npos)
			{
				string ip = hostport.substr(0, colon); // 服务端的ip地址
				uint16_t port = static_cast<uint16_t>(atoi(hostport.c_str() + colon + 1));
				g_backends.push_back(InetAddress(ip, port));
			}
			else
			{
				fprintf(stderr, "invalid backend address %s\n", argv[i]);
				return 1;
			}
		}

		uint16_t port = static_cast<uint16_t>(atoi(argv[1]));
		InetAddress listenAddr(port);

		EventLoop loop;
		TcpServer server(&loop, listenAddr, "TcpBalancer");
		server.setConnectionCallback(onServerConnection);
		server.setMessageCallback(onServerMessage);
		server.setThreadNum(4); // 好吧，居然动用了4个线程去处理
		server.start();
		loop.loop();
	}
}

