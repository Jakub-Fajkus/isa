SRC=Channel.cpp Irc.cpp IrcServer.cpp LoggedUser.cpp main.cpp SyslogServer.cpp UserMessage.cpp utils.cpp
FLAGS=-std=c++14

all: $(SRC)
	g++ -o isabot $^ $(FLAGS)
