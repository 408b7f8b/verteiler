/*Copyright 2019, 2019 David A. Breunig

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

		http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#include "Receiver.hpp"

#include <ctime>
#include <csignal>

#include "TCPSSLClient.h"

using namespace Verteiler;

Receiver::Receiver(const std::string& address_, const std::string& port_,
				   std::function<void(const std::string&, const std::string&, const std::string&)> callback_incoming_msg_,
				   const std::string& identifier_,
				   std::function<void(const std::string&, const std::string&)> callback_notification_) {
	address = address_;
	port = port_;
	callback_incoming_msg = callback_incoming_msg_;
	callback_notification = callback_notification_;
	currState = DISCONNECTED;
	identifier = identifier_;
}

void Receiver::Run() {
	pthread_create(&(this->thread), nullptr, Receiver::thread_main, this);
}

void Receiver::RegisterToTopic(const std::string& topic) {
	if(topic.empty())
		return;

	if(!vector_has<std::string>(topic, this->topics)){
		this->topics.push_back(topic);
	}

	if(currState == CONNECTED){
		this->msg_outgoing.put("REG:" + topic);
	}
}

void Receiver::RegisterToTopic(const std::vector<std::string>& topics_) {
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

void Receiver::UnregisterFromTopic(const std::string& topic) {
	if(topic.empty())
		return;

	for (auto it = this->topics.end() - 1; it >= this->topics.begin(); --it) {
		if (*it == topic) {
			this->topics.erase(it);
		}
	}

	if(currState == CONNECTED){
		this->msg_outgoing.put("UNREG:" + topic);
	}
}

void Receiver::UnregisterFromTopic(const std::vector<std::string>& topics_) {
	if(topics_.empty())
		return;

	std::string agr;

	for (auto& s : topics_) {
		for (auto it = this->topics.end() - 1; it >= this->topics.begin(); --it) {
			if (*it == s) {
				this->topics.erase(it);
				agr += s + ";";
			}
		}
	}

	if(currState == CONNECTED){
		this->msg_outgoing.put("UNREG:" + agr);
	}
}

bool Receiver::IsConnectionOK(){
	return currState == CONNECTED;
}

void* Receiver::thread_main(void* c) {
	sigset_t blockedSignal;
	sigemptyset(&blockedSignal);
	sigaddset(&blockedSignal, SIGPIPE);
	pthread_sigmask(SIG_BLOCK, &blockedSignal, nullptr);

	auto cl = (Receiver*) c;

	CTCPSSLClient* SecureTcpClient = nullptr;

	cl->active = true;

	std::clock_t start(0), last_contact(0);

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

					cl->RegisterToTopic(cl->topics);

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
					if(!cl->callback_notification){
						cl->callback_notification(cl->identifier, "TIMEOUT");
					}
				}

				std::shared_ptr<std::string> msg;
				if (cl->msg_outgoing.get(&msg)) {
					std::string tmp = *msg;
					if (!SecureTcpClient->Send(tmp)) {
						cl->currState = CONNECTING;
						cl->callback_notification(cl->identifier, "SEND FAIL");
					}
					start = now;
				} else if ((now - start) / (double) CLOCKS_PER_SEC > cl->pingpong_interval_sec) {
					std::string tmp = "PING";
					if (!SecureTcpClient->Send(tmp)) {
						cl->currState = CONNECTING;
						cl->callback_notification(cl->identifier, "SEND FAIL");
					}
				}

				break;
			}
		}

		usleep(cl->automat_interval_usec);
	}

	if(SecureTcpClient != nullptr) {
		SecureTcpClient->Disconnect();
		delete (SecureTcpClient);
	}

	return nullptr;
}

void Receiver::Halt() {
	this->active = false;
	pthread_join(this->thread, nullptr);
}