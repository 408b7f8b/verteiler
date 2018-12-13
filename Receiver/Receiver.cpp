//
// Created by root on 23.11.18.
//

#include "Receiver.hpp"
#include <csignal>

using namespace Verteiler;

Receiver::Receiver(std::string address_, std::string port_, std::function<void(Receiver*, std::string, std::string)> callback_incoming_msg_) {
	address = address_;
	port = port_;
	callback_incoming_msg = callback_incoming_msg_;
	currState = DISCONNECTED;
}

void Receiver::Run() {
	pthread_create(&(this->thread), NULL, Receiver::thread_main, this);
}

void Receiver::RegisterToTopic(std::string topic) {
	this->msg_outgoing.put("REG:" + topic);
}

void Receiver::RegisterToTopic(std::vector<std::string> topics) {
	std::string agr;

	for (auto& s : topics) {
		agr += s + ";";
	}

	this->RegisterToTopic(agr);
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
		switch(cl->currState){
			case DISCONNECTED: {
				if (cl->logging_active) {
					SecureTcpClient = new CTCPSSLClient(standard_logging);
				} else {
					SecureTcpClient = new CTCPSSLClient(standard_logging, ASecureSocket::OpenSSLProtocol::TLS,
													   ASocket::SettingsFlag::NO_FLAGS);
				}

				cl->conn_try_number = 255;
				cl->currState = CONNECTING;

				break;
			}
			case CONNECTING:{
				if(SecureTcpClient->Connect(cl->address, cl->port)){
					cl->currState = CONNECTED;

					SecureTcpClient->SetRcvTimeout(cl->rcv_snd_timeout_msec);
					SecureTcpClient->SetSndTimeout(cl->rcv_snd_timeout_msec);

					start = std::clock();
					last_contact = start;
				}else{
					usleep(cl->conn_try_interval_usec);
					--(cl->conn_try_number);
				}

				break;
			}
			case CONNECTED: {
				std::clock_t now = std::clock();

				char Buffer[MAX_NUMBER_CHAR] = {0};

				int l = SecureTcpClient->Receive(Buffer, MAX_NUMBER_CHAR);

				if (l > 0) {
					std::vector<std::string> split = string_split(std::string(Buffer).substr(0, l), {':'});

					if(split.size() > 1){
						std::string topic = split[0];
						std::string content = split[1];

						cl->callback_incoming_msg(cl, topic, content);
					}else if(split.size() == 1){
						cl->callback_incoming_msg(cl, "", split[0]);

						if(split[0] == "PONG"){

						}
					}

					last_contact = now;
				}

				if((now - last_contact) / (double) CLOCKS_PER_SEC > cl->timeout_sec ) {
					cl->conn_try_number = 255;
					cl->currState = CONNECTING;
				}

				std::shared_ptr<std::string> msg;
				if (cl->msg_outgoing.get(&msg)) {
					std::string tmp = *msg;
					if(!SecureTcpClient->Send(tmp)){
						cl->conn_try_number = 255;
						cl->currState = CONNECTING;
					}
					start = now;
				} else if ((now - start) / (double) CLOCKS_PER_SEC > cl->pingpong_interval_sec) {
					std::string tmp = "PING";
					if(!SecureTcpClient->Send(tmp)){
						cl->conn_try_number = 255;
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