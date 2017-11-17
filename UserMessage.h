//
// Author: Jakub Fajkus
// Project: ISA IRC bot
// Last revision: 17.11.2017
//

#ifndef ISA_USERMESSAGE_H
#define ISA_USERMESSAGE_H

#include <iostream>

using namespace std;

class UserMessage {
public:
    string user;
    string message;

    UserMessage(const string &user, const string &message);
};


#endif //ISA_USERMESSAGE_H
