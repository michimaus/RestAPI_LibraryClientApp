#include <iostream>
#include <stdlib.h>
#include "MyClient.h"

#define SERVER_IP "3.8.116.10"
#define SERVER_PORT 8080

int main () {
    char ip[17];
    memset(ip, 0, 17);

    strcpy(ip, SERVER_IP);
    unsigned short int port = SERVER_PORT;

    // Creating the object for the client and
    // calling the main runner of the app
    MyClient newClient(ip, port);
    newClient.runClient();

    return 0;
}