//
// Author: Jakub Fajkus
// Project: ISA IRC bot
// Last revision: 18.11.2017
//

#include "Irc.h"
#include <algorithm>


Irc::Irc(IrcServer *irc_server, SyslogServer *syslog_server, vector<string> keywords, string channels)
        : irc_server(irc_server), syslog_server(syslog_server), keywords(std::move(keywords)),
          channels_string(channels) {

    //create all channel objects based on the channels string(coma separated channel names)
    for (string &channel : explode_string(",", channels)) {
        Channel new_channel(channel);

        //creates a channel if it does not exist
        this->get_channel(channel);
    }
}

void Irc::listen() {
    //the main program loop
    while (true) {
        //get the irc server message
        string response = this->irc_server->read_message();
        //split the message by space
        vector<string> tokens = explode_string(" ", response);

        //PING command
        if (tokens.size() >= 2 && tokens[0] == "PING") {
            this->irc_server->send_message(string("PONG ") + explode_string(":", tokens[1])[1]);
        } else if (tokens.size() >= 2 && (tokens[1] == "PRIVMSG" || tokens[1] == "NOTICE")) {
            handle_privmsg(response, tokens);
        } else if (tokens.size() >= 6 && tokens[1] == "353") { //RPL_NAMEREPLY - response to NAMES command
            this->handle_name_reply(tokens);
        } else if (tokens.size() >= 3 && tokens[1] == "JOIN") {
            this->handle_join(response, tokens);
        } else if (tokens.size() >= 3 && tokens[1] == "PART") {
            this->handle_part(response, tokens);
        } else if (tokens.size() >= 3 && tokens[1] == "QUIT") {
            this->handle_quit(response);
        } else if (tokens.size() >= 4 && tokens[1] == "KICK" && tokens[3] == "xfajku06") {
//            :user!~user@2001:67c:1220:8b4:bca1:7c2:446d:3f2e KICK #test xfajku06 :xfajku06
            throw "The bot was kicked";
        } else if (tokens.size() >= 4 && tokens[1] == "KICK") {
            this->handle_kick(tokens);
        } else if (tokens.size() >= 2 && tokens[1] == "NICK") {
            this->handle_nick(response, tokens);
        } else {
            this->handle_error_messages(tokens);
        }
    }
}


void Irc::handle_nick(string response, vector<string> tokens) {
    //:user!~user@ip-89-103-184-234.net.upcbroadband.cz NICK :users
    string old_nickname = get_nickname_from_message(response);
    string new_nickname = string(tokens[2], 1); //remove the :

    //change the users nick in all channels
    for (Channel &channel : this->channels) {
        channel.remove_user(old_nickname);
        channel.add_user(new_nickname);

        //send all queued messages for the new user's nickname
        this->send_messages_to_user(new_nickname, channel.name);
    }

}

void Irc::handle_kick(vector<string> tokens) {
    //:user!~user@ip-89-103-184-234.net.upcbroadband.cz KICK #usertest user2 :user2
    this->remove_user_from_channel(tokens[2], tokens[3]);

}

void Irc::handle_quit(string response) {
    //:user!~user@ip-89-103-184-234.net.upcbroadband.cz QUIT :Client Quit
    string nickname = this->get_nickname_from_message(response);

    //remove user from all channels
    for (Channel &channel : this->channels) {
        channel.remove_user(nickname);
    }
}

void Irc::handle_part(string response, vector<string> tokens) {
    //leaving the channel
    //:user!~user@ip-89-103-184-234.net.upcbroadband.cz PART #ISAChannel :"Leaving"
    string nickname = this->get_nickname_from_message(response);
    vector<string> channels = explode_string(",", tokens[2]);

    //remove user from all specified channels
    for (const string &channel_name : channels) {
        this->remove_user_from_channel(channel_name, nickname);
    }
}

void Irc::handle_join(string response, vector<string> tokens) {
    //:Anne!~Anne@annee.powered.by.lunarbnc.net JOIN #freenode
    string nickname = this->get_nickname_from_message(response);
    string channel = tokens[2];

    //add the user to the channel
    this->add_user_to_channel(channel, nickname);
    //send all queued messages for the user
    this->send_messages_to_user(nickname, channel);
}

void Irc::handle_name_reply(vector<string> tokens) {
    //:tolkien.freenode.net 353 xfajku06 = #freenode :Whiskey hexa- ChrisLane mt
    string channel_name = tokens[4];

    //remove the : from the first user name
    tokens[5] = string(tokens[5], 1);

    //add all users
    for (int i = 5; i < tokens.size(); ++i) {

        //is it the channel operator?
        if (tokens[i][0] == '@') {
            tokens[i] = string(tokens[i], 1); //remove the first character
        }

        //add the user to the channel
        this->add_user_to_channel(channel_name, tokens[i]);
        //send all queued messages for the user
        this->send_messages_to_user(tokens[i], channel_name);
    }
}

