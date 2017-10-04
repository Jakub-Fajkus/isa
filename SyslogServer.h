//
// Created by jakub on 30.09.17.
//

#ifndef ISA_SYSLOGSERVER_H
#define ISA_SYSLOGSERVER_H

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

#define DATE_FORMAT_LENGTH 16 //Mmm dd hh:mm:ss

using namespace std;

class SyslogServer {
private:
    int syslog_socket;
    struct sockaddr_in syslog_address;
    char* get_local_ip();

public:
    explicit SyslogServer(string server_hostname);
    void log(const string user_message);

    bool close_socket();
};


#endif //ISA_SYSLOGSERVER_H
