//
// Author: Jakub Fajkus
// Project: ISA IRC bot
// Last revision: 17.11.2017
//

#ifndef ISA_SYSLOGSERVER_H
#define ISA_SYSLOGSERVER_H

#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <ctime>

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

public:
   string local_ip;

    /**
     * Connect to the server with given hostname or IP address
     *
     * @param server_hostname Hostname or IP address of the syslog server
     */
    explicit SyslogServer(string server_hostname);

    /**
     * Log the given user message
     *
     * @param user_message
     */
    void log(string user_message);

    /**
     * Close the server socket
     *
     * @return True, if the socket was successfully closed, false otherwise
     */
    bool close_socket();
};


#endif //ISA_SYSLOGSERVER_H
