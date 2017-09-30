#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <ctime>

#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <strings.h>
#include <string>
#include <cstring>
#include <ifaddrs.h>
#include <vector>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<netinet/in.h>
#include<unistd.h>
#include<netdb.h>
#include<err.h>

#define BUFSIZE 1025
#define DATE_FORMAT_LENGTH 16 //Mmm dd hh:mm:ss

const char *months[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

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

void log(int client_socket, struct sockaddr_in server_address, const string user_message);

int create_syslog_socket(const string server_hostname, struct sockaddr_in *server_address);

int create_irc_socket(const string server_hostname, const int port, struct sockaddr_in *server_address);

vector<string> explode_string(const string delimiter, string source);

bool parse_parameters(int argc, char *argv[], string *ircHost, int *ircPort, string *channels,
                      string *syslogServer,
                      vector<string> keywords);

char* get_local_ip(int sock);

void send_message(string message, int socket);

string read_message(int socket);

string get_today_date();

void handle_privmsg(string response, unsigned long privmsg_position, vector<string> tokens, int irc_socket) ;

int main(int argc, char *argv[]) {
    using namespace std;

    struct sockaddr_in syslog_server_address, irc_server_address;
    int syslog_socket, irc_socket;
    string irc_host;
    int irc_port;
    vector<string> keywords;
    string syslog_server, channels;


    if (!parse_parameters(argc, argv, &irc_host, &irc_port, &channels, &syslog_server, keywords) || irc_port == -1){
        cerr << "Invalid parameters";
        return 1;
    }


//    cout << "PREPARE!:" << endl;
//    for (auto&& i : channels) std::cout << i << ' ';
//    for (auto&& i : keywords) std::cout << i << ' ';
//



    syslog_socket = create_syslog_socket(syslog_server, &syslog_server_address);

    log(syslog_socket, syslog_server_address, "<xfajku06>: neco to dela");

    irc_socket = create_irc_socket(irc_host, irc_port, &irc_server_address);

    if (-1 == connect(irc_socket, (struct sockaddr *)&irc_server_address, sizeof(irc_server_address))) {
        cerr << "Cannot connect to the IRC socket" << endl;
    }

    string user_message = "USER xfajku06 xfajku06 xfajku06 xfajku06";
    send_message(user_message, irc_socket);

    send_message("NICK xfajku06", irc_socket);

    send_message("JOIN " + channels, irc_socket);

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-noreturn"
    while(true) {
        string response = read_message(irc_socket);
        vector<string> tokens = explode_string(" ", response);

        //ping command
        if (tokens.size() >= 2 && tokens[0] == "PING") {
            send_message(string("PONG ") + explode_string(":",tokens[1])[1], irc_socket);
            continue;
        }

//        //is it a PRIVMSG ?
        unsigned long privmsg_position = response.find(" PRIVMSG ");
        if (privmsg_position != string::npos) {
            handle_privmsg(response, privmsg_position, tokens, irc_socket);
            continue;
        }

    }
#pragma clang diagnostic pop



    return 0;
}

void handle_privmsg(string response, unsigned long privmsg_position, vector<string> tokens, int irc_socket) {
    unsigned long user_msg_start = response.find(":", privmsg_position);
    if (user_msg_start != string::npos) {
        string msg(response, user_msg_start+1);

        if (response[0] == ':') {
            unsigned long excl_mark_position = response.find("!");
            string nickname(response, 1, excl_mark_position -1);
            cout << "nickname: >>" << nickname << "<<" << endl;

            cout << msg << endl;

            string channel_name = tokens[2];
            if (!(channel_name[0] == '#' || channel_name[0] == '&')) {
                err(1, "Cannot read the chanel name from the privmsg");
            }

            if (msg.find("?today") == 0) {
                string message = string("PRIVMSG " + channel_name + " :" + get_today_date());
                send_message(message, irc_socket);
            } else if (msg.find("?msg") == 0) {

            }
        }

    }
}

string get_today_date() {
    time_t time_number;
    struct tm *time_struct;

    char date_buf[11] = "";
    time(&time_number);
    time_struct = localtime(&time_number);
    strftime(date_buf, 11, "%d.%m.%Y", time_struct);

    return string(date_buf);
}

void send_message(string message, int socket) {
    message += "\r\n";

    int i = write(socket,message.c_str(),message.length());                    // send data to the server
    if (i == -1)                                 // check if data was sent correctly
        err(1,"write() failed");
    else if (i != message.length())
        err(1,"write(): buffer written partially");
    else
        cout << "Sent: " << message<<endl;
}

string read_message(int socket) {
    char buffer[513] = "";
    ssize_t i;

    unsigned int read_chars = 0;
    while (true) {
        i = read(socket, buffer + read_chars, 1);
        if (i == -1) {
            err(1, "read() failed");
        }

        //if there is an CR LF pair, stop reading!
        if (read_chars >= 1 && buffer[read_chars] == '\n' && buffer[read_chars-1] == '\r') {

            cout << "RECEIVED: >>>" << string(buffer, read_chars - 1) << "<<<" << endl;

            return string(buffer, read_chars - 1);
        }

        read_chars++;
    }
}

char* get_local_ip(int sock) {
    struct sockaddr_in local;
    socklen_t len;

    memset(&local,0,sizeof(local));   // erase the local address structure

    // obtain the local IP address and port using getsockname()
    len = sizeof(local);
    if (getsockname(sock,(struct sockaddr *) &local, &len) == -1) {
        cerr << "getsockname() failed";
        exit(1);
    }

    return inet_ntoa(local.sin_addr);
}

void log(int client_socket, struct sockaddr_in server_address, const string user_message) {

    int bytestx, bytesrx;
    socklen_t serverlen;
    string buf;
    char dateBuf[DATE_FORMAT_LENGTH];
    time_t time_number;
    struct tm *time_struct;

    time(&time_number);
    time_struct = localtime(&time_number);
    strftime(dateBuf, DATE_FORMAT_LENGTH, " %e %T", time_struct);
    buf = months[time_struct->tm_mon] + string(dateBuf) + " ";

    buf += get_local_ip(client_socket);
    buf += " isabot " + user_message;

    cout << endl << endl <<buf << endl << endl;

/* odeslani zpravy na server */
    serverlen = sizeof(server_address);
    bytestx = sendto(client_socket, buf.c_str(), buf.length() > 1024 ? 1024 : buf.length(), 0,
                     (struct sockaddr *) &server_address, serverlen);
    if (bytestx < 0)
        perror("ERROR: sendto");
}

int create_syslog_socket(const string server_hostname, struct sockaddr_in *server_address) {
    struct hostent *server;
    int syslog_socket;


    /* 2. ziskani adresy serveru pomoci DNS */
    if ((server = gethostbyname(server_hostname.c_str())) == nullptr) {
        cerr << "ERROR: no such host: " << server_hostname << endl;
        exit(EXIT_FAILURE);
    }

/* 3. nalezeni IP adresy serveru a inicializace struktury server_address */
    bzero((char *) server_address, sizeof(*server_address));
    server_address->sin_family = AF_INET;
    bcopy((char *) server->h_addr, (char *) &(server_address->sin_addr.s_addr), server->h_length);
    server_address->sin_port = htons(514);

/* Vytvoreni soketu */
    if ((syslog_socket = socket(AF_INET, SOCK_DGRAM, 0)) <= 0) {
        perror("ERROR: socket syslog");
        exit(EXIT_FAILURE);
    }

    return syslog_socket;
}

int create_irc_socket(const string server_hostname, const int port, struct sockaddr_in *server_address) {
    struct hostent *server;
    int irc_socket;

    /* 2. ziskani adresy serveru pomoci DNS */
    if ((server = gethostbyname(server_hostname.c_str())) == nullptr) {
        cerr << "ERROR: no such host: " << server_hostname << endl;
        exit(EXIT_FAILURE);
    }

/* 3. nalezeni IP adresy serveru a inicializace struktury server_address */
    bzero((char *) server_address, sizeof(*server_address));
    server_address->sin_family = AF_INET;
    bcopy((char *) server->h_addr, (char *) &(server_address->sin_addr.s_addr), server->h_length);
    server_address->sin_port = htons(port);

/* Vytvoreni soketu */
    if ((irc_socket = socket(AF_INET, SOCK_STREAM, 0)) <= 0) {
        perror("ERROR: socket irc");
        exit(EXIT_FAILURE);
    }

    return irc_socket;
}


//isabot HOST[:PORT] CHANNELS [-s SYSLOG_SERVER] [-l HIGHLIGHT] [-h|--help]
bool parse_parameters(int argc, char *argv[], string *ircHost, int *ircPort, string *channels,
                      string *syslogServer,
                      vector<string> keywords) {

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
                *ircPort = atoi(current.substr(position+1).c_str());

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
                keywords = explode_string(",", string(argv[i+1]));
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
//    for (auto&& i : keywords) std::cout << i << ' ';

    return true;
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
