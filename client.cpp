#define _WINSOCK_DEPRECATED_NO_WARNINGS 
#define PACKET_SIZE 1024
#pragma comment(lib,"ws2_32.lib")

#include <winsock2.h>
#include <iostream>
#include <sstream>
#include <string>
#include <thread>
#include <WS2tcpip.h>
#include <conio.h> 
#include <mysql/jdbc.h>

using std::cout;
using std::endl;
using std::string;
using std::cin;

const string server = "tcp://127.0.0.1:3306"; 
const string username = "root";
const string password = "1234"; 
const string dbname = "chattingdb"; 

sql::mysql::MySQL_Driver* driver;
sql::Connection* con;
sql::PreparedStatement* pstmt;
sql::ResultSet* result;
sql::Statement* stmt;

SOCKET client_socket;
string nickname;

	void createuser()
	{
		string id, passwd;
		cout << " /////////회원가입//////////" << endl;
		cout << "아이디를 입력하세요: " ;
		cin >> id;
		cout << "비밀번호를 입력하세요: ";
		char ch = ' ';
		while (ch != 13) { 
			ch = _getch();
			if (ch == 13) break;
			else if (ch == 8) {
				if (!passwd.empty()) {
					passwd.pop_back();
					cout << "\b \b";								
				}
			}
			else {
				passwd.push_back(ch);
				cout << '*' ; // 별표로 대체하여 출력
			}
		}
			pstmt = con->prepareStatement("INSERT INTO users (id,password) VALUES (?, ?)");
			pstmt->setString(1, id);
			pstmt->setString(2, passwd);
			pstmt->execute();
			cout << "\n회원가입이 완료되었습니다." << endl;
			passwd.clear();
	}

	int login()
	{
		string passwd;
		cout << " /////////로그인//////////" << endl;
		cout << "아이디를 입력하세요 : ";
		cin >> nickname;
		cout << "비밀번호를 입력해주세요 : ";
		char ch = ' ';
		while (ch != 13) { 
			ch = _getch();
			if (ch == 13) break; 
			else if (ch == 8) { 
				if (!passwd.empty()) { 
					passwd.pop_back(); 
					cout << "\b \b"; 							
				}
			}
			else {
				passwd.push_back(ch);
				cout << '*'; 
			}
		}
		cout << endl;
		pstmt = con->prepareStatement("SELECT id, password FROM users WHERE id=?");
		pstmt->setString(1, nickname);
		
		result = pstmt->executeQuery();

		if (result->next())
		{ 
			string db_id = result->getString(1).c_str(); 
			string db_pw = result->getString(2).c_str();

			if (db_id == nickname && db_pw == passwd)
			{
				cout << nickname << "님 환영합니다." << endl;
			
				return 1;
			}
			else cout << "해당하는 정보가 없습니다." << endl;
			return -1;
		}
		else cout << "해당하는 정보가 없습니다." << endl;
		return -1;
	}

	void startMenu()
	{
		cout << "\n";
		cout << " "; cout << "//////////////////////////////////////\n";
		cout << " "; cout << "                                      \n";
		cout << " "; cout << "                1.로그인              \n";
		cout << " "; cout << "                                      \n";
		cout << " "; cout << "                2.회원가입            \n";
		cout << " "; cout << "                                      \n";
		cout << " "; cout << "                3.종료                \n";
		cout << " "; cout << "                                      \n";
		cout << " "; cout << "//////////////////////////////////////\n\n";
	}

	void Store(string check_id) 
	{

		string StoreUser, StoreMsg;  

		bool id = true;
		cout << "이전 대화 내용" << endl;
			con->setSchema("chattingdb");
			pstmt = con->prepareStatement("SELECT * FROM chatting ;");
		result = pstmt->executeQuery();
		while (result->next()) 
		{
			StoreUser = result->getString("id").c_str();
			StoreMsg = result->getString("chat").c_str();
		
			if (StoreUser == check_id)
			{
				id = false;
			}
			if (id == false)
			{
				cout << StoreUser << " : " << StoreMsg << endl;
			}
		}
	}


	void proc_recv() 
	{
		char buffer[PACKET_SIZE] = {}; 
		while (!WSAGetLastError()) 
		{
			ZeroMemory(&buffer, PACKET_SIZE); 
			recv(client_socket, buffer, PACKET_SIZE, 0); 
			cout << buffer << endl;
		}
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
		stmt = con->createStatement();
		delete stmt;

		bool loginSuccess = false;

		while (!loginSuccess)
		{
			startMenu(); 
			char start = 0;
			cout << "번호입력 >> ";
			cin >> start;
			switch (start)
			{
			case '1': 
				if (login() == 1) {
					loginSuccess = true;
					break;
				}
				else {
					continue;
				}
			case '2': 
				createuser();
				continue;
			case '3':
				return -1;
			default: 
				cout << "잘못된 입력입니다. 다시 입력해주세요." << endl;
				continue;
			}

		}
	
		client_socket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

		SOCKADDR_IN addr = {};
		addr.sin_family = AF_INET;
		addr.sin_port = htons(4444);
		InetPton(AF_INET, TEXT("127.0.0.1"), &addr.sin_addr);

		while (1) { 
			if (!connect(client_socket, (SOCKADDR*)&addr, sizeof(addr)))
				send(client_socket, nickname.c_str(), nickname.length(), 0);
			break;
		}

		std::thread th1(proc_recv);
		Store(nickname);
		while (1) {
			string text;
			getline(cin, text);
			const char* buffer = text.c_str();
			send(client_socket, buffer, strlen(buffer), 0);
		}

		th1.join(); 

		closesocket(client_socket); 

		WSACleanup(); 
		return 0;
	}