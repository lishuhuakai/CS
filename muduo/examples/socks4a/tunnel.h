#ifndef MUDUO_EXAMPLES_SOCKS4A_TUNNEL_H
#define MUDUO_EXAMPLES_SOCKS4A_TUNNEL_H

#include <muduo/base/Logging.h>
#include <muduo/net/EventLoop.h>
#include <muduo/net/InetAddress.h>
#include <muduo/net/TcpClient.h>
#include <muduo/net/TcpServer.h>

#include <boost/bind.hpp>
#include <boost/enable_shared_from_this.hpp>

class Tunnel : public boost::enable_shared_from_this<Tunnel>,
	boost::noncopyable
{ // 好吧，让我好好瞧一瞧到底什么是Tunnel吧！
public:
	Tunnel(muduo::net::EventLoop* loop,
		const muduo::net::InetAddress& serverAddr, // 服务端的地址
		const muduo::net::TcpConnectionPtr& serverConn) // 居然是一个连接
		: client_(loop, serverAddr, serverConn->name()), // 好吧，作为一个客户端去连接服务端吧！
		serverConn_(serverConn)
	{
		LOG_INFO << "Tunnel " << serverConn->peerAddress().toIpPort()
			<< " <-> " << serverAddr.toIpPort();
	}

	~Tunnel()
	{
		LOG_INFO << "~Tunnel";
	}

	void setup()
	{
		client_.setConnectionCallback(
			boost::bind(&Tunnel::onClientConnection, shared_from_this(), _1));
		client_.setMessageCallback(
			boost::bind(&Tunnel::onClientMessage, shared_from_this(), _1, _2, _3));
		serverConn_->setHighWaterMarkCallback(
			boost::bind(&Tunnel::onHighWaterMarkWeak, boost::weak_ptr<Tunnel>(shared_from_this()), _1, _2),
			10 * 1024 * 1024); // 这个到底是什么玩意？HighWaterMarkWeak高水位
	}

	void teardown()
	{
		client_.setConnectionCallback(muduo::net::defaultConnectionCallback); // 默认的回调函数
		client_.setMessageCallback(muduo::net::defaultMessageCallback);
		if (serverConn_)
		{
			serverConn_->setContext(boost::any());
			serverConn_->shutdown(); // 关闭连接
		}
	}

	void connect()
	{
		client_.connect(); // 用于连接客户端
	}

	void disconnect()
	{
		client_.disconnect();
		// serverConn_.reset();
	}

	void onClientConnection(const muduo::net::TcpConnectionPtr& conn)
	{ // 一旦用户连接了
		LOG_DEBUG << (conn->connected() ? "UP" : "DOWN");
		if (conn->connected())
		{
			conn->setTcpNoDelay(true); // 好吧，暂时还没有看懂这是个什么意思！
			conn->setHighWaterMarkCallback(
				boost::bind(&Tunnel::onHighWaterMarkWeak, boost::weak_ptr<Tunnel>(shared_from_this()), _1, _2),
				10 * 1024 * 1024); // 高水位触发吗？
			serverConn_->setContext(conn);
			if (serverConn_->inputBuffer()->readableBytes() > 0)
			{
				conn->send(serverConn_->inputBuffer());
			}
		}
		else
		{
			teardown();
		}
	}

	void onClientMessage(const muduo::net::TcpConnectionPtr& conn,
		muduo::net::Buffer* buf,
		muduo::Timestamp)
	{ // 一旦客户端发来了消息
		LOG_DEBUG << conn->name() << " " << buf->readableBytes();
		if (serverConn_)
		{
			serverConn_->send(buf); // 向server端发送
		}
		else
		{
			buf->retrieveAll();
			abort();
		}
	}

	void onHighWaterMark(const muduo::net::TcpConnectionPtr& conn,
		size_t bytesToSent)
	{ // 也就是说，如果发送速度太快的话，需要断开连接，是吧！
		LOG_INFO << "onHighWaterMark " << conn->name()
			<< " bytes " << bytesToSent;
		disconnect();
	}

	static void onHighWaterMarkWeak(const boost::weak_ptr<Tunnel>& wkTunnel,
		const muduo::net::TcpConnectionPtr& conn,
		size_t bytesToSent)
	{ // 这些玩意算什么，我真不知道
		boost::shared_ptr<Tunnel> tunnel = wkTunnel.lock();
		if (tunnel)
		{
			tunnel->onHighWaterMark(conn, bytesToSent);
		}
	}

private:
	muduo::net::TcpClient client_; // 一端指向client
	muduo::net::TcpConnectionPtr serverConn_; // 一端连着server
};
typedef boost::shared_ptr<Tunnel> TunnelPtr;

#endif
