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

            //is it a PRIVMSG or NOTICE?
        } else if (tokens.size() >= 2 && (tokens[1] == "PRIVMSG" || tokens[1] == "NOTICE")) {
            handle_privmsg(response, tokens);
            continue;
        } else if (tokens.size() >= 6 && tokens[1] == "353") { //RPL_NAMEREPLY - response to NAMES command
//            :tolkien.freenode.net 353 xfajku06 = #freenode :Whiskey hexa- ChrisLane mt
            string channel_name = tokens[4];
            Channel *channel = this->get_channel(channel_name);
            //clear the logged users
            channel->users = vector<LoggedUser>();

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

//      :Anne!~Anne@annee.powered.by.lunarbnc.net JOIN #freenode
        } else if (tokens.size() >= 3 && tokens[1] == "JOIN") {
            string nickname = this->get_nickname_from_message(response);
            string channel = tokens[2];

            //add the user to the channel
            this->add_user_to_channel(channel, nickname);
            this->send_messages_to_user(nickname, channel);

        }
//        //leaving the channel
//        :fikus!~fikus@ip-89-103-184-234.net.upcbroadband.cz PART #ISAChannel :"Leaving"
        else if (tokens.size() >= 3 && tokens[1] == "PART") {
            string nickname = this->get_nickname_from_message(response);
            vector<string> channels = explode_string(",", tokens[2]);

            for (const string &channel_name : channels) {
                this->remove_user_from_channel(channel_name, nickname);
            }
            //:fikus!~fikus@ip-89-103-184-234.net.upcbroadband.cz QUIT :Client Quit
        } else if (tokens.size() >= 3 && tokens[1] == "QUIT") {

            string nickname = this->get_nickname_from_message(response);

            for (Channel &channel : this->channels) {
                channel.remove_user(nickname);
            }
            //:fikus!~fikus@ip-89-103-184-234.net.upcbroadband.cz KICK #fikustest fikus2 :fikus2
        } else if (tokens.size() >= 4 && tokens[1] == "KICK") {
            this->remove_user_from_channel(tokens[2], tokens[3]);
        }

        //todo: prejmenovani usera!
//        :fikus!~fikus@ip-89-103-184-234.net.upcbroadband.cz NICK :fikuss

    }
}

void Irc::handle_privmsg(string response, vector<string> tokens) {
    unsigned long privmsg_position = response.find(" PRIVMSG ");
    unsigned long user_msg_start = response.find(":", privmsg_position);

    if (user_msg_start != string::npos) {
        string msg(response, user_msg_start + 1);

        if (response[0] == ':') {
            string nickname = this->get_nickname_from_message(response);
            cout << "nickname: >>" << nickname << "<<" << endl;

            cout << msg << endl;

            string channel_name = tokens[2];
            if (!(channel_name[0] == '#' || channel_name[0] == '&')) {
                err(1, "Cannot read the chanel name from the privmsg");
            }

            if (msg.find("?today") == 0) {
                this->irc_server->send_message("PRIVMSG " + channel_name + " :" + get_today_date());
            } else if (msg.find("?msg ") == 0) {
                //?msg login:ahoj pepo
                if (msg.find(":") != string::npos) {
                    string send_to_nickname = string(msg, 5, msg.find(":") - 5);

                    //queue the message
                    this->add_message_for_user(send_to_nickname, channel_name, string(msg, 5)); //5 = length("?msg ")

                    //send the queued message if the user is logged in
                    if (this->get_channel(channel_name)->is_logged(send_to_nickname)) {
                        this->send_messages_to_user(send_to_nickname, channel_name);
                    }
                }
            }

            if (contains_keywords(response, keywords)) {
                this->syslog_server->log("<" + nickname + ">: " + msg);
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
    return string(response, 1, excl_mark_position - 1);
}

void Irc::remove_user_from_channel(string channel, string user) {
    Channel *channel_object = this->get_channel(channel);
    channel_object->remove_user(user);
}

vector<string> Irc::get_messages_for_user(string user, string channel) {
    //loop through the users on the channel,
    //if the user is found, remove the record and return the messages

    vector<string> unsent_messages;
    vector<UserMessage> *messages = &this->get_channel(channel)->messages;

    //get all the messages for the user and remove them

    for (int j = 0; j < messages->size(); ++j) {
        UserMessage &message = (*messages)[j];

        if (message.user == user) {
            unsent_messages.emplace_back(message.message);
            messages->erase(messages->begin() + j); //remove the j-th element
            --j;//because of the message erasing, the j-th element must be processed once again as the array was shifted
        }
    }

    return unsent_messages;
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


#pragma clang diagnostic pop