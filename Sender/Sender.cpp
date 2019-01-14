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

#include "Sender.hpp"

#include <csignal>

#include "TCPSSLServer.h"

using namespace Verteiler;

Sender::Sender(const std::string& certFile_, const std::string& keyFile_, const std::string& port_) {
	certFile = certFile_;
	keyFile = keyFile_;
	port = port_;
}

Sender::~Sender() {
	if (active) {
		Sender::Halt();
	}
}

void Sender::Start() {
	this->active = true;
	pthread_create(&(this->thread), nullptr, Sender::thread_main, this);
}

void Sender::Halt() {
	this->active = false;
	pthread_join(this->thread, nullptr);
}

bool Sender::Send(const std::string& topic, const std::string& message) {
	if (this->topics_andtheirreceivers.count(topic) && this->topics_andtheirmessages.count(topic)) {
		this->topics_andtheirmessages.at(topic)->put(message);
		return true;
	}

	return false;
}

void Sender::CreateTopic(const std::string& topic) {
	topics_andtheirreceivers.insert({topic, {}});
	topics_andtheirmessages.insert({topic, std::unique_ptr<ThreadFIFO<std::string>>(new ThreadFIFO<std::string>)});
}

void* Sender::thread_main(void* s) {
	sigset_t blockedSignal;
	sigemptyset(&blockedSignal);
	sigaddset(&blockedSignal, SIGPIPE);
	pthread_sigmask(SIG_BLOCK, &blockedSignal, nullptr);

	auto sender = (Verteiler::Sender*) s;

	CTCPSSLServer* SecureTcpServer;

	if (sender->logging_active) {
		SecureTcpServer = new CTCPSSLServer(sender->logging_callback, sender->port);
	} else {
		SecureTcpServer = new CTCPSSLServer(sender->logging_callback, sender->port, ASecureSocket::OpenSSLProtocol::TLS,
											ASocket::SettingsFlag::NO_FLAGS);
	}

	SecureTcpServer->SetSSLCertFile(sender->certFile);
	SecureTcpServer->SetSSLKeyFile(sender->keyFile);

	std::map<int, ASecureSocket::SSLSocket*> receivers;

	int i = 0;

	auto last_receiver = new ASecureSocket::SSLSocket();

	std::vector<int> obsolete;

	while (sender->active) {
		if (SecureTcpServer->Listen(*last_receiver, sender->listen_timeout_msec)) {
			receivers.insert({i++, last_receiver});
			SecureTcpServer->SetRcvTimeout(*last_receiver, sender->rcv_snd_timeout_msec);
			SecureTcpServer->SetSndTimeout(*last_receiver, sender->rcv_snd_timeout_msec);
			last_receiver = new ASecureSocket::SSLSocket();
		}

		for (auto& c : receivers) {
			char rcv_buffer[MAX_NUMBER_CHAR_SND] = {0};
			auto received_bytes = (unsigned long)SecureTcpServer->Receive(*c.second, &(rcv_buffer[0]), MAX_NUMBER_CHAR_SND);
			if (received_bytes > 0 && strlen(rcv_buffer) > 4) {
				std::string msg = std::string(rcv_buffer).substr(0, received_bytes);
				if (!msg.compare(0, 4, "REG:")) {
					std::string shortened = msg.substr(4);
					std::vector<std::string> split = string_split(shortened, {0x7E});

					for (auto& sp : split) {
						if (sender->topics_andtheirreceivers.count(sp)) {
							sender->topics_andtheirreceivers.at(sp).push_back(c.first);
						}
					}
				} else if (msg.compare(0, 4, "PING")) {
					if (!SecureTcpServer->Send(*c.second, "PONG")) {
						obsolete.push_back(c.first);
					}
				}
			}
		}

		for (auto& t : sender->topics_andtheirmessages) {
			std::shared_ptr<std::string> msg;
			if (t.second->get(&msg) && sender->topics_andtheirreceivers.count(t.first)) {
				for (auto& c : sender->topics_andtheirreceivers.at(t.first)) {
					if (receivers.count(c)) {
						if (!SecureTcpServer->Send(*receivers.at(c), t.first + ":" + *msg)) {
							obsolete.push_back(c);
						}
					}
				}
			}
		}

		if (!obsolete.empty()) {
			for (auto in = obsolete.end() - 1; in >= obsolete.begin(); --in) {
				receivers.erase(*in);
				for (auto& t : sender->topics_andtheirreceivers) {
					for (auto it = t.second.end() - 1; it >= t.second.begin(); --it) {
						if (*it == *in) {
							t.second.erase(it);
						}
					}
				}
				obsolete.erase(in);
			}
		}
	}

	delete (last_receiver);

	for (auto& r : receivers) {
		SecureTcpServer->Disconnect(*r.second);
		delete (r.second);
	}

	delete (SecureTcpServer);

	return nullptr;
}

bool Sender::HasTopicReceivers(const std::string& topic) {
	if (topics_andtheirreceivers.count(topic)) {
		if (!topics_andtheirreceivers[topic].empty()) {
			return true;
		}
	}

	return false;
}