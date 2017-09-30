//
// Created by jakub on 30.09.17.
//

#include "IrcServer.h"

IrcServer::IrcServer(string server_hostname, int port) {
    struct hostent *server;
//    this->irc_address = new struct sockaddr_in;

    cout << server_hostname << "<<<<<<<" << endl << endl;

    /* 2. ziskani adresy serveru pomoci DNS */
    if ((server = gethostbyname(server_hostname.c_str())) == nullptr) {
        cerr << "ERROR: no such host: " << server_hostname << endl;
        exit(EXIT_FAILURE);
    }

/* 3. nalezeni IP adresy serveru a inicializace struktury server_address */
    memset(&this->irc_address,0,sizeof(this->irc_address));
    this->irc_address.sin_family = AF_INET;
    bcopy((char *) server->h_addr, (char *) &(this->irc_address.sin_addr.s_addr), server->h_length);
    this->irc_address.sin_port = htons(port);

/* Vytvoreni soketu */
    if ((this->irc_socket = socket(AF_INET, SOCK_STREAM, 0)) <= 0) {
        perror("ERROR: socket irc");
        exit(EXIT_FAILURE);
    }

    if (-1 == connect(this->irc_socket, (struct sockaddr *) &this->irc_address, sizeof(this->irc_address))) {
        cerr << "Cannot connect to the IRC socket" << endl;
        exit(1);
    }
}

string IrcServer::read_message() {
    char buffer[513] = "";
    ssize_t i;

    unsigned int read_chars = 0;
    while (true) {
        i = read(this->irc_socket, buffer + read_chars, 1);
        if (i == -1) {
            err(1, "read() failed");
        }

        //if there is an CR LF pair, stop reading!
        if (read_chars >= 1 && buffer[read_chars] == '\n' && buffer[read_chars - 1] == '\r') {

            cout << "RECEIVED: >>>" << string(buffer, read_chars - 1) << "<<<" << endl;

            return string(buffer, read_chars - 1);
        }

        read_chars++;
    }
}

void IrcServer::send_message(string message) {
    message += "\r\n";

    int i = write(this->irc_socket, message.c_str(), message.length());                    // send data to the server
    if (i == -1)                                 // check if data was sent correctly
        err(1, "write() failed");
    else if (i != message.length())
        err(1, "write(): buffer written partially");
    else
        cout << "Sent: " << message << endl;
}
