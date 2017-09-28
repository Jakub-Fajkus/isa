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
 */

using namespace std;

void log(int client_socket, struct sockaddr_in server_address, const string user_message);

int createSocket(const char *server_hostname, struct sockaddr_in *server_address);

vector<string> explode_string(const string delimiter, string source);

bool parse_parameters(int argc, char *argv[], string *ircHost, int *ircPort, std::vector<std::string> channels,
                     string *syslogServer, vector<string> keywords);

int main(int argc, char *argv[]) {
    using namespace std;

    struct sockaddr_in server_address;
    int syslog_socket;
    string irc_host;
    int irc_port;
    vector<string> channels, keywords;
    string syslog_server;


    if (!parse_parameters(argc, argv, &irc_host, &irc_port, channels, &syslog_server, keywords) || irc_port == -1){
        cerr << "Invalid parameters";
        return 1;
    }


//    cout << "PREPARE!:" << endl;
//    for (auto&& i : channels) std::cout << i << ' ';
//    for (auto&& i : keywords) std::cout << i << ' ';
//


    syslog_socket = createSocket("127.0.0.1", &server_address); //todo: get from params!
    log(syslog_socket, server_address, "<xfajku06>: neco to dela");


///* prijeti odpovedi a jeji vypsani */
//    bytesrx = recvfrom(client_socket, buf, BUFSIZE, 0, (struct sockaddr *) &server_address, &serverlen);
//    if (bytesrx < 0)
//        perror("ERROR: recvfrom");
//    printf("Echo from server: %s", buf);
    return 0;
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

    cout << buf; //todo: remove me

    buf += "192.168.0.1 "; //todo: either get from parameters or use get_machine_ip_address
    buf += "isabot " + user_message;

/* odeslani zpravy na server */
    serverlen = sizeof(server_address);
    bytestx = sendto(client_socket, buf.c_str(), buf.length() > 1024 ? 1024 : buf.length(), 0,
                     (struct sockaddr *) &server_address, serverlen);
    if (bytestx < 0)
        perror("ERROR: sendto");
}

int createSocket(const char *server_hostname, struct sockaddr_in *server_address) {
    struct hostent *server;
    int client_socket;


    /* 2. ziskani adresy serveru pomoci DNS */
    if ((server = gethostbyname(server_hostname)) == nullptr) {
        fprintf(stderr, "ERROR: no such host as %s\n", server_hostname);
        exit(EXIT_FAILURE);
    }

/* 3. nalezeni IP adresy serveru a inicializace struktury server_address */
    bzero((char *) server_address, sizeof(*server_address));
    server_address->sin_family = AF_INET;
    bcopy((char *) server->h_addr, (char *) &(server_address->sin_addr.s_addr), server->h_length);
    server_address->sin_port = htons(514);

/* tiskne informace o vzdalenem soketu */
    printf("INFO: Server socket: %s : %d \n", inet_ntoa(server_address->sin_addr), ntohs(server_address->sin_port));

/* Vytvoreni soketu */
    if ((client_socket = socket(AF_INET, SOCK_DGRAM, 0)) <= 0) {
        perror("ERROR: socket");
        exit(EXIT_FAILURE);
    }

    return client_socket;
}


//isabot HOST[:PORT] CHANNELS [-s SYSLOG_SERVER] [-l HIGHLIGHT] [-h|--help]
bool parse_parameters(int argc, char *argv[], string *ircHost, int *ircPort, std::vector<std::string> channels,
                     string *syslogServer,
                     vector<string> keywords) {

    *ircPort = -1;
    *syslogServer = NULL;

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
            channels = explode_string(string(","), current);

            for (string str : channels) {
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

    if (*syslogServer == NULL) {
        *syslogServer = "127.0.0.1";
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
            end_position = source.find(',', start_position + 1);

            start_position++;
        } while (end_position != string::npos);

        output.emplace_back(source.substr(start_position));
    }


    return output;
}


//todo: copied, not tested, needs changes!!!
//char* get_machine_ip_address() {
//    struct ifaddrs *ifaddr, *ifa;
//    int family, s, n;
//    char host[NI_MAXHOST];
//
//    if (getifaddrs(&ifaddr) == -1) {
//        perror("getifaddrs");
//        exit(EXIT_FAILURE);
//    }
//
//    /* Walk through linked list, maintaining head pointer so we
//       can free list later */
//
//    for (ifa = ifaddr, n = 0; ifa != NULL; ifa = ifa->ifa_next, n++) {
//        if (ifa->ifa_addr == NULL)
//            continue;
//
//        family = ifa->ifa_addr->sa_family;
//
//        /* Display interface name and family (including symbolic
//           form of the latter for the common families) */
//
//
//        printf("%-8s %s (%d)\n",
//               ifa->ifa_name,
//               (family == AF_PACKET) ? "AF_PACKET" :
//               (family == AF_INET) ? "AF_INET" :
//               (family == AF_INET6) ? "AF_INET6" : "???",
//               family);
//
//        /* For an AF_INET* interface address, display the address */
//
//        if (family == AF_INET || family == AF_INET6) {
//            s = getnameinfo(ifa->ifa_addr,
//                            (family == AF_INET) ? sizeof(struct sockaddr_in) :
//                            sizeof(struct sockaddr_in6),
//                            host, NI_MAXHOST,
//                            NULL, 0, NI_NUMERICHOST);
//            if (s != 0) {
//                printf("getnameinfo() failed: %s\n", gai_strerror(s));
//                exit(EXIT_FAILURE);
//            }
//
//            printf("\t\taddress: <%s>\n", host);
//
//
//        }
//    }
//}

