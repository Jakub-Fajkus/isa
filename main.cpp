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
#include <cstdlib>
#include <signal.h>

#include "utils.h"
#include "SyslogServer.h"
#include "IrcServer.h"
#include "Irc.h"

using namespace std;


bool parse_parameters(int argc, char *argv[], string *ircHost, int *ircPort, string *channels, string *syslogServer,
                      vector<string> &keywords);

Irc* irc;

static void handler(int signum)
{
    irc->quit();
    exit(0);
}

int main(int argc, char *argv[]) {
    struct sigaction sa{};

    sa.sa_handler = handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART; /* Restart functions if interrupted by handler */
    if (sigaction(SIGINT, &sa, nullptr) == -1) {
        cerr << "Cannot register signal";
        exit(1);
    }

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
    irc = new Irc(irc_server, syslog_server, keywords, channels);

    irc->init_connection();
    irc->listen();

    return 0;
}

//isabot HOST[:PORT] CHANNELS [-s SYSLOG_SERVER] [-l HIGHLIGHT] [-h|--help]
bool parse_parameters(int argc, char *argv[], string *ircHost, int *ircPort, string *channels,
                      string *syslogServer,
                      vector<string> &keywords) {

    *ircPort = -1;
    *syslogServer = "";

    for(int i = 1; i < argc; ++i) {
        string current = argv[i];

        if (current == "-h" || current == "--help") {
            cout << "isabot HOST[:PORT] CHANNELS [-s SYSLOG_SERVER] [-l HIGHLIGHT] [-h|--help]\n"
                    "HOST je název serveru (např. irc.freenode.net)\n"
                    "PORT je číslo portu, na kterém server naslouchá (výchozí 6667)\n"
                    "CHANNELS obsahuje název jednoho či více kanálů, na které se klient připojí (název kanálu je zadán včetně úvodního # nebo &; v případě více kanálů jsou tyto odděleny čárkou)\n"
                    "-s SYSLOG_SERVER je ip adresa logovacího (SYSLOG) serveru\n"
                    "-l HIGHLIGHT seznam klíčových slov oddělených čárkou (např. \"ip,tcp,udp,isa\")\n";
            exit(0);
        }
    }

    //program name + 2 parameters, both with value
    if (argc < 3) {
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
                *ircHost = current;
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
            }
        }

    }

    if (syslogServer->empty()) {
        *syslogServer = string("127.0.0.1");
    }

    return true;
}


