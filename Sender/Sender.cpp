//
// Created by root on 23.11.18.
//

#include <csignal>
#include "Sender.hpp"
#include "../allg/string_add.hpp"

using namespace Verteiler;

Sender::Sender(std::string certFile_, std::string keyFile_, std::string port_) {
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
	pthread_create(&(this->thread), NULL, Sender::thread_main, this);
}

void Sender::Halt() {
	this->active = false;
	pthread_join(this->thread, NULL);
}

bool Sender::Send(std::string topic, std::string message) {
	if (this->topics_andtheirreceivers.count(topic) && this->topics_andtheirmessages.count(topic)) {
		this->topics_andtheirmessages.at(topic)->put(message);
		return true;
	}

	return false;
}

void Sender::CreateTopic(std::string topic){
	topics_andtheirreceivers.insert({topic, {}});
	topics_andtheirmessages.insert( { topic, std::unique_ptr<ThreadFIFO<std::string>>(new ThreadFIFO<std::string>) } );
}

void* Sender::thread_main(void* s) {
	sigset_t blockedSignal;
	sigemptyset(&blockedSignal);
	sigaddset(&blockedSignal, SIGPIPE);
	pthread_sigmask(SIG_BLOCK, &blockedSignal, NULL);

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

		for(auto& c : receivers){
			char rcv_buffer[MAX_NUMBER_CHAR] = {0};
			int received_bytes = SecureTcpServer->Receive(*c.second, &(rcv_buffer[0]), MAX_NUMBER_CHAR);
			if(received_bytes > 0 && strlen(rcv_buffer) > 4){
				std::string msg = std::string(rcv_buffer).substr(0, received_bytes);
				if(!msg.compare(0, 4, "REG:")){
					std::string shortened = msg.substr(4);
					std::vector<std::string> split = string_split(shortened, {0x7E});

					for(auto& s : split){
						if(sender->topics_andtheirreceivers.count(s)){
							sender->topics_andtheirreceivers.at(s).push_back(c.first);
						}
					}
				}else if(msg.compare(0, 4, "PING")){
					if(!SecureTcpServer->Send(*c.second, "PONG")){
						obsolete.push_back(c.first);
					}
				}
			}
		}

		for(auto& t : sender->topics_andtheirmessages){
			std::shared_ptr<std::string> msg;
			if(t.second->get(&msg) && sender->topics_andtheirreceivers.count(t.first)){
				for(auto& c : sender->topics_andtheirreceivers.at(t.first)) {
					if(receivers.count(c)){
						if(!SecureTcpServer->Send(*receivers.at(c), t.first + ":" + *msg)){
							obsolete.push_back(c);
						}
					}
				}
			}
		}

		if(obsolete.size() > 0){
			for(std::vector<int>::iterator in = obsolete.end()-1; in >= obsolete.begin(); --in){
				receivers.erase(*in);
				for(auto& t : sender->topics_andtheirreceivers){
					for(std::vector<int>::iterator it = t.second.end()-1; it >= t.second.begin(); --it){
						if(*it == *in){
							t.second.erase(it);
						}
					}
				}
				obsolete.erase(in);
			}
		}

	}

	delete (last_receiver);

	for (auto& i : receivers) {
		SecureTcpServer->Disconnect(*i.second);
		delete (i.second);
	}

	delete (SecureTcpServer);

	return NULL;
}

bool Sender::HasTopicReceivers(std::string topic){
	if (topics_andtheirreceivers.count(topic)) {
		if(!topics_andtheirreceivers[topic].empty()){
			return true;
		}
	}

	return false;
}