//
// Created by jakub on 01.10.17.
//

#include "Irc.h"

Irc::Irc(IrcServer *irc_server, SyslogServer *syslog_server, vector<string> keywords) : irc_server(irc_server),
                                                                                        syslog_server(syslog_server),
                                                                                        keywords(keywords) {}

void Irc::listen() {
    while (true) {
        string response = this->irc_server->read_message();
        vector<string> tokens = explode_string(" ", response);

        //ping command
        if (tokens.size() >= 2 && tokens[0] == "PING") {
            this->irc_server->send_message(string("PONG ") + explode_string(":", tokens[1])[1]);
            continue;
        }

        //todo: notice
        //RECEIVED: >>>:card.freenode.net NOTICE * :*** No Ident response<<<

//        //is it a PRIVMSG ?
        unsigned long privmsg_position = response.find(" PRIVMSG ");
        if (privmsg_position != string::npos) {
            handle_privmsg(response, privmsg_position, tokens);
            continue;
        }

    }

}

void Irc::handle_privmsg(string response, unsigned long privmsg_position, vector<string> tokens) {
    unsigned long user_msg_start = response.find(":", privmsg_position);
    if (user_msg_start != string::npos) {
        string msg(response, user_msg_start + 1);

        if (response[0] == ':') {
            unsigned long excl_mark_position = response.find("!");
            string nickname(response, 1, excl_mark_position - 1);
            cout << "nickname: >>" << nickname << "<<" << endl;

            cout << msg << endl;

            string channel_name = tokens[2];
            if (!(channel_name[0] == '#' || channel_name[0] == '&')) {
                err(1, "Cannot read the chanel name from the privmsg");
            }

            if (msg.find("?today") == 0) {
                string message = string("PRIVMSG " + channel_name + " :" + get_today_date());
                this->irc_server->send_message(message);
            } else if (msg.find("?msg") == 0) {

            } else {
                if (contains_keywords(response, keywords)) {
                    this->syslog_server->log("<" + nickname + ">: " + msg);
                }
            }
        }

    }
}

bool Irc::contains_keywords(const string &message, vector<string> keywords) {
    for (const string &keyword : keywords) {
        if (message.find(keyword) != string::npos) {
            return true;
        }
    }

    return false;
}

void Irc::init_connection(string channels) {
    irc_server->send_message("USER xfajku06 xfajku06 xfajku06 xfajku06");
    irc_server->send_message("NICK xfajku06");
    irc_server->send_message("JOIN " + channels);
}

