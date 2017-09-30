//
// Created by jakub on 30.09.17.
//

#ifndef ISA_IRCSERVER_H
#define ISA_IRCSERVER_H

#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <ctime>

#include <stdlib.h>
#include <arpa/inet.h>
#include <strings.h>
#include <cstring>
#include <vector>
#include <unistd.h>
#include <err.h>

using namespace std;

class IrcServer {
private:
    int irc_socket;
    struct sockaddr_in irc_address;
public:
    explicit IrcServer(string server_hostname, int port);
    string read_message();
    void send_message(string message);
};


#endif //ISA_IRCSERVER_H
