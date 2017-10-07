//
// Created by jakub on 01.10.17.
//

#ifndef ISA_IRC_H
#define ISA_IRC_H

#include "IrcServer.h"
#include "SyslogServer.h"
#include "utils.h"
#include "Channel.h"

class Irc {
private:
    IrcServer *irc_server;
    SyslogServer *syslog_server;
    vector<string> keywords;
    vector<Channel> channels;
    string channels_string;

    void handle_privmsg(string response, vector<string> tokens);
    bool contains_keywords(vector<string> tokens, vector<string> keywords);
    void check_for_error_messages(vector<string> tokens);
    Channel* get_channel(string &name);
    void add_user_to_channel(string channel, string user);
    void remove_user_from_channel(string channel, string user);
    string get_nickname_from_message(string response);
    vector<string> get_messages_for_user(string user, string channel);
    void send_messages_to_user(string user, string channel);
    void add_message_for_user(string user, string channel, string message);
    void handle_name_reply(vector<string> tokens);
    void handle_join(string response, vector<string> tokens);
    void handle_part(string response, vector<string> tokens);
    void handle_quit(string response);
    void handle_kick(vector<string> response);
    void handle_nick(string response, vector<string> tokens);

public:
    Irc(IrcServer *irc_server, SyslogServer *syslog_server, vector<string> keywords, string channels);
    void listen();
    void init_connection();
    void quit();

};


#endif //ISA_IRC_H
