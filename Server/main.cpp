#include <iostream>
#include <thread>
#include <mutex>
#include <fstream>
#include <vector>
#include <string>
#include "winsock2.h"

#define MAX_CON 16  //最大连接数
#define Con_PORT_NUM  10509 //服务器控制端口号
#define Tran_PORT_NUM 20509 //服务器传输端口号
#define BUF_SIZE 65535 //最大缓存大小

using namespace std;

SOCKET fd_A[MAX_CON] = { 0 }; //客户端socket
int connecting = 0; //此时连接数
string path = "H:\\QQMusicCache\\Log\\QQMusicExternal";
string allName;
SOCKET  Server_Con; //控制信息传输socket
SOCKET  Server_Tran; //文件信息传输socket
ifstream fin[MAX_CON];  //文件读取


#pragma comment(lib,"Ws2_32.lib")

//获取文件名
int getFileName(string path, vector<string> & fileName)
{
    WIN32_FIND_DATA FindFileData;
    HANDLE hFind;
    path += "\\*.*";

    hFind = FindFirstFile(path.c_str(), &FindFileData);

    if(hFind == INVALID_HANDLE_VALUE)
    {
        return 0;
    }
    else
    {
        do
        {
            fileName.push_back(FindFileData.cFileName);
        }
        while(FindNextFile(hFind, &FindFileData) != 0);
    }

    // 查找结束
    FindClose(hFind);

    return 0;
}

//发送文件名
int sendFileName()
{
    char c_fileName[1024] = { 0 };
    SOCKADDR_IN Cli;
    int lenCli = sizeof(Cli);
    string fileName;

    while(true)
    {
        int retVal = recvfrom(Server_Con, c_fileName, 1024, 0, (sockaddr *)&Cli, &lenCli);

        if(retVal <= 0)
        {
            continue;
        }

        string fileName = c_fileName;

        memset(c_fileName, 0, 1024);
        strcpy(c_fileName, allName.c_str());
        sendto(Server_Con, c_fileName, 1024, 0, (sockaddr *)&Cli, lenCli);

    }
}

//传输文件
int tranFile()
{
    //监控文件描述符集合
    fd_set fdsr; //读取集合
    fd_set fdsw; //写集合
    //设置timevalue
    timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 0;

    char  buf[BUF_SIZE] = { 0 }; //数据缓冲区
    string fileName[MAX_CON];//客户端要下载的文件名
    SOCKADDR_IN  addrClit;//客户端地址信息
    SOCKET  newClient; //客户端套接字

    while(true)
    {
        FD_ZERO(&fdsr);
        FD_ZERO(&fdsw);
        FD_SET(Server_Tran, &fdsr);

        for(int i = 0; i < MAX_CON; i++)
        {
            if(fd_A[i] != 0)
            {
                FD_SET(fd_A[i], &fdsr);
                FD_SET(fd_A[i], &fdsw);
            }
        }

        //读取select
        int retVal = select(FD_SETSIZE + 1, &fdsr, NULL, NULL, &tv);

        //select函数运行出错
        if(retVal <= 0)
        {
            continue;
        }

        //任何读取或者新连接到来
        if(retVal > 0)
        {
            for(int i = 0; i < MAX_CON; i++)
            {
                if(fd_A[i] == 0)
                {
                    continue;
                }

                if(FD_ISSET((fd_A[i]), &fdsr))
                {
                    retVal = recv(fd_A[i], buf, BUF_SIZE, 0);

                    if(retVal <= 0)
                    {
                        cout << "Client" << i << "关闭连接" << endl;
                        closesocket(fd_A[i]);
                        fd_A[i] = 0;
                        fin[i].close();
                        fileName[i] = "";
                        connecting--;
                    }
                    else
                    {
                        fileName[i] = buf;
                        cout << fileName[i] << endl;
                        fin[i].open(path + "\\" + fileName[i], fstream::in | fstream::binary);

                        if(!fin[i])
                        {
                            cout << "无法打开文件，是否有错" << endl;
                        }

                        memset(buf, 0, BUF_SIZE);
                    }
                }
            }

            if(FD_ISSET(Server_Tran, &fdsr))
            {
                int addrClientlen = sizeof(addrClit);
                newClient = accept(Server_Tran, (sockaddr FAR *)&addrClit, &addrClientlen);

                if(INVALID_SOCKET == newClient)
                {
                    cout << "接受客户端连接请求失败！" << endl;
                }
                else
                {
                    if(connecting == MAX_CON)
                    {
                        cout << "已经达到最大连接数！" << endl;
                        send(newClient, "Con not connected now!", sizeof("Con not connected now!"), 0);
                    }
                    else
                    {
                        for(int i = 0; i < MAX_CON; i++)
                        {
                            if(fd_A[i] == 0)
                            {
                                fd_A[i] = newClient;
                                break;
                            }
                        }

                        connecting++;
                        cout << "New connection arrived" << endl;
                    }
                }
            }
        }

        //写select
        retVal = select(FD_SETSIZE + 1, NULL, &fdsw, NULL, &tv);

        //select函数运行出错
        if(retVal < 0)
        {
            continue;
        }

        //有可写socket
        if(retVal > 0)
        {
            for(int i = 0; i < MAX_CON; i++)
            {
                if(fd_A[i] == 0)
                {
                    continue;
                }

                if(FD_ISSET((fd_A[i]), &fdsr))
                {
                    while(!fin[i].eof())//如果没有到达文件末尾则继续
                    {
                        fin[i].read(buf, BUF_SIZE);
                        cout << buf << endl;

                        retVal = send(fd_A[i], buf, fin[i].gcount(), 0);

                        if(retVal == SOCKET_ERROR)
                        {
                            cout << "Send Into error: " << GetLastError() << endl;
                            return -1;
                        }
                    }
                    memset(buf, 0, BUF_SIZE);
                    closesocket(fd_A[i]);
                    fd_A[i] = 0;
                    fileName[i] = "";
                    fin[i].close();
                    connecting--;
                }
            }
        }
    }
}

