//
// Author: Jakub Fajkus
// Project: ISA IRC bot
// Last revision: 17.11.2017
//

#ifndef ISA_LOGGEDUSER_H
#define ISA_LOGGEDUSER_H

#include <iostream>

using namespace std;

class LoggedUser {
public:
    string nickname;

    explicit LoggedUser(const string &nickname);
};


#endif //ISA_LOGGEDUSER_H
