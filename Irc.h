//
// Author: Jakub Fajkus
// Project: ISA IRC bot
// Last revision: 17.11.2017
//

#ifndef ISA_IRC_H
#define ISA_IRC_H

#include "IrcServer.h"
#include "SyslogServer.h"
#include "utils.h"
#include "Channel.h"

class Irc {
private:
    IrcServer *irc_server;
    SyslogServer *syslog_server;
    vector<string> keywords;
    vector<Channel> channels;
    string channels_string;

    /**
     * Proccesses the PRIVMSG or NOTICE message
     *
     * @param response Response string
     * @param tokens Response string split by space
     */
    void handle_privmsg(string response, vector<string> tokens);

    /**
     * Check if the given tokens contain the given keywords
     *
     * @param tokens
     * @param keywords
     *
     * @return True, if at least one keyword is present in the tokens, false otherwise
     */
    bool contains_keywords(vector<string> tokens, vector<string> keywords);

    /**
     * Handles the error messages
     *
     * @param tokens Tokens of the message
     */
    void handle_error_messages(vector<string> tokens);

    /**
     * Get the channel object.
     *
     * If the object is not found, it is created.
     *
     * @param name Name of the channel
     * @return Channel
     */
    Channel *get_channel(string &name);

    /**
     * Adds the user to the channel
     *
     * @param channel Channel name
     * @param nickname User nickname
     */
    void add_user_to_channel(string channel, string nickname);

    /**
     * Removes the user from the channel
     *
     * @param channel Channel name
     * @param nickname User nickname
     */
    void remove_user_from_channel(string channel, string nickname);

    /**
     * Get the user's nickname from the message
     *
     * @param message The message string
     *
     * @return User's nickname. If the nickname could not be found, empty string is returned
     */
    string get_nickname_from_message(string message);

    /**
     * Get vector of strings of all queued messages for the user
     *
     * @param nickname User's nickname
     * @param channel Channel name
     * @return All user's messages for the chanel
     */
    vector<string> get_messages_for_user(string nickname, string channel);

    /**
     * Send all queued messages to the user
     *
     * @param nickname User's nickname
     * @param channel Channel name
     */
    void send_messages_to_user(string nickname, string channel);

    /**
     * Queue a new message for the user to given channel
     *
     * @param nickname User's nickname
     * @param channel Channel name
     * @param message Mesage content
     */
    void add_message_for_user(string nickname, string channel, string message);

    /**
     * Handles the name reply from the server.
     *
     * The server sends a list of nicknames connected to a channel.
     * The method adds all nicknames to the channel
     *
     * @param tokens Message tokens
     */
    void handle_name_reply(vector<string> tokens);

    /**
     * Handles the JOIN message.
     *
     * Add the user to a channel
     *
     * @param response Message string
     * @param tokens Message tokens
     */
    void handle_join(string response, vector<string> tokens);

    /**
     * Handles the PART message
     *
     * Remove the user from the specified channels
     *
     * @param response Message string
     * @param tokens Message tokens
     */
    void handle_part(string response, vector<string> tokens);

    /**
     * Handles the QUIT message
     *
     * Remove the user from all channels
     *
     * @param response Message string
     */
    void handle_quit(string response);

    /**
     * Handles the KICK message of an user.
     *
     * Remove user from the channel
     *
     * This does not handle the KICK of the bot itself.
     *
     * @param response Message string
     */
    void handle_kick(vector<string> response);

    /**
     * Handles the NICK message
     *
     * Removes the user's old nickname from all channels.
     * Add the user's new nickname to all channels
     *
     * @param response Message string
     * @param tokens Message tokens
     */
    void handle_nick(string response, vector<string> tokens);

public:
    /**
     * Creates an object and creates channels based on the channels string given
     *
     * @param irc_server IRC server connection
     * @param syslog_server Syslog server connection
     * @param keywords Keywords which will be logged to the syslog server if present in a user's message
     * @param channels Channels to connect to. It's coma separated list of channel names(insluding the # and & characters)
     */
    Irc(IrcServer *irc_server, SyslogServer *syslog_server, vector<string> keywords, string channels);


    /**
     * Initialize the connection with the IRC server
     *
     * Connects to all channels given
     */
    void init_connection();

    /**
     * Start the main program loop, in which messages from the IRC server are received and processed.
     */
    void listen();

    /**
     * Closs the conneciton with the IRC and syslog servers.
     */
    void quit();
};


#endif //ISA_IRC_H
