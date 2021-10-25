#include <iostream>
#include <string>
#include <cstring>
#include <unistd.h>
#include <vector>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 

using namespace std;

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
		
		//cout << "Response: " << buffer << endl;
		
		// C-style string to C++ string
		string buffer_string{buffer};

		
		size_t found_alternate{},found_space{};
		
		for(;;)
		{
			found_alternate = buffer_string.find("Alternate server:",found_alternate+1);
			if(found_alternate == string::npos)
				break;
			found_space = buffer_string.find(" ",found_alternate + 18);
			//cout << "found_alternate: " << found_alternate << " found_space: " <<found_space << endl;
			alternative_servers.push_back(buffer_string.substr(found_alternate + 18,found_space - found_alternate - 18));			
		}
		
		return;
		
	}
	
	~proxy()
	{
		close(sockfd);
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