void Irc::handle_error_messages(vector<string> tokens) {

    //all error messages have at least 2 tokens
    if (tokens.size() < 2) {
        return;
    }

    if (tokens[1] == "433") { //nickname used
        throw "The bot's nickname(xfajku06) is already used";
    } else if (tokens[1] == "465") { //bot banned
        throw "The bot is banned";
    } else if (tokens[1] == "403") { //ERR_NOSUCHCHANNEL
        //:hobana.freenode.net 403 xfajku06 &fikustest :No such channel
        throw string("ERR_CANNOTSENDTOCHAN: Cannot connect to channel because it does not exist: ") + tokens[3];
    } else if (tokens[1] == "404") { //ERR_CANNOTSENDTOCHAN
        throw string("ERR_CANNOTSENDTOCHAN: Cannot send a message to channel: ") + tokens[2];
    } else if (tokens[1] == "405") { //ERR_TOOMANYCHANNELS
        throw "ERR_TOOMANYCHANNELS: Cannot join channel because the bot is connected to too much channels";
    } else if (tokens[1] == "436") { //ERR_NICKCOLLISION
        throw "ERR_NICKCOLLISION: The nickname is already used";
    } else if (tokens[1] == "442") { //ERR_NOTONCHANNEL
        throw "ERR_NOTONCHANNEL: Unable to perform an operation on a channel - the bot may not successfully connect to a channel";
    } else if (tokens[1] == "471") { //ERR_CHANNELISFULL
        throw string("ERR_CHANNELISFULL: Cannot connect to a channel because it is full: ") + tokens[2];
    } else if (tokens[1] == "473") { //ERR_INVITEONLYCHAN
        throw string("ERR_INVITEONLYCHAN: Cannot connect to a channel because it is invite only: ") + tokens[2];
    } else if (tokens[1] == "474") { //ERR_BANNEDFROMCHAN
        throw string("ERR_BANNEDFROMCHAN: Cannot connect to a channel because the bot is banned: ") + tokens[2];
    } else if (tokens[1] == "475") { //ERR_BADCHANNELKEY
        throw string("ERR_BADCHANNELKEY: Cannot connect to a channel because of bad channel key: ") + tokens[2];
    }
}

void Irc::handle_privmsg(string response, vector<string> tokens) {
    unsigned long privmsg_position = response.find(" PRIVMSG ");
    //index to the string when the actual user's message starts
    unsigned long user_msg_start = response.find(":", privmsg_position);

    //:user!~user@2001:67c:1220:8b4:bca1:7c2:446d:3f2e PRIVMSG #usertest :msg
    //:tolkien.freenode.net NOTICE * :*** Looking up your hostname...
    string user_message = string(response, user_msg_start + 1); //remove the :

    if (response[0] == ':') {
        string channel_name = tokens[2];

        if (channel_name != "*") {
            if (!(channel_name[0] == '#' || channel_name[0] == '&')) {
                throw string("Cannot read the chanel name from the privmsg");
            }

            string nickname = this->get_nickname_from_message(tokens[0]);

            //check, if it is a special message
            if (user_message == "?today") {
                this->irc_server->send_message("PRIVMSG " + channel_name + " :" + get_today_date());
            } else if (user_message.find("?msg ") == 0) {
                //?msg login:hi
                if (user_message.find(":") != string::npos) {
                    //get the nickname from the message
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
            tokens.erase(tokens.begin(), tokens.begin() + 3);
            tokens[0] = string(tokens[0], 1);
            //if message is server notice, there is no nickname
            if (!nickname.empty() && contains_keywords(tokens, keywords)) {
                this->syslog_server->log(nickname + ": " + user_message);
            }
        }
    }
}

bool Irc::contains_keywords(vector<string> tokens, vector<string> keywords) {
    //check if the given tokens contain the keywords
    for (const string &keyword : keywords) {
        for (const string &token : tokens) {
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

Channel *Irc::get_channel(string &name) {
    //look if the channel object is created
    //if so, return it
    for (int i = 0; i < this->channels.size(); ++i) {
        if (this->channels[i].name == name) {
            return &this->channels[i];
        }
    }

    //if the channel was not created, create it and save it to the channels
    Channel new_channel(name);
    this->channels.emplace_back(new_channel);

    //return the new channel
    return &this->channels.back();
}

void Irc::add_user_to_channel(string channel, string user) {
    Channel *chann = this->get_channel(channel);
    chann->add_user(user);
}

string Irc::get_nickname_from_message(string message) {
    //:Anne!~Anne@annee.powered.by.lunarbnc.net JOIN #freenode
    unsigned long excl_mark_position = message.find("!");
    //if the exclamation mark is not found, no username is returned
    if (excl_mark_position == string::npos) {
        return string("");
    }
    //get the username between the : and !
    return string(message, 1, excl_mark_position - 1);
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
    //get all queues messages for the user
    vector<string> messages = this->get_messages_for_user(user, channel);

    //send the messages one by one
    for (const string &message_string : messages) {
        this->irc_server->send_message("PRIVMSG " + channel + " :" + message_string);
    }
}

void Irc::add_message_for_user(string user, string channel, string message) {
    this->get_channel(channel)->add_message(user, message);


}


void Irc::quit() {
    //try to close the irc connection
    this->irc_server->send_message("QUIT");
    if (!this->irc_server->close_socket()) {
        cerr << "Unable to close irc socket";
    }

    //try to close the syslog socket
    if (!this->syslog_server->close_socket()) {
        cerr << "Unable to close syslog socket";
    }
}

