//
// Created by jakub on 01.10.17.
//

#ifndef ISA_IRC_H
#define ISA_IRC_H

#include "IrcServer.h"
#include "SyslogServer.h"
#include "utils.h"


class Irc {
private:
    IrcServer *irc_server;
    SyslogServer *syslog_server;
    vector<string> keywords;
    void handle_privmsg(string response, unsigned long privmsg_position, vector<string> tokens);
    bool contains_keywords(const string &message, vector<string> keywords);

public:
    Irc(IrcServer *irc_server, SyslogServer *syslog_server, vector<string> keywords);
    void listen();
    void init_connection(string channels);
};


#endif //ISA_IRC_H
