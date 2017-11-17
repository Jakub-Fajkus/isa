//
// Author: Jakub Fajkus
// Project: ISA IRC bot
// Last revision: 17.11.2017
//


#ifndef ISA_UTILS_H
#define ISA_UTILS_H

#include <iostream>
#include <ctime>
#include <vector>


using namespace std;

/**
 * Ge the today date in format: dd.mm.yyyy
 *
 * @return Today date in string
 */
string get_today_date();

/**
 * Splits the string given by the delimiter
 *
 * @param delimiter Delimiting string
 * @param source String to be split
 *
 * @return Vector of strings
 */
vector<string> explode_string(const string delimiter, string source);

#endif //ISA_UTILS_H
