#include "codec.h"

using namespace muduo;
using namespace muduo::net;
using namespace pubsub;

ParseResult pubsub::parseMessage(Buffer* buf,
	string* cmd,
	string* topic,
	string* content)
{
	ParseResult result = kError;
	const char* crlf = buf->findCRLF(); // 首先是要找到结尾符
	if (crlf) // 如果结尾符是存在的
	{
		const char* space = std::find(buf->peek(), crlf, ' '); // 找到空格
		if (space != crlf)
		{
			cmd->assign(buf->peek(), space); // 首先是找到命令
			topic->assign(space + 1, crlf); // 找到topic
			if (*cmd == "pub") // pub表示往topic发送消息
			{
				const char* start = crlf + 2;
				crlf = buf->findCRLF(start); 
				if (crlf)
				{
					content->assign(start, crlf); // 得到content
					buf->retrieveUntil(crlf + 2); // 然后丢弃掉这些东西
					result = kSuccess;
				}
				else
				{
					result = kContinue;
				}
			} // if *cmd == "pub"
			else // 命令是sub或者unsub
			{
				buf->retrieveUntil(crlf + 2);
				result = kSuccess;
			} // if space != crlf
		}
		else // 以\r\n开头
		{
			result = kError;
		}
	}
	else// 没有找到结尾符
	{
		result = kContinue;
	}
	return result;
}

