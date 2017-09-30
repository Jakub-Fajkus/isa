#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <ctime>

#include <stdlib.h>
#include <arpa/inet.h>
#include <strings.h>
#include <cstring>
#include <vector>
#include <unistd.h>
#include <err.h>

#include "utils.h"
#include "SyslogServer.h"
#include "IrcServer.h"
#include "Irc.h"

#define BUFSIZE 1025


/* NOTES:
 * UDP syslog packet consists of: PRI, HEADER, MSG
 * IT'S LENGTH MUST BE 1024 BYTES OR LESS
 * PRI: uzavrena v <>, obsahuje 3-5 znaku
 * v nasem pripade: 16 a 6 =  <134>
 *
 * HOSTNAME obsahuje IP adresu stroje
 * V MSG muzou byt pouze ascii znaky todo: diakritika?
 *
 *
 *
 *
 * IRC: IRC zprava se sklada az ze 3 casti, ktere jsou oddeleny mezerou:
 * prefix(volitenlne) - pokud je ve zprave, zprava zacina :
 * command - validni IRC command, nebo 3 ascii cislice
 * command parameters
 *
 * klienti nemaji posilat prefix ve svych zpravach!!
 *
 * kazda zprava je omezena na 512 znaku, vcetne crlf(ktere je na konci kazde zpravy), takze zbyva 510 znaku na command a jeho parametry
 * crlf slouzi k rozdeleni streamu oktetu na jednotlive zpravy
 * prazdne zpravy jsou zahozeny
 *
 *
 *
 *
 */


struct send_msg {
    std::string user;
    std::string message;
    std::string channel;
};

using namespace std;

void log(int syslog_socket, struct sockaddr_in syslog_address, const string user_message);


vector<string> explode_string(const string delimiter, string source);

bool parse_parameters(int argc, char *argv[], string *ircHost, int *ircPort, string *channels, string *syslogServer,
                      vector<string> &keywords);


void send_message(string message, int socket);

string get_today_date();

void handle_privmsg(string response, unsigned long privmsg_position, vector<string> tokens, vector<string> keywords,
                    int irc_socket, int syslog_socket, struct sockaddr_in syslog_address);

bool contains_keywords(const string &message, vector<string> keywords);

int main(int argc, char *argv[]) {
    using namespace std;

    struct sockaddr_in syslog_server_address;
    int irc_socket;
    string irc_host;
    int irc_port;
    vector<string> keywords;
    string syslog_server_name, channels;


    if (!parse_parameters(argc, argv, &irc_host, &irc_port, &channels, &syslog_server_name, keywords) ||
        irc_port == -1) {
        cerr << "Invalid parameters";
        return 1;
    }

    SyslogServer *syslog_server = new SyslogServer(syslog_server_name);
    IrcServer *irc_server = new IrcServer(irc_host, irc_port);
    Irc *irc = new Irc(irc_server, syslog_server, keywords);


//    cout << "PREPARE!:" << endl;
//    for (auto&& i : channels) std::cout << i << ' ';
//    for (auto&& i : keywords) std::cout << i << ' ';
//


    irc->init_connection(channels);
    irc->listen();


    return 0;
}

//isabot HOST[:PORT] CHANNELS [-s SYSLOG_SERVER] [-l HIGHLIGHT] [-h|--help]
bool parse_parameters(int argc, char *argv[], string *ircHost, int *ircPort, string *channels,
                      string *syslogServer,
                      vector<string> &keywords) {

    *ircPort = -1;
    *syslogServer = "";

    //program name + 2 parameters, both with value
    if (argc < 3) {
        fprintf(stderr, "Invalid arguments - not enough arguments");
        return false;
    }

    for (int i = 1; i < argc; ++i) {
        string current = argv[i];

        if (i == 1 || i == 2) {
            if (current.find('-') != string::npos) { //contains the -
                return false;
            }
        }

        if (i == 1) {
            unsigned long position = current.find(':');
            if (position == string::npos) { //not found, hurray
                *ircHost = current; //todo: memory leak? does it copy or just assign?
            } else {
                *ircHost = current.substr(0, position);
                *ircPort = atoi(current.substr(position + 1).c_str());

                if (*ircPort == 0) {
                    return false;
                }

            }
        } else if (i == 2) {
            *channels = current;

            for (string str : explode_string(string(","), current)) {
                if (str.length() > 0 && !(str[0] == '#' || str[0] == '&')) {
                    cerr << "Invalid channel " << str;
                    exit(1);
                }
            }
        } else {
            if (current == "-s") {
                *syslogServer = argv[i + 1];
                i++;
            } else if (current == "-l") {
                keywords = explode_string(",", string(argv[i + 1]));
                i++;
            } else if (current == "-h" || current == "--help") {
                cout << "Printing some help!" << endl; //todo: add some help...
                exit(0);
            }
        }

    }

    if (syslogServer->empty()) {
        *syslogServer = string("127.0.0.1");
    }

//    for (auto&& i : channels) std::cout << i << ' ';
    for (auto &&i : keywords) std::cout << i << ' ';

    return true;
}


