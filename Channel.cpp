//
// Created by jakub on 01.10.17.
//

#include "Channel.h"

Channel::Channel(string &name) : name(name) {}

void Channel::add_user(string nickname) {
    //check if the channel is already there
    for (const LoggedUser &user : this->users) {
        if (user.nickname == nickname) {
            return;
        }
    }

    LoggedUser logged_user(nickname);

    this->users.emplace_back(logged_user);
}

void Channel::remove_user(string user) {
    for (int j = 0; j < this->users.size(); ++j) {
        LoggedUser &logged_user = this->users[j];

        if (logged_user.nickname == user) {
            this->users.erase(this->users.begin() + j); //remove the j-th element
        }
    }
}

bool Channel::is_logged(string user) {
//get user if is logged or null, if not
    for (const LoggedUser &user_object : this->users) {
        if (user_object.nickname == user) {
            return true;
        }
    }

    return false;
}

void Channel::add_message(string user, string message) {
    this->messages.emplace_back(UserMessage(user, message));
}

void Channel::get_mesage(string user, string message) {

}

void Channel::remove_message(UserMessage message) {

}
