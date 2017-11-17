//
// Author: Jakub Fajkus
// Project: ISA IRC bot
// Last revision: 17.11.2017
//

#include "SyslogServer.h"

const char *months[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

SyslogServer::SyslogServer(string server_hostname) {
    struct hostent *server;

    //get the dns record of the IRC server
    if ((server = gethostbyname(server_hostname.c_str())) == nullptr) {
        throw "ERROR: no such IRC server: " + server_hostname;
    }

    //get the server ip address
    memset(&this->syslog_address, 0, sizeof(this->syslog_address));
    this->syslog_address.sin_family = AF_INET;
    bcopy((char *) server->h_addr, (char *) &(this->syslog_address.sin_addr.s_addr), server->h_length);
    this->syslog_address.sin_port = htons(514);

    //create the server socket
    if ((this->syslog_socket = socket(AF_INET, SOCK_DGRAM, 0)) <= 0) {
        throw "ERROR: cannot connect to the syslog socket";
    }
}

void SyslogServer::log(const string user_message) {
    int bytestx;
    socklen_t serverlen;
    string buf = "<134>";
    char dateBuf[DATE_FORMAT_LENGTH];
    time_t time_number;
    struct tm *time_struct;

    //get the current time
    time(&time_number);
    time_struct = localtime(&time_number);

    //format the time to the syslog format
    strftime(dateBuf, DATE_FORMAT_LENGTH, " %e %T", time_struct);
    buf += months[time_struct->tm_mon] + string(dateBuf) + " ";
    //add the local ip or hostname
    buf += this->local_ip;
    //add the message
    buf += " isabot " + user_message;

    //send the message to the server
    serverlen = sizeof(syslog_address);
    bytestx = sendto(syslog_socket, buf.c_str(), buf.length() > 1024 ? 1024 : buf.length()+1, 0,
                     (struct sockaddr *) &syslog_address, serverlen);
    if (bytestx < 0)
        throw "error while trying to send data to syslog server";
}

bool SyslogServer::close_socket() {
    return close(this->syslog_socket) == 0;
}