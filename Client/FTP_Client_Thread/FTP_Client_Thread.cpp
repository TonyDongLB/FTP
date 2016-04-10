// FTP_Client_Thread.cpp : 定义控制台应用程序的入口点。
//

#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include "winsock2.h"

#define BUF_SIZE  65535

#pragma comment(lib, "ws2_32.lib")

using namespace std;

int main()
{
	WSADATA wsad;
	SOCKET Server;
	SOCKET Con;
	SOCKADDR_IN servAddr;
	SOCKADDR_IN conAddr;
	char buf[BUF_SIZE] = { 0 };
	int retVal;
	string fileName;

	//初始化套接字动态库
	if (WSAStartup(MAKEWORD(2, 2), &wsad) != 0)
	{
		cout << "初始化套接字出错！" << endl;
		return -1;
	}

	Server = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	Con = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	if (INVALID_SOCKET == Server || INVALID_SOCKET == Con)
	{
		cout << "创建套接字出错！" << endl;
		WSACleanup();
		return -2;
	}

	//设置服务器地址
	servAddr.sin_family = AF_INET;
	servAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	servAddr.sin_port = htons((short)20509);
	int nServAddlen = sizeof(servAddr);

	conAddr.sin_family = AF_INET;
	conAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	conAddr.sin_port = htons((short)10509);
	int lenConAddr = sizeof(conAddr);

	retVal = sendto(Con, "1", sizeof("1"), 0, (sockaddr*)&conAddr, lenConAddr);
	if ( retVal = recvfrom(Con, buf, BUF_SIZE, 0, (sockaddr*)&conAddr, &lenConAddr) <= 0 )
	{
		cout << "获取文件目录失败！" << endl;
		return -1;
	}

	string fileNames = buf;
	vector<string> name_list; // 存放分割后的字符串
	int comma_n = 0;
	do
	{
		string tmp_s = "";
		comma_n = fileNames.find('\n');

		if (-1 == comma_n)
		{
			tmp_s = fileNames.substr(0, fileNames.length());
			name_list.push_back(tmp_s);
			break;
		}

		tmp_s = fileNames.substr(0, comma_n);
		fileNames.erase(0, comma_n + 1);
		name_list.push_back(tmp_s);
	} while (true);

	for (unsigned i = 0; i < name_list.size(); i++)
	{
		cout << name_list[i] << endl;
	}


		//连接服务器
		retVal = connect(Server, (LPSOCKADDR)&servAddr, sizeof(servAddr));
		if (SOCKET_ERROR == retVal)
		{
			cout << "连接服务器失败！" << endl;
			getchar();
			closesocket(Server);
			WSACleanup();
			return -3;
		}

		cout << "清输入要下载的文件：";
		bool flag = true;
		while (flag) 
		{
			cin >> fileName;

			for (unsigned i = 0; i < name_list.size(); i++)
			{
				if (name_list[i] == fileName)
				{
					flag = false;
					break;
				}
				if (i == name_list.size() - 1)
				{
					cout << "输入文件名有错！请重新输入：";
				}
			}
		}
		char c_fileName[256];
		strcpy(c_fileName, fileName.c_str());
		retVal = send(Server, c_fileName, strlen(c_fileName), 0);
		if (SOCKET_ERROR == retVal)
		{
			cout << "发送文件名失败！" << endl;
			return -4;
		}
		else
		{
			cout << "发送成功，等待下载！" << endl;
		}
		ofstream out(fileName, fstream::out | fstream::binary);
		memset(buf, 0, BUF_SIZE);
		retVal = 1;
		while ((retVal = recv(Server, buf, BUF_SIZE, 0)) > 0)
		{
			cout << retVal << endl;
			out.write(buf, retVal);
			memset(buf, 0, BUF_SIZE);
		}
		out.close();
		cout << "文件接受完毕！" << endl;

		getchar();
}

