#include <despot/ippc/client.h>
#include <despot/util/tinyxml/tinyxml.h>
#include <iostream>

using namespace std;
using namespace despot::util::tinyxml;

namespace despot {

Client::Client(void) {
}

Client::~Client(void) {
}

void Client::setHostName(string hostname) {
	HOSTNAME = hostname;
}

void Client::setPort(string port) {
	PORT = port;
}

void Client::initializeSocket() {
	int status;
	//	    struct addrinfo host_info;       // The struct that getaddrinfo() fills up with data.
	//	    struct addrinfo *host_info_list; // Pointer to the to the linked list of host_info's.

	// The MAN page of getaddrinfo() states "All  the other fields in the structure pointed
	// to by hints must contain either 0 or a null pointer, as appropriate." When a struct
	// is created in c++, it will be given a block of memory. This memory is not nessesary
	// empty. Therefor we use the memset function to make sure all fields are NULL.
	memset(&host_info, 0, sizeof host_info);

	std::cout << "Setting up the structs..." << std::endl;

	host_info.ai_family = AF_UNSPEC;   // IP version not specified. Can be both.
	host_info.ai_socktype = SOCK_STREAM; // Use SOCK_STREAM for TCP or SOCK_DGRAM for UDP.

	// Now fill up the linked list of host_info structs with google's address information.
	cout << "hostname: " << HOSTNAME << endl;
	cout << "port: " << PORT << endl;
	status = getaddrinfo(HOSTNAME.c_str(), PORT.c_str(), &host_info,
		&host_info_list);
	// getaddrinfo returns 0 on succes, or some other value when an error occured.
	// (translated into human readable text by the gai_gai_strerror function).
	if (status != 0)
		std::cout << "getaddrinfo error" << gai_strerror(status);

	std::cout << "Creating a socket..." << std::endl;
	//  int socketfd ; // The socket descripter
	socketfd = socket(host_info_list->ai_family, host_info_list->ai_socktype,
		host_info_list->ai_protocol);
	if (socketfd == -1)
		std::cout << "socket error ";

	std::cout << "Connect()ing..." << std::endl;
	status = connect(socketfd, host_info_list->ai_addr,
		host_info_list->ai_addrlen);
	if (status == -1)
		std::cout << "connect error";

}

void Client::connectToServer() {

}

void Client::closeConnection() {
	freeaddrinfo(host_info_list);
	close(socketfd);
}
void Client::sendMessage(string sendbuf) {
	cout << "message: " << sendbuf << endl;
	sendbuf.insert(sendbuf.end(), '\0');
	send(socketfd, sendbuf.c_str(), sendbuf.length(), 0);
}

string Client::recvMessage() {
	/*char recvbuf[1];
	 string s;
	 int n;
	 n = recv(ConnectSocket, recvbuf, 1, 0);
	 while(recvbuf[0] != '\0')
	 {
	 s.insert(s.end(), recvbuf[0]);
	 n = recv( ConnectSocket, recvbuf, 1, 0);
	 }
	 return s;*/
	//int n;
	string s;
	bool end = false;
	ssize_t bytes_received;
	//int k = 0;
	do {
		char recvbuf[1024];
		//cout<<"k:"<<k<<endl;
		bytes_received = recv(socketfd, recvbuf, 1024, 0);
		//cout<<"recv result: "<<n<<endl;
		if (bytes_received == 0) { // Exit if host shuts down
			std::cout << "host shut down." << std::endl;
			exit(1);
		}
		if (bytes_received == -1)
			std::cout << "recieve error!" << std::endl;

		for (int i = 0; i < bytes_received; i++) {
			if (recvbuf[i] == '\0') {
				end = true;
				s.append(recvbuf, i);
				break;
			}
		}
		if (!end) {
			s.append(recvbuf, bytes_received);
		}
		//k++;
	} while (!end);
	return s;
}

string Client::recvMessageTwice() {
	string s;
	int end = 0;
	do {
		char recvbuf[1024];
		int n = recv(socketfd, recvbuf, 1024, 0);
		for (int i = 0; i < n; i++) {
			//cout<<i<<" "<<recvbuf[i]<<endl;
			if (recvbuf[i] == '\0') {
				end++;
			}
		}
	} while (end < 2);
	return s;
}

string Client::createSessionRequestMes(string problemName) {
	TiXmlDocument doc;
	TiXmlElement request("session-request");
	TiXmlElement problem("problem-name");
	TiXmlElement client("client-name");
	TiXmlText pName(problemName.c_str());
	TiXmlText cName("NUS-POMDPGroup");
	problem.InsertEndChild(pName);
	client.InsertEndChild(cName);
	request.InsertEndChild(client);
	request.InsertEndChild(problem);
	doc.InsertEndChild(request);
	TiXmlPrinter printer;
	doc.Accept(&printer);
	string s(printer.CStr());
	return s;
}

int Client::processSessionInitMes(string mes) {
	TiXmlDocument doc;
	doc.Parse(mes.c_str());
	TiXmlHandle hDoc(&doc);
	TiXmlElement * root = hDoc.FirstChild("session-init").ToElement();
	TiXmlElement * pNextSibling = root->FirstChild("num-rounds")->ToElement();
	return atoi(pNextSibling->GetText());
}

string Client::createRoundRequestMes() {
	string s("<round-request/>");
	return s;
}

void Client::processRoundInitMes(string mes) {

}

map<string, string> Client::processTurnMes(string mes) {
	cout << "mes: " << mes << endl;
	map<string, string> result;
	TiXmlDocument doc;
	doc.Parse(mes.c_str());
	TiXmlHandle hDoc(&doc);
	TiXmlElement * root = hDoc.FirstChild("turn").ToElement();
	TiXmlElement * pNextSibling =
		root->FirstChild("observed-fluent")->ToElement();
	while (pNextSibling != NULL) {
		TiXmlElement * pName =
			pNextSibling->FirstChild("fluent-name")->ToElement();
		TiXmlElement * pValue =
			pNextSibling->FirstChild("fluent-value")->ToElement();
		string name(pName->GetText());
		TiXmlNode * pnArg = pNextSibling->FirstChild("fluent-arg");
		if (pnArg != NULL) {
			name.append("__");
			name.append(pnArg->ToElement()->GetText());
			while ((pnArg = pnArg->NextSibling()) != NULL) {
				string vArg(pnArg->Value());
				if (vArg.compare("fluent-arg") == 0) {
					name.append("_");
					name.append(pnArg->ToElement()->GetText());
				}
			}
		}
		string value(pValue->GetText());
		result[name] = value;
		pNextSibling = pNextSibling->NextSiblingElement();
	}
	return result;
}

//get set reward from turn message: added by wkg
double Client::getStepReward(string mes) {
	TiXmlDocument doc;
	doc.Parse(mes.c_str());
	TiXmlHandle hDoc(&doc);
	TiXmlElement * root = hDoc.FirstChild("turn").ToElement();
	TiXmlElement * pNextSibling =
		root->FirstChild("immediate-reward")->ToElement();
	return atof(pNextSibling->GetText());

}

string Client::createActionMes(const string actionName,
	const string actionValue1) {
	TiXmlDocument doc;
	TiXmlElement actions("actions");
	if (actionValue1.compare("noop") != 0) {
		int index3 = actionValue1.find("___");
		int last3 = 0;
		while (index3 != string::npos) {
			string actionValue = actionValue1.substr(last3, index3 - last3);
			TiXmlElement action("action");
			TiXmlElement name("action-name");
			int index = actionValue.find("__");
			if (index != string::npos) {
				TiXmlText vName(actionValue.substr(0, index).c_str());
				name.InsertEndChild(vName);
				action.InsertEndChild(name);

				int last = index + 2;
				index = actionValue.find_first_of('_', last);
				while (index != string::npos) {
					TiXmlElement arg("action-arg");
					TiXmlText vArg(
						actionValue.substr(last, index - last).c_str());
					arg.InsertEndChild(vArg);
					action.InsertEndChild(arg);
					last = index + 1;
					index = actionValue.find_first_of('_', last);
				}
				TiXmlElement arg("action-arg");
				TiXmlText vArg(actionValue.substr(last, index - last).c_str());
				arg.InsertEndChild(vArg);
				action.InsertEndChild(arg);
			} else {
				TiXmlText vName(actionValue.c_str());
				name.InsertEndChild(vName);
				action.InsertEndChild(name);
			}
			TiXmlElement value("action-value");
			TiXmlText vValue("true");
			value.InsertEndChild(vValue);
			action.InsertEndChild(value);
			actions.InsertEndChild(action);

			last3 = index3 + 3;
			index3 = actionValue1.find("___", last3);
		}
		string actionValue = actionValue1.substr(last3, index3 - last3);
		TiXmlElement action("action");
		TiXmlElement name("action-name");
		int index = actionValue.find("__");
		if (index != string::npos) {
			TiXmlText vName(actionValue.substr(0, index).c_str());
			name.InsertEndChild(vName);
			action.InsertEndChild(name);

			int last = index + 2;
			index = actionValue.find_first_of('_', last);
			while (index != string::npos) {
				TiXmlElement arg("action-arg");
				TiXmlText vArg(actionValue.substr(last, index - last).c_str());
				arg.InsertEndChild(vArg);
				action.InsertEndChild(arg);
				last = index + 1;
				index = actionValue.find_first_of('_', last);
			}
			TiXmlElement arg("action-arg");
			TiXmlText vArg(actionValue.substr(last, index - last).c_str());
			arg.InsertEndChild(vArg);
			action.InsertEndChild(arg);
		} else {
			TiXmlText vName(actionValue.c_str());
			name.InsertEndChild(vName);
			action.InsertEndChild(name);
		}
		TiXmlElement value("action-value");
		TiXmlText vValue("true");
		value.InsertEndChild(vValue);
		action.InsertEndChild(value);
		actions.InsertEndChild(action);

		last3 = index3 + 3;
		index3 = actionValue1.find("___", last3);
	}
	doc.InsertEndChild(actions);
	TiXmlPrinter printer;
	doc.Accept(&printer);
	string s(printer.CStr());
	return s;
}

double Client::processRoundEndMes(string mes) {
	TiXmlDocument doc;
	doc.Parse(mes.c_str());
	TiXmlHandle hDoc(&doc);
	TiXmlElement * root = hDoc.FirstChild("round-end").ToElement();
	TiXmlElement * pNextSibling = root->FirstChild("round-reward")->ToElement();
	return atof(pNextSibling->GetText());
}

double Client::processSessionEndMes(string mes) {
	TiXmlDocument doc;
	doc.Parse(mes.c_str());
	TiXmlHandle hDoc(&doc);
	TiXmlElement * root = hDoc.FirstChild("session-end").ToElement();
	TiXmlElement * pNextSibling = root->FirstChild("total-reward")->ToElement();
	return atof(pNextSibling->GetText());
}

} // namespace despot