int main()
{
    //获取目标文件夹文件名
    vector<string> v_fileName;
    getFileName(path, v_fileName);

    for(unsigned i = 0; i < v_fileName.size(); i++)
    {
        if("." == v_fileName[i] || ".." == v_fileName[i])
        {
            continue;
        }

        if(i == v_fileName.size() - 1)
        {
            allName = allName + v_fileName[i];
            break;
        }

        allName = allName + v_fileName[i] + "\n";
    }

    WSADATA  wsad; //存储SOCKET初始化信息
    SOCKADDR_IN  addrServ_Con; //服务器地址控制信息数据块
    SOCKADDR_IN  addrServ_Tran; //服务器地址传输信息数据块

    //初始化套接字动态库
    if(WSAStartup(MAKEWORD(2, 2), &wsad) != 0)
    {
        cout << "创建套接字失败！" << endl;
        WSACleanup();//释放资源
        return -1;
    }

    Server_Tran = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if(INVALID_SOCKET == Server_Tran)
    {
        printf("初始化套接字Server_Tran失败!\n");
        WSACleanup();//释放套接字资源;
        return  -1;
    }

    //设置服务器套接字
    addrServ_Tran.sin_family = AF_INET;
    addrServ_Tran.sin_port = htons(Tran_PORT_NUM);
    addrServ_Tran.sin_addr.s_addr = INADDR_ANY;

    //绑定套接字
    if(SOCKET_ERROR == bind(Server_Tran, (LPSOCKADDR)&addrServ_Tran, sizeof(SOCKADDR_IN)))
    {
        cout << "绑定套接字Tran失败！" << endl;
        closesocket(Server_Tran);//关闭套接字
        WSACleanup();//释放资源
        return -2;
    }

    Server_Con = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    if(INVALID_SOCKET == Server_Con)
    {
        printf("初始化套接字Server_Con失败!\n");
        WSACleanup();//释放套接字资源;
        return  -1;
    }

    addrServ_Con.sin_family = AF_INET;
    addrServ_Con.sin_port = htons(Con_PORT_NUM);
    addrServ_Con.sin_addr.s_addr = INADDR_ANY;

    if(SOCKET_ERROR == bind(Server_Con, (LPSOCKADDR)&addrServ_Con, sizeof(SOCKADDR_IN)))
    {
        cout << "绑定套接字Con失败！" << endl;
        closesocket(Server_Con);//关闭套接字
        WSACleanup();//释放资源
        return -2;
    }

    //监听
    if(SOCKET_ERROR == listen(Server_Tran, 1))
    {
        cout << "监听失败！" << endl;
        closesocket(Server_Tran);
        WSACleanup();
        return -3;
    }
    else
    {
        cout << "开始监听~" << endl;
    }

    thread t1(sendFileName);
    thread t2(tranFile);
    t1.join();
    t2.join();
}

