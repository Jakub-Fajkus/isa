//
// Author: Jakub Fajkus
// Project: ISA IRC bot
// Last revision: 17.11.2017
//

#ifndef ISA_CHANNEL_H
#define ISA_CHANNEL_H

#include <iostream>
#include <vector>
#include "LoggedUser.h"
#include "UserMessage.h"

using namespace std;

class Channel {
private:
    vector<LoggedUser> users;
    vector<UserMessage> messages;
public:
    string name;

    explicit Channel(string &name);

    /**
     * Add the nickname to the currently logged in users
     *
     * If the user with the nickname is already logged, it is not added
     *
     * @param nickname Nickname of the user
     */
    void add_user(string nickname);

    /**
     * Remove the nickname from the currently logged users
     *
     * If the nickname is not found, no error is reported
     *
     * @param nickname Nickname of the user
     */
    void remove_user(string nickname);

    /**
     * Check, if the user with the nickname is logged
     *
     * @param nickname Nickname of the user
     *
     * @return True if the user is in the logged users, false otehrwise
     */
    bool is_logged(string nickname);

    /**
     * Get all messages, which are queued to be sent to the user with the nickname given.
     *
     * The messages returned are then removed from the queue.
     *
     * @param nickname Nickname of the user
     *
     * @return Vector of strings. Each string is one queued message.
     */
    vector<string> get_messages_for_user(string nickname);


    /**
     * Queues the given message for an user with the given nickname
     *
     * @param nickname Nickname of the user
     * @param message Message text
     */
    void add_message(string nickname, string message);
};


#endif //ISA_CHANNEL_H
