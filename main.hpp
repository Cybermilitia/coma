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
#include "./telnet.hpp"


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
		stun_server.sin_port = htons(443);
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

int telnet_function(void){
	#ifdef POSIX
		termios stored_settings;
		tcgetattr(0, &stored_settings);
		termios new_settings = stored_settings;
		new_settings.c_lflag &= (~ICANON);
		new_settings.c_lflag &= (~ISIG); // don't automatically handle control-C
		tcsetattr(0, TCSANOW, &new_settings);
	#endif

	boost::asio::io_service io_service;
	// resolve the host name and port number to an iterator that can be used to connect to the server
	tcp::resolver resolver(io_service);
	tcp::resolver::query query("52.88.68.92", "1234");
	tcp::resolver::iterator iterator = resolver.resolve(query);
	// define an instance of the main class of this program
	telnet_client c(io_service, iterator);
	// run the IO service as a separate thread, so the main thread can block on standard input
	boost::thread t(boost::bind(&boost::asio::io_service::run, &io_service));
	while (1)
	{
		char ch;
		cin.get(ch); // blocking wait for standard input
		if (ch == 3) // ctrl-C to end program
			break;
		c.write(ch);
	}
	c.close(); // close the telnet client connection
	t.join(); // wait for the IO service thread to close

	#ifdef POSIX // restore default buffering of standard input
		tcsetattr(0, TCSANOW, &stored_settings);
	#endif
	
	return 0;
}

//Telnet wrapper
int telnet_function_call(void)
{
    std::mutex m;
    std::condition_variable cv;
    int retValue;

    std::thread t([&cv, &retValue]() 
    {
        retValue = telnet_function();
        cv.notify_one();
    });

    t.detach();

    {
        std::unique_lock<std::mutex> l(m);
        if(cv.wait_for(l, 1s) == std::cv_status::timeout) 
            throw std::runtime_error("Timeout");
    }
	
    return 0;    
}