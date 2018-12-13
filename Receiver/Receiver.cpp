#include "Receiver.hpp"

#include <ctime>
#include <csignal>

#include "TCPSSLClient.h"

using namespace Verteiler;

Receiver::Receiver(std::string address_, std::string port_,
				   std::function<void(std::string, std::string, std::string)> callback_incoming_msg_,
				   std::string identifier_) {
	address = address_;
	port = port_;
	callback_incoming_msg = callback_incoming_msg_;
	currState = DISCONNECTED;
	identifier = identifier_;
}

void Receiver::Run() {
	pthread_create(&(this->thread), NULL, Receiver::thread_main, this);
}

void Receiver::RegisterToTopic(std::string topic) {
	if(topic.empty())
		return;

	if(currState == CONNECTED){
		this->msg_outgoing.put("REG:" + topic);
	}

	if(!vector_has<std::string>(topic, this->topics)){
		this->topics.push_back(topic);
	}
}

void Receiver::RegisterToTopic(std::vector<std::string> topics_) {
	if(topics_.empty())
		return;

	std::string agr;

	for (auto& s : topics_) {
		if(!vector_has<std::string>(s, this->topics)){
			this->topics.push_back(s);
			agr += s + ";";
		}
	}

	if(currState == CONNECTED){
		this->msg_outgoing.put("REG:" + agr);
	}
}

void* Receiver::thread_main(void* c) {
	sigset_t blockedSignal;
	sigemptyset(&blockedSignal);
	sigaddset(&blockedSignal, SIGPIPE);
	pthread_sigmask(SIG_BLOCK, &blockedSignal, NULL);

	Receiver* cl = (Receiver*) c;

	CTCPSSLClient* SecureTcpClient;

	cl->active = true;

	std::clock_t start, last_contact;

	while (cl->active) {
		switch (cl->currState) {
			case DISCONNECTED: {
				if (cl->logging_active) {
					SecureTcpClient = new CTCPSSLClient(cl->logging_callback);
				} else {
					SecureTcpClient = new CTCPSSLClient(cl->logging_callback, ASecureSocket::OpenSSLProtocol::TLS,
														ASocket::SettingsFlag::NO_FLAGS);
				}

				cl->currState = CONNECTING;

				break;
			}
			case CONNECTING: {
				if (SecureTcpClient->Connect(cl->address, cl->port)) {
					cl->currState = CONNECTED;

					SecureTcpClient->SetRcvTimeout(cl->rcv_snd_timeout_msec);
					SecureTcpClient->SetSndTimeout(cl->rcv_snd_timeout_msec);

					//cl->RegisterToTopic(cl->topics);

					start = std::clock();
					last_contact = start;
				} else {
					usleep(cl->conn_try_interval_usec);
				}

				break;
			}
			case CONNECTED: {
				std::clock_t now = std::clock();

				char Buffer[MAX_NUMBER_CHAR_RCV] = {0};

				int l = SecureTcpClient->Receive(Buffer, MAX_NUMBER_CHAR_RCV);

				if (l > 0) {
					std::vector<std::string> split = string_split(std::string(Buffer).substr(0, l), {':'});

					if (split.size() > 1) {
						std::string topic = split[0];
						std::string content = split[1];

						cl->callback_incoming_msg(cl->identifier, topic, content);
					} else if (split.size() == 1) {
						cl->callback_incoming_msg(cl->identifier, "", split[0]);

						if (split[0] == "PONG") {

						}
					}

					last_contact = now;
				}

				if ((now - last_contact) / (double) CLOCKS_PER_SEC > cl->conn_timeout_sec) {
					cl->currState = CONNECTING;
				}

				std::shared_ptr<std::string> msg;
				if (cl->msg_outgoing.get(&msg)) {
					std::string tmp = *msg;
					if (!SecureTcpClient->Send(tmp)) {
						cl->currState = CONNECTING;
					}
					start = now;
				} else if ((now - start) / (double) CLOCKS_PER_SEC > cl->pingpong_interval_sec) {
					std::string tmp = "PING";
					if (!SecureTcpClient->Send(tmp)) {
						cl->currState = CONNECTING;
					}
				}

				break;
			}
		}

		usleep(cl->automat_interval_usec);
	}

	SecureTcpClient->Disconnect();
	delete (SecureTcpClient);

	return NULL;
}

void Receiver::Halt() {
	this->active = false;
	pthread_join(this->thread, NULL);
}