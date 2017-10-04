//
// Created by jakub on 01.10.17.
//

#include "Channel.h"

Channel::Channel(string &name) : name(name) {}

void Channel::add_user(string nickname) {
    //check if the user is already there
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

vector<string> Channel::get_messages_for_user(string user) {
    vector<string> unsent_messages;

    //get all the messages for the user and remove them
    for (int j = 0; j < this->messages.size(); ++j) {
        UserMessage &message = this->messages[j];

        if (message.user == user) {
            unsent_messages.emplace_back(message.message);
            this->messages.erase(this->messages.begin() + j); //remove the j-th element
            --j;//because of the message erasing, the j-th element must be processed once again as the array was shifted
        }
    }

    return unsent_messages;
}
