//
// Created by jakub on 01.10.17.
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
