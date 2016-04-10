// FTP_Client_Thread.cpp : �������̨Ӧ�ó������ڵ㡣
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

	//��ʼ���׽��ֶ�̬��
	if (WSAStartup(MAKEWORD(2, 2), &wsad) != 0)
	{
		cout << "��ʼ���׽��ֳ���" << endl;
		return -1;
	}

	Server = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	Con = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	if (INVALID_SOCKET == Server || INVALID_SOCKET == Con)
	{
		cout << "�����׽��ֳ���" << endl;
		WSACleanup();
		return -2;
	}

	//���÷�������ַ
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
		cout << "��ȡ�ļ�Ŀ¼ʧ�ܣ�" << endl;
		return -1;
	}

	string fileNames = buf;
	vector<string> name_list; // ��ŷָ����ַ���
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


		//���ӷ�����
		retVal = connect(Server, (LPSOCKADDR)&servAddr, sizeof(servAddr));
		if (SOCKET_ERROR == retVal)
		{
			cout << "���ӷ�����ʧ�ܣ�" << endl;
			getchar();
			closesocket(Server);
			WSACleanup();
			return -3;
		}

		cout << "������Ҫ���ص��ļ���";
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
					cout << "�����ļ����д����������룺";
				}
			}
		}
		char c_fileName[256];
		strcpy(c_fileName, fileName.c_str());
		retVal = send(Server, c_fileName, strlen(c_fileName), 0);
		if (SOCKET_ERROR == retVal)
		{
			cout << "�����ļ���ʧ�ܣ�" << endl;
			return -4;
		}
		else
		{
			cout << "���ͳɹ����ȴ����أ�" << endl;
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
		cout << "�ļ�������ϣ�" << endl;

		getchar();
}

