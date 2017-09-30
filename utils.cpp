//
// Created by jakub on 01.10.17.
//

#include <iostream>
#include <ctime>
#include <vector>

using namespace std;


string get_today_date() {
    time_t time_number;
    struct tm *time_struct;

    char date_buf[11] = "";
    time(&time_number);
    time_struct = localtime(&time_number);
    strftime(date_buf, 11, "%d.%m.%Y", time_struct);

    return string(date_buf);
}

vector<string> explode_string(const string delimiter, string source) {
    unsigned long start_position = 0;
    unsigned long end_position = source.find(delimiter);
    vector<string> output;

    if (end_position == string::npos) {
        output.emplace_back(source);
    } else {
        do {
            string test = source.substr(start_position, end_position - start_position);
            output.emplace_back(source.substr(start_position, end_position - start_position));

            start_position = end_position;
            end_position = source.find(delimiter, start_position + 1);

            start_position++;
        } while (end_position != string::npos);

        output.emplace_back(source.substr(start_position));
    }


    return output;
}