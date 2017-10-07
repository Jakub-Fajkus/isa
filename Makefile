SRC=Channel.cpp Irc.cpp IrcServer.cpp LoggedUser.cpp main.cpp SyslogServer.cpp UserMessage.cpp utils.cpp
FLAGS=-std=c++14 -fno-access-control  -fcheck-new -Wpedantic

all: $(SRC)
	g++ -o isabot $^ $(FLAGS)
