#include <iostream>
#include "server/server.h"

int main()
{
    std::cout << "Welcome to Server" << std::endl;
    Server server = Server("127.0.0.1", 7335);
    server.start();
    return 0;
}