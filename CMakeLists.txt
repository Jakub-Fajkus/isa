cmake_minimum_required(VERSION 3.8)
project(isa)

set(CMAKE_CXX_STANDARD 17)

set(SOURCE_FILES main.cpp SyslogServer.cpp SyslogServer.h IrcServer.cpp IrcServer.h Irc.cpp Irc.h utils.cpp utils.h LoggedUser.cpp LoggedUser.h Channel.cpp Channel.h UserMessage.cpp UserMessage.h)
add_executable(isa ${SOURCE_FILES})