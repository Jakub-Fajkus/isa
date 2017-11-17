//
// Author: Jakub Fajkus
// Project: ISA IRC bot
// Last revision: 17.11.2017
//

#include "IrcServer.h"

IrcServer::IrcServer(string server_hostname, int port) {
    struct hostent *server;

    //get the dns record of the IRC server
    if ((server = gethostbyname(server_hostname.c_str())) == nullptr) {
        throw "ERROR: no such host: " + server_hostname;
    }

    //get the server ip address
    memset(&this->irc_address, 0, sizeof(this->irc_address));
    this->irc_address.sin_family = AF_INET;
    bcopy((char *) server->h_addr, (char *) &(this->irc_address.sin_addr.s_addr), server->h_length);
    this->irc_address.sin_port = htons(port);

    //create the server socket
    if ((this->irc_socket = socket(AF_INET, SOCK_STREAM, 0)) <= 0) {
        throw "ERROR: socket irc";
    }

    //connect to te socket
    if (-1 == connect(this->irc_socket, (struct sockaddr *) &this->irc_address, sizeof(this->irc_address))) {
        throw "Cannot connect to the IRC socket";
    }



}

string IrcServer::read_message() {
    char buffer[513] = "";
    ssize_t i;

    unsigned int read_chars = 0;
    while (true) {
        //read the message character
        i = read(this->irc_socket, buffer + read_chars, 1);
        if (i == -1) {
            throw "read() failed";
        }

        //if there is an CR LF pair, stop reading!
        if (read_chars >= 1 && buffer[read_chars] == '\n' && buffer[read_chars - 1] == '\r') {
            return string(buffer, read_chars - 1);
        }

        if (read_chars > 512) {
            throw "The IRC message was too long";
        }

        read_chars++;
    }
}

void IrcServer::send_message(string message) {
    message += "\r\n";

    //send message to the server
    int i = write(this->irc_socket, message.c_str(), message.length());
    if (i == -1) {// check if message was sent correctly
        throw "IRC message write() failed";
    } else if (i != message.length()) {
        throw "IRC message write(): buffer written partially";
    }
}

bool IrcServer::close_socket() {
    return close(this->irc_socket) == 0;
}

//inspired by the echo-client.c file from the course examples
char *IrcServer::get_local_ip() {
    struct sockaddr_in local;
    socklen_t len;

    //erase the local address structure
    memset(&local, 0, sizeof(local));

    // obtain the local IP address and port using getsockname()
    len = sizeof(local);
    if (getsockname(this->irc_socket, (struct sockaddr *) &local, &len) == -1) {
        throw "getsockname() failed";
    }

    return inet_ntoa(local.sin_addr);
}