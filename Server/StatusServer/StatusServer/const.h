#pragma once
#include <boost/beast/http.hpp>
#include <boost/beast.hpp>
#include <boost/asio.hpp>
#include <memory>
#include <iostream>
#include "Singleton.h"
#include <functional>
#include <map>
#include <unordered_map>
#include <json/json.h>  //����json��������
#include <json/value.h> //����json�ڵ�
#include <json/reader.h> //�������ַ���תΪjson�ڵ�Ĺ���
#include <boost/filesystem.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <atomic>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <hiredis.h>
#include <cassert>
namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
namespace net = boost::asio;            // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>


#define MAX_LENGTH  1024*2
//ͷ���ܳ���
#define HEAD_TOTAL_LEN 4
//ͷ��id����
#define HEAD_ID_LEN 2
//ͷ�����ݳ���
#define HEAD_DATA_LEN 2
#define MAX_RECVQUE  10000
#define MAX_SENDQUE 1000


enum ErrorCodes {
	Success = 0,
	Error_Json = 1001,  //Json��������
	RPCFailed = 1002,  //RPC�������
	VerifyExpired = 1003, //��֤�����
	VerifyCodeErr = 1004, //��֤�����
	UserExist = 1005,       //�û��Ѿ�����
	PasswdErr = 1006,    //�������
	EmailNotMatch = 1007,  //���䲻ƥ��
	PasswdUpFailed = 1008,  //��������ʧ��
	PasswdInvalid = 1009,   //�������ʧ��
	TokenInvalid = 1010,   //TokenʧЧ
	UidInvalid = 1011,  //uid��Ч
};

#define CODEPREFIX "code_"

// Defer ����һ�����͵� RAII��Resource Acquisition Is Initialization�����ߣ����������������ʱ�Զ�ִ��ĳЩ���������������������� Go �����е� defer �ؼ��֣������� C++ ��ʵ�����ƵĹ��ܡ�
class Defer {
public:
	// ����һ��lambda���ʽ���ߺ���ָ��
	Defer(std::function<void()> func) : func_(func) {}

	// ����������ִ�д���ĺ���
	~Defer() {
		func_();
	}

private:
	std::function<void()> func_;// std::function<void()> func_���洢һ���ɵ��ö����� lambda ���ʽ����ָ�룩������������ʱִ�С�
};
/*
Defer ��ĺ���˼�������� C++ �� RAII ���ƣ�ȷ���ڶ����뿪������ʱ�Զ�ִ��ĳЩ������������˵��

�� Defer ���󱻴���ʱ�����ᱣ��һ���������� lambda ���ʽ����

�� Defer �����뿪������ʱ�����磬�������ػ�������������������������ᱻ���ã��Ӷ�ִ�б���ĺ�����

*/
#define LOGIN_COUNT  "logincount"

#define USERIPPREFIX  "uip_"
#define USERTOKENPREFIX  "utoken_"
#define IPCOUNTPREFIX  "ipcount_"
#define USER_BASE_INFO "ubaseinfo_"
#define LOGIN_COUNT  "logincount"
