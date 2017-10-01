//
// Created by jakub on 01.10.17.
//

#include "Channel.h"

Channel::Channel(string &name) : name(name) {}

void Channel::add_user(string nickname) {
    //check if the channel is already there
    for(const LoggedUser &user : this->users) {
        if (user.nickname == nickname) {
            return;
        }
    }

    LoggedUser logged_user(nickname);

    this->users.emplace_back(logged_user);
}

void Channel::remove_user(string user) {

}

void Channel::get_user(string user) {

}

void Channel::add_message(string user, string message) {
    int size = this->messages.size();
    this->messages.emplace_back(UserMessage(user, message));
    size = this->messages.size();
    size = this->messages.size();
}

void Channel::get_mesage(string user, string message) {

}

void Channel::remove_message(UserMessage message) {

}
