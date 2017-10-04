//
// Created by jakub on 01.10.17.
//

#include "Irc.h"
#include <algorithm>


#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-noreturn"

Irc::Irc(IrcServer *irc_server, SyslogServer *syslog_server, vector<string> keywords, string channels)
        : irc_server(irc_server), syslog_server(syslog_server), keywords(std::move(keywords)),
          channels_string(channels) {

    for (string &channel : explode_string(",", channels)) {
        Channel new_channel(channel);

        //creates a channel if it does not exist
        this->get_channel(channel);
    }
}

void Irc::listen() {
    static bool is_first = true;

    while (true) {
        string response = this->irc_server->read_message();
        vector<string> tokens = explode_string(" ", response);

        //get the server name
        if (is_first) {
            this->server_name = string(tokens[0], 1);

            is_first = false;
        }

        //PING command
        if (tokens.size() >= 2 && tokens[0] == "PING") {
            this->irc_server->send_message(string("PONG ") + explode_string(":", tokens[1])[1]);
            continue;

        } else if (tokens.size() >= 2 && (tokens[1] == "PRIVMSG" || tokens[1] == "NOTICE")) {
            handle_privmsg(response, tokens);
            continue;
        } else if (tokens.size() >= 6 && tokens[1] == "353") { //RPL_NAMEREPLY - response to NAMES command
//            :tolkien.freenode.net 353 xfajku06 = #freenode :Whiskey hexa- ChrisLane mt
            string channel_name = tokens[4];

            //remove the : from the first user name
            tokens[5] = string(tokens[5], 1);

            //add all users
            for (int i = 5; i < tokens.size(); ++i) {

                //is it the operator?
                if (tokens[i][0] == '@') {
                    tokens[i] = string(tokens[i], 1); //remove the first character
                }
                this->add_user_to_channel(channel_name, tokens[i]);
                this->send_messages_to_user(tokens[i], channel_name);
            }

        } else if (tokens.size() >= 3 && tokens[1] == "JOIN") {
            //:Anne!~Anne@annee.powered.by.lunarbnc.net JOIN #freenode
            string nickname = this->get_nickname_from_message(response);
            string channel = tokens[2];

            //add the user to the channel
            this->add_user_to_channel(channel, nickname);
            this->send_messages_to_user(nickname, channel);

        } else if (tokens.size() >= 3 && tokens[1] == "PART") {
            //leaving the channel
            //:fikus!~fikus@ip-89-103-184-234.net.upcbroadband.cz PART #ISAChannel :"Leaving"
            string nickname = this->get_nickname_from_message(response);
            vector<string> channels = explode_string(",", tokens[2]);

            for (const string &channel_name : channels) {
                this->remove_user_from_channel(channel_name, nickname);
            }
        } else if (tokens.size() >= 3 && tokens[1] == "QUIT") {
            //:fikus!~fikus@ip-89-103-184-234.net.upcbroadband.cz QUIT :Client Quit
            string nickname = this->get_nickname_from_message(response);

            for (Channel &channel : this->channels) {
                channel.remove_user(nickname);
            }
        } else if (tokens.size() >= 4 && tokens[1] == "KICK") {
            //:fikus!~fikus@ip-89-103-184-234.net.upcbroadband.cz KICK #fikustest fikus2 :fikus2
            this->remove_user_from_channel(tokens[2], tokens[3]);
        } else if (tokens.size() >= 2 && tokens[1] == "NICK") {
            //:fikus!~fikus@ip-89-103-184-234.net.upcbroadband.cz NICK :fikuss
            string old_nickname = get_nickname_from_message(response);
            string new_nickname = string(tokens[2], 1); //remove the :

            //change the users nick in all channels
            for (Channel &channel : this->channels) {
                channel.remove_user(old_nickname);
                channel.add_user(new_nickname);
            }
        } else if (tokens.size() >= 4 && tokens[1] == "KICK" && tokens[3] == "xfajku06") {
//            :fikus!~fikus@2001:67c:1220:8b4:bca1:7c2:446d:3f2e KICK #fikustest xfajku06 :xfajku06
            //the bot was kicked..., so close the channels and exit
            this->quit();
            cerr << "The bot was kicked" << endl;
            exit(1);
        } else if (tokens.size() >= 2 && tokens[1] == "433") { //nickname used
            this->quit();
            cerr << "The bot's nickname(xfajku06) is already used" << endl;
            exit(1);
        } else if (tokens.size() >= 2 && tokens[1] == "465") { //bot banned
            cerr << "The bot is banned" << endl;
            exit(1);
        } else if (tokens.size() >= 2 && tokens[1] == "404") { //ERR_CANNOTSENDTOCHAN
            err(1, (string("ERR_CANNOTSENDTOCHAN: Cannot send a message to channel: ") + tokens[2]).c_str());
        } else if (tokens.size() >= 2 && tokens[1] == "405") { //ERR_TOOMANYCHANNELS
            err(1, "ERR_TOOMANYCHANNELS: Cannot join channel because the bot is connected to too much channels");
        } else if (tokens.size() >= 2 && tokens[1] == "436") { //ERR_NICKCOLLISION
            err(1, "ERR_NICKCOLLISION: The nickname is already used");
        } else if (tokens.size() >= 2 && tokens[1] == "442") { //ERR_NOTONCHANNEL
            err(1,
                "ERR_NOTONCHANNEL: Unable to:  perform an operation on a channel - the bot may not successfully connect to a channel");
        } else if (tokens.size() >= 2 && tokens[1] == "471") { //ERR_CHANNELISFULL
            err(1, (string("ERR_CHANNELISFULL: Cannot connect to a channel because it is full: ") + tokens[2]).c_str());
        } else if (tokens.size() >= 2 && tokens[1] == "473") { //ERR_INVITEONLYCHAN
            err(1, (string("ERR_INVITEONLYCHAN: Cannot connect to a channel because it is invite only: ") +
                    tokens[2]).c_str());
        } else if (tokens.size() >= 2 && tokens[1] == "474") { //ERR_BANNEDFROMCHAN
            err(1, (string("ERR_BANNEDFROMCHAN: Cannot connect to a channel because the bot is banned: ") +
                    tokens[2]).c_str());
        } else if (tokens.size() >= 2 && tokens[1] == "475") { //ERR_BADCHANNELKEY
            err(1, (string("ERR_BADCHANNELKEY: Cannot connect to a channel because of bad channel key: ") +
                    tokens[2]).c_str());
        }

    }
}

