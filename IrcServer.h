//
// Author: Jakub Fajkus
// Project: ISA IRC bot
// Last revision: 17.11.2017
//

#ifndef ISA_IRCSERVER_H
#define ISA_IRCSERVER_H

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

using namespace std;

class IrcServer {
private:
    int irc_socket;
    struct sockaddr_in irc_address;
public:
    /**
     * Connect to the server with given hostname listening on the given port
     *
     * @param server_hostname Hostname of the IRC server
     * @param port Port of the IRC server
     */
    explicit IrcServer(string server_hostname, int port);

    /**
     * Read the next IRC message
     *
     * @return Message string
     */
    string read_message();

    /**
     * Send a message to the server
     *
     * @param message Message string.
     */
    void send_message(string message);

    /**
     * Close the server socket
     *
     * @return True, if the socket was successfully closed, false otherwise
     */
    bool close_socket();

    /**
     * Get the local IP address
     *
     * @return
     */
    char *get_local_ip();
};


#endif //ISA_IRCSERVER_H
