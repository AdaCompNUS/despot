#pragma once

#include <cstring>      // Needed for memset
#include <unistd.h>
#include <sys/socket.h> // Needed for the socket functions
#include <sys/types.h>
#include <netdb.h>      // Needed for the socket functions
#include <vector>
#include <string>
#include <map>

#include "util/tinyxml/tinyxml.h"

using namespace std;

class Client {
public:
	Client(void);
	~Client(void);

private:
	int socketfd;
	string HOSTNAME;
	string PORT;
	struct addrinfo host_info; // The struct that getaddrinfo() fills up with data.^M
	struct addrinfo *host_info_list;

public:
	void setHostName(string hostname);
	void setPort(string port);

	void initializeSocket();

	void connectToServer();

	void closeConnection();

	void sendMessage(string sendbuf);

	string recvMessage();

	string recvMessageTwice();

	string createSessionRequestMes(string problemName);

	int processSessionInitMes(string mes);

	string createRoundRequestMes();

	void processRoundInitMes(string mes);

	map<string, string> processTurnMes(string mes);

	//get step reward from turn message: added by wkg
	double getStepReward(string mes);

	string createActionMes(const string actionName, string actionValue);

	double processRoundEndMes(string mes);

	double processSessionEndMes(string mes);
};