void Irc::handle_privmsg(string response, vector<string> tokens) {
    unsigned long privmsg_position = response.find(" PRIVMSG ");
    unsigned long user_msg_start = response.find(":", privmsg_position);

    //:fikus!~fikus@2001:67c:1220:8b4:bca1:7c2:446d:3f2e PRIVMSG #fikustest :msg
    //:tolkien.freenode.net NOTICE * :*** Looking up your hostname...
    string user_message = string(response, user_msg_start + 1); //remove the :

    if (response[0] == ':') {
        string channel_name = tokens[2];

        if (channel_name != "*") {
            if (!(channel_name[0] == '#' || channel_name[0] == '&')) {
                err(1, "Cannot read the chanel name from the privmsg");
            }

            string nickname = this->get_nickname_from_message(tokens[0]);
            cout << "nickname: >>" << nickname << "<<" << endl;

            cout << user_message << endl;

            if (user_message.find("?today") == 0) {
                this->irc_server->send_message("PRIVMSG " + channel_name + " :" + get_today_date());
            } else if (user_message.find("?msg ") == 0) {
                //?msg login:ahoj pepo
                if (user_message.find(":") != string::npos) {
                    string send_to_nickname = string(user_message, 5, user_message.find(":") - 5); //5 = length("?msg ")

                    //queue the message
                    this->add_message_for_user(send_to_nickname, channel_name,
                                               string(user_message, 5)); //5 = length("?msg ")

                    //send the queued message if the user is logged in
                    if (this->get_channel(channel_name)->is_logged(send_to_nickname)) {
                        this->send_messages_to_user(send_to_nickname, channel_name);
                    }
                }
            }

            //erase the first 3 tokens to search them in a convenient way
            tokens.erase(tokens.begin(), tokens.begin()+3);
            tokens[0] = string(tokens[0], 1);
            //if message is server notice, there is no nickname
            if (!nickname.empty() && contains_keywords(tokens, keywords)) {
                this->syslog_server->log(nickname + ": " + user_message);
            }
        }

    }
}

bool Irc::contains_keywords(vector<string> tokens, vector<string> keywords) {
    for (const string &keyword : keywords) {
        for(const string &token : tokens) {
            if (keyword == token) {
                return true;
            }
        }
    }

    return false;
}

void Irc::init_connection() {
    irc_server->send_message("USER xfajku06 xfajku06 xfajku06 xfajku06");
    irc_server->send_message("NICK xfajku06");
    irc_server->send_message("JOIN " + this->channels_string);
}

void Irc::send_names_command() {
    this->irc_server->send_message("NAMES " + this->channels_string);
}

Channel *Irc::get_channel(string &name) {

    for (int i = 0; i < this->channels.size(); ++i) {
        if (this->channels[i].name == name) {
            return &this->channels[i];
        }
    }

    Channel new_channel(name);
    this->channels.emplace_back(new_channel);

    return &this->channels.back();
}

void Irc::add_user_to_channel(string channel, string user) {
    Channel *chann = this->get_channel(channel);
    chann->add_user(user);
}

string Irc::get_nickname_from_message(string response) {
    unsigned long excl_mark_position = response.find("!");
    if (excl_mark_position == string::npos) {
        return string("");
    }
    return string(response, 1, excl_mark_position - 1);
}

void Irc::remove_user_from_channel(string channel, string user) {
    Channel *channel_object = this->get_channel(channel);
    channel_object->remove_user(user);
}

vector<string> Irc::get_messages_for_user(string user, string channel) {
    //loop through the users on the channel,
    //if the user is found, remove the record and return the messages

    return this->get_channel(channel)->get_messages_for_user(user);
}

void Irc::send_messages_to_user(string user, string channel) {
    vector<string> messages = this->get_messages_for_user(user, channel);
    for (const string &message_string : messages) {
        this->irc_server->send_message("PRIVMSG " + channel + " " + message_string);
    }
}

void Irc::add_message_for_user(string user, string channel, string message) {
    this->get_channel(channel)->add_message(user, message);


}

Irc::Irc() {
    //intentionally left blank
}

void Irc::quit() {
    this->irc_server->send_message("QUIT");
    if (!this->irc_server->close_socket() || !this->syslog_server->close_socket()) {
        cerr << "Unable to close socket";
        exit(1);
    }

}


#pragma clang diagnostic pop