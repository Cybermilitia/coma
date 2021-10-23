#include "./main.hpp"

int main()
{
    bool timedout = false;
    try {
		//string ip{"47.168.22.11"};
		string ip{"86.108.188.91"};
        worker_function_call(ip);
    }
    catch(std::runtime_error& e) {
        std::cout << e.what() << std::endl;
        timedout = true;
    }

    if(!timedout)
	{
        std::cout << "Status Up!" << std::endl;
	}
	else
	{
		std::cout << "Status Down!" << std::endl;
	}

    try {
		//string ip{"47.168.22.11"};
		string ip{"86.108.188.91"};
        telnet_function_call();
    }
    catch(std::runtime_error& e) {
        std::cout << e.what() << std::endl;
        timedout = true;
    }

    if(!timedout)
	{
        std::cout << "Status Up!" << std::endl;
	}
	else
	{
		std::cout << "Status Down!" << std::endl;
	}

    return 0;
}

