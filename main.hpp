#include <iostream>
#include <cstring>
#include <vector>
#include <string>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <chrono>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h> 
//#include "./telnet.hpp"


using namespace std;

class worker{
	
	public:
	
	worker(string ip){
		
		//String to Char * , due to inet_addr accepts only char *
		int n = ip.size();
 		char ip_array[n + 1];
		strcpy(ip_array, ip.c_str());
		
		// UDP socket prepares
		int stun_socket = socket(AF_INET, SOCK_DGRAM, 0);

		//Message sender - client prepares
		struct sockaddr_in stun_client;
		memset(&stun_client, 0, sizeof(stun_client));
		stun_client.sin_family = AF_INET;
		stun_client.sin_port = htons(1111);
                stun_client.sin_addr.s_addr = inet_addr("127.0.0.1");
		int rc = bind(stun_socket, (struct sockaddr *)&stun_client, sizeof(stun_client));

		//Remote side - server prepares
		struct sockaddr_in stun_server;
		memset(&stun_server, 0, sizeof(stun_server));
		stun_server.sin_family = AF_INET;
		stun_server.sin_port = htons(3478);
		stun_server.sin_addr.s_addr = inet_addr(ip_array); 
		
		//Stun message prepares
		typedef struct stun_header_tag {
			uint16_t message_type;
			uint16_t message_length;
			unsigned char transaction_id[16];
		} stun_header_t;

		stun_header_t header;
		header.message_type = htons(0x0001); /* Classical Stun - Binding Request */
		header.message_length = htons(0);
		*(int *)(&header.transaction_id[8]) = 0xFFEEFFEE; /* transaction id in the response should keep consistent with this one */

		//Sending message
		rc = sendto(stun_socket, (void *)&header, sizeof(header), 0, (struct sockaddr *)&stun_server, sizeof(stun_server));

		//Waiting for response
		char response[64];
		rc = recvfrom(stun_socket, response, 64, 0, NULL, 0);
		
		cout << "rc: " << rc << endl;
		
	}
};

class proxy
{
	public:
	proxy(string ip, int port)
	{
		//String to Char * , due to inet_addr accepts only char *
		int n = ip.size();
 		char ip_array[n + 1];
		strcpy(ip_array, ip.c_str());
		
		portno = port;
		sockfd = socket(AF_INET, SOCK_STREAM, 0);
		
		if (sockfd < 0) 
		{	
			cout << "Socket problem" << endl;
			return;
		}

		// IP preparing
		if ((server = gethostbyname(ip_array)) == NULL) 
		{
			cout << "Host problem!" << endl; 
			return;
		}
		memset(&serv_addr, 0, sizeof(serv_addr));
		serv_addr.sin_family = AF_INET;
		//bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
		memcpy(&serv_addr.sin_addr.s_addr, server->h_addr, server->h_length);
		// Port preparing
		serv_addr.sin_port = htons(portno);
		
		// Connecting
		if (connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0)
		{
			cout << "Telnet problem. Can't connect for now!" << endl;
			return;
		}
		
		// Buffer cleaning
		memset(buffer, 0, 2048);

		// Password
		strcpy(buffer, "qwerty");
		
		if(!write(sockfd,buffer,strlen(buffer)))
		{
			cout << "Password problem!" << endl;
			return;				
		}
		
		// Buffer cleaning
		memset(buffer, 0, 2048);
		
		if(!read(sockfd,buffer,2047))
		{
			cout << "No response!" << endl;
			return;				
		}
		
		// Buffer cleaning
		memset(buffer, 0, 2048);

		// PC
		strcpy(buffer, "pc");
		
		if(!write(sockfd,buffer,strlen(buffer)))
		{
			cout << "PC problem!" << endl;
			return;				
		}

		// Buffer cleaning
		memset(buffer, 0, 2048);
		
		if(!read(sockfd,buffer,2047))
		{
			cout << "No response!" << endl;
			return;				
		}
		
		cout << "Response: " << buffer << endl;
		
		// C-style string to C++ string
		string buffer_string{buffer};

		
		size_t found_alternate{},found_space{};
		
		for(;;)
		{
			found_alternate = buffer_string.find("Alternate server:",found_alternate+1);
			if(found_alternate == string::npos)
				break;
			found_space = buffer_string.find(" ",found_alternate + 18);
			cout << "found_alternate: " << found_alternate << " found_space: " <<found_space << endl;
			alternative_servers.push_back(buffer_string.substr(found_alternate + 18,found_space - found_alternate - 18));			
		}
				
		cout << "socket: " << sockfd << endl;
		close(sockfd);
		cout << "socket down " << endl;

		return;
		
	}
	
	~proxy()
	{
		cout << "~proxy()" << endl;
		//delete(this);
	}

	
	vector<string> get_workers()
	{
		return alternative_servers;
	}
	
	
	private:
		int sockfd{}, portno{}, n{};
		struct sockaddr_in serv_addr{};
		struct hostent *server{};
		char buffer[2048]{};
		vector<string> alternative_servers{};

};


//Create a worker
worker* worker_function(string ip)
{
	worker * worker1 = new worker(ip);
    return worker1;
}

//Worker wrapper
int worker_function_call(string ipi)
{
    std::mutex m;
    std::condition_variable cv;
    worker* retValue;

    std::thread t([&cv, &retValue, ipi]() 
    {
        retValue = worker_function(ipi);
        cv.notify_one();
    });

    t.detach();

    {
        std::unique_lock<std::mutex> l(m);
        if(cv.wait_for(l, 1s) == std::cv_status::timeout) 
            throw std::runtime_error("Timeout");
    }
	
	delete retValue;

    return 0;    
}

vector<string> *  proxy_function(string ip, int port){
	
	cout << "vector<string> * workers_list" << endl;
	proxy * proxy1 = new proxy(ip,443);
	cout << "vector<string> * workers_list" << endl;
	vector<string> * workers_list = new vector<string>;
	vector<string> workers_list_ret = proxy1->get_workers();
	for( string i:workers_list_ret)
	{
		cout << i << endl;
		workers_list->push_back(i);
	}
	//proxy1->~proxy();
	//delete proxy1;
	cout << "Before return" << endl;
	return workers_list;
}

//Proxy wrapper
vector<string> * proxy_function_call(string ip, int port)
{
    std::mutex m;
    std::condition_variable cv;
    vector<string> * retValue;

    std::thread t([&cv, &retValue, ip, port]() 
    {
        retValue = proxy_function(ip, port);
        cv.notify_one();
    });

    t.detach();

    {
        std::unique_lock<std::mutex> l(m);
        if(cv.wait_for(l, 10s) == std::cv_status::timeout) 
            throw std::runtime_error("Timeout");
    }
	
    return retValue;    
}

int proxy_kill_function(proxy * proxy1){
	
	//delete proxy1;
	return 0;
}