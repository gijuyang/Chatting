#pragma comment(lib, "ws2_32.lib")
#define MAX_SIZE 1024
#define MAX_CLIENT 4

#include <winsock2.h> 
#include <iostream>
#include <sstream>
#include <string>
#include <thread>
#include <vector>
#include <mysql/jdbc.h>

using std::cout;
using std::endl;
using std::string;
using std::cin;
using std::vector;

const string server = "tcp://127.0.0.1:3306";
const string username = "root";
const string password = "1234";
const string dbname = "chattingdb";

sql::mysql::MySQL_Driver* driver;
sql::Connection* con;
sql::Statement* stmt;
sql::PreparedStatement* pstmt;
sql::ResultSet* result;

    struct SOCKET_INFO
     {
          SOCKET socket;
          string nickname;
     };

vector<SOCKET_INFO> socket_list;
SOCKET_INFO server_socket;
int client_count = 0;

void server_init();             //socket, bind,listen 
void add_client();              //accept
void send_msg(const char msg);  //전체메시지 
void recv_msg(int idx);         //recv
void del_client(int idx); 


    void server_init() 
    {
        server_socket.socket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP); 

        SOCKADDR_IN server_addr = {};
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(4444);
        server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
        bind(server_socket.socket, (sockaddr*)&server_addr, sizeof(server_addr));
        listen(server_socket.socket, SOMAXCONN);
        server_socket.nickname = "server";
        cout << " "; cout << "//////////////////////////////////////\n";
        cout << " "; cout << "/                                    /\n";
        cout << " "; cout << "/                                    /\n";
        cout << " "; cout << "/             server on              /\n";
        cout << " "; cout << "/                                    /\n";
        cout << " "; cout << "/                                    /\n";
        cout << " "; cout << "//////////////////////////////////////\n\n";
    }

    void add_client() 
    {
        SOCKADDR_IN addr = {};
        int addrsize = sizeof(addr);
        char buf[MAX_SIZE] = { };

        ZeroMemory(&addr, addrsize);
        SOCKET_INFO new_client = {};
        new_client.socket = accept(server_socket.socket, (sockaddr*)&addr, &addrsize);
        recv(new_client.socket, buf, MAX_SIZE, 0);
        new_client.nickname = string(buf);

        string msg = new_client.nickname + "님이 입장했습니다";
        cout << msg << endl;
        socket_list.push_back(new_client);

        std::thread th(recv_msg, client_count);
        client_count++;

        cout << "현재 인원수:" << client_count << "명" << endl;

        th.join();
    }

    void send_msg(const char* msg) 
    {
        for (int i = 0; i < client_count; i++)
        {
            send(socket_list[i].socket, msg, MAX_SIZE, 0);
        }
    }

    void recv_msg(int idx) 
    {
        char buf[MAX_SIZE] = { };
        string msg = "";
        while (1)
        {
            ZeroMemory(&buf, MAX_SIZE);
            if (recv(socket_list[idx].socket, buf, MAX_SIZE, 0) > 0)
            {
                cout << socket_list[idx].nickname + ":" + buf << endl;
                msg = socket_list[idx].nickname + ":" + buf;
                    if (buf != "") {
                        pstmt = con->prepareStatement("insert into chatting(id, chat) values(?,?)");
                        pstmt->setString(1, socket_list[idx].nickname);
                        pstmt->setString(2, buf);
                        pstmt->execute();
                    }
                    send_msg(msg.c_str());
            }
            else
            {
                msg = socket_list[idx].nickname + "님이 퇴장했습니다";
                cout << msg << endl;
                send_msg(msg.c_str());
                del_client(idx);
                return;
            }

        }
    }

    void del_client(int idx) 
    {
        closesocket(socket_list[idx].socket);
        client_count--;
    }


    int main()
    {
        WSADATA wsa;
        WSAStartup(MAKEWORD(2, 2), &wsa);

        try {
            driver = sql::mysql::get_mysql_driver_instance();
            con = driver->connect(server, username, password);
            con->setSchema(dbname);
        }
        catch (sql::SQLException& e) {
            cout << e.what() << endl;
            exit(1);
        }


        stmt = con->createStatement();
        stmt->execute("set names euckr");
        if (stmt) { delete stmt; stmt = nullptr; }

        server_init();

        std::thread th1[MAX_CLIENT];

        for (int i = 0; i < MAX_CLIENT; i++)
        {
            th1[i] = std::thread(add_client);
        }

        while (1)
        {
            string text, msg = "";
            std::getline(cin, text);
            const char* buf = text.c_str();
            msg = server_socket.nickname + ":" + buf;
        }

        for (int i = 0; i < MAX_CLIENT; i++)
        {
            th1[i].join();
        }

        WSACleanup();
    }