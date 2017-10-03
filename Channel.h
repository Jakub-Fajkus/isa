//
// Created by jakub on 01.10.17.
//

#ifndef ISA_CHANNEL_H
#define ISA_CHANNEL_H

#include <iostream>
#include <vector>
#include "LoggedUser.h"
#include "UserMessage.h"

using namespace std;

class Channel {
public:
    string name;
    vector<LoggedUser> users;
    vector<UserMessage> messages;

    explicit Channel(string &name);
    void add_user(string nickname);
    void remove_user(string user);
    bool is_logged(string user);

    void add_message(string user, string message);
    void get_mesage(string user, string message);
    void remove_message(UserMessage message);
};


#endif //ISA_CHANNEL_H
