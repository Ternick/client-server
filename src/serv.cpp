#include <iostream>
#include "server/server.h"
#include "iio_client/iio_client.h"

int main()
{
    std::cout << "Welcome to Server" << std::endl;
    IIOClient client = IIOClient("iio_dummy_part_no", "instance1");
    Server server = Server("127.0.0.1", 7335, &client);
    server.start();
    return 0;
}