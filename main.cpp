#include "./main.hpp"

int main()
{
    bool timedout{false};
	vector<string> * workers_list{};
	size_t found{};
	proxy * proxy1{};

    try 
	{
		//string ip{"47.168.22.11"};
		string ip{"86.108.188.238"};
		int port{443};
        workers_list = proxy_function_call(ip, port);
    }
    catch(std::runtime_error& e) 
	{
        std::cout << e.what() << std::endl;
        timedout = true;
    }	
    if(!timedout)
	{
        std::cout << "Proxy Coturn status UP!" << std::endl;
	}
	else
	{
		std::cout << "Proxy Coturn status DOWN!" << std::endl;
	}
	cout << "Before" << endl;
	proxy_kill_function(proxy1);
	//delete proxy1;
	cout << "After" << endl;
	
	for(auto ip_and_port:*workers_list){
		
		cout << "************************" << endl;
		try 
		{
			found = ip_and_port.find(":");
			string ip{ip_and_port.substr(0,found)};
			worker_function_call(ip);
		}
		catch(std::runtime_error& e) {
			std::cout << "ERROR: " << e.what() << std::endl;
			timedout = true;
		}
		
		if(!timedout)
		{
			std::cout << "Worker status UP! " << ip_and_port <<  std::endl;
		}
		else
		{
			std::cout << "Worker status DOWN! " << ip_and_port << std::endl;
		}
		
		timedout = false;

	}

	delete workers_list;

    return 0;
}

