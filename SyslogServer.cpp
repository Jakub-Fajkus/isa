//
// Created by jakub on 30.09.17.
//

#include "SyslogServer.h"

const char *months[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

SyslogServer::SyslogServer(string server_hostname) {
    struct hostent *server;

    /* 2. ziskani adresy serveru pomoci DNS */
    if ((server = gethostbyname(server_hostname.c_str())) == nullptr) {
        cerr << "ERROR: no such host: " << server_hostname << endl;
        exit(EXIT_FAILURE);
    }

/* 3. nalezeni IP adresy serveru a inicializace struktury server_address */
    memset(&this->syslog_address,0,sizeof(this->syslog_address));
    this->syslog_address.sin_family = AF_INET;
    bcopy((char *) server->h_addr, (char *) &(this->syslog_address.sin_addr.s_addr), server->h_length);
    this->syslog_address.sin_port = htons(514);

/* Vytvoreni soketu */
    if ((this->syslog_socket = socket(AF_INET, SOCK_DGRAM, 0)) <= 0) {
        perror("ERROR: socket syslog");
        exit(EXIT_FAILURE);
    }
}

void SyslogServer::log(const string user_message) {
    int bytestx;
    socklen_t serverlen;
    string buf = "<134>";
    char dateBuf[DATE_FORMAT_LENGTH];
    time_t time_number;
    struct tm *time_struct;

    time(&time_number);
    time_struct = localtime(&time_number);
    strftime(dateBuf, DATE_FORMAT_LENGTH, " %e %T", time_struct);
    buf += months[time_struct->tm_mon] + string(dateBuf) + " ";

    buf += this->get_local_ip();
    buf += " isabot " + user_message;

    cout << endl << endl << buf << endl << endl;

/* odeslani zpravy na server */
    serverlen = sizeof(syslog_address);
    bytestx = sendto(syslog_socket, buf.c_str(), buf.length() > 1024 ? 1024 : buf.length(), 0,
                     (struct sockaddr *) &syslog_address, serverlen);
    if (bytestx < 0)
        perror("ERROR: sendto");
}

char *SyslogServer::get_local_ip() {
    struct sockaddr_in local;
    socklen_t len;

    memset(&local, 0, sizeof(local));   // erase the local address structure

    // obtain the local IP address and port using getsockname()
    len = sizeof(local);
    if (getsockname(this->syslog_socket, (struct sockaddr *) &local, &len) == -1) {
        cerr << "getsockname() failed";
        exit(1);
    }

    return inet_ntoa(local.sin_addr);

}

bool SyslogServer::close_socket() {
    return close(this->syslog_socket) == 0;
}