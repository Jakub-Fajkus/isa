//
// Author: Jakub Fajkus
// Project: ISA IRC bot
// Last revision: 17.11.2017
//

#include "Channel.h"

Channel::Channel(string &name) : name(name) {}

void Channel::add_user(string nickname) {
    //check if the user is already there
    //if it is, do not add him again
    for (const LoggedUser &user : this->users) {
        if (user.nickname == nickname) {
            return;
        }
    }

    //the user is not in the logged users
    //so, add it there
    LoggedUser logged_user(nickname);
    this->users.emplace_back(logged_user);
}

void Channel::remove_user(string nickname) {
    //look for the nickname in all the users
    for (int j = 0; j < this->users.size(); ++j) {
        LoggedUser &logged_user = this->users[j];

        //if the nickname is the same
        if (logged_user.nickname == nickname) {
            //remove the j-th element, which is the nickname we are looking for
            this->users.erase(this->users.begin() + j);
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

vector<string> Channel::get_messages_for_user(string user) {
    //set up the vector for the messages
    vector<string> unsent_messages;

    //get all the messages for the user and remove them
    for (int j = 0; j < this->messages.size(); ++j) {
        UserMessage &message = this->messages[j];

        //if the message is for the user
        if (message.user == user) {
            //add it to the list which will be returned
            unsent_messages.emplace_back(message.message);

            //remove the j-th element, which is the message added to the vector
            this->messages.erase(this->messages.begin() + j);

            //because of the message erasing, the j-th element must be processed once again as the array was shifted
            --j;
        }
    }

    return unsent_messages;
}
