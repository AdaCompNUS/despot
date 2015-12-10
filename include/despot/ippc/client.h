#ifndef CLIENT_H_
#define CLIENT_H_

#include <cstring>      // Needed for memset
#include <unistd.h>
#include <sys/socket.h> // Needed for the socket functions
#include <sys/types.h>
#include <netdb.h>      // Needed for the socket functions
#include <vector>
#include <string>
#include <map>

namespace despot {

class Client {
public:
	Client(void);
	~Client(void);

private:
	int socketfd;
  std::string HOSTNAME;
	std::string PORT;
	struct addrinfo host_info; // The struct that getaddrinfo() fills up with data.^M
	struct addrinfo *host_info_list;

public:
	void setHostName(std::string hostname);
	void setPort(std::string port);

	void initializeSocket();

	void connectToServer();

	void closeConnection();

	void sendMessage(std::string sendbuf);

  std::string recvMessage();

  std::string recvMessageTwice();

  std::string createSessionRequestMes(std::string problemName);

	int processSessionInitMes(std::string mes);

  std::string createRoundRequestMes();

	void processRoundInitMes(std::string mes);

  std::map<std::string, std::string> processTurnMes(std::string mes);

	//get step reward from turn message: added by wkg
	double getStepReward(std::string mes);

  std::string createActionMes(const std::string actionName, std::string actionValue);

	double processRoundEndMes(std::string mes);

	double processSessionEndMes(std::string mes);
};

} // namespace despot


#endif
