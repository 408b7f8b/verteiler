//
// Created by root on 23.11.18.
//

#include <csignal>
#include "lieferant.hpp"
#include "../allg/string_add.hpp"

using namespace verteiler;

lieferant::lieferant(std::string CertFile_, std::string KeyFile_, std::string Port_) {
	CertFile = CertFile_;
	KeyFile = KeyFile_;
	Port = Port_;
}

lieferant::~lieferant() {
	if (aktiv) {
		lieferant::Beenden(this);
	}
}

void lieferant::Start(lieferant* s) {
	if (s == NULL)
		return;

	s->aktiv = true;

	pthread_create(&(s->thread), NULL, lieferant::Listen, s);
}

void lieferant::Beenden(lieferant* s) {
	if (s == NULL)
		return;

	s->aktiv = false;

	pthread_join(s->thread, NULL);
}

bool lieferant::Senden(verteiler::lieferant* s, std::string thema, std::string nachricht) {
	if (s->themaKunden.count(thema) && s->themaNachrichten.count(thema)) {
		s->themaNachrichten.at(thema)->setze(nachricht);
		return true;
	}

	return false;
}

void lieferant::ThemaAnlegen(verteiler::lieferant* s, std::string thema){
	s->themaKunden.insert({thema, {}});
	s->themaNachrichten.insert( { thema, std::unique_ptr<ThreadFIFO<std::string>>(new ThreadFIFO<std::string>) } );
}

void* lieferant::Listen(void* s) {
	//signal(SIGPIPE,SIG_IGN);

	sigset_t blockedSignal;
	sigemptyset(&blockedSignal);
	sigaddset(&blockedSignal, SIGPIPE);
	pthread_sigmask(SIG_BLOCK, &blockedSignal, NULL);

	lieferant* lieferant = (verteiler::lieferant*) s;

	CTCPSSLServer* SecureTCPSSLServer;

	if (lieferant->logg_aktiv) {
		SecureTCPSSLServer = new CTCPSSLServer(lieferant->Logg, lieferant->Port);
	} else {
		SecureTCPSSLServer = new CTCPSSLServer(lieferant->Logg, lieferant->Port, ASecureSocket::OpenSSLProtocol::TLS,
											   ASocket::SettingsFlag::NO_FLAGS);
	}

	SecureTCPSSLServer->SetSSLCertFile(lieferant->CertFile);
	SecureTCPSSLServer->SetSSLKeyFile(lieferant->KeyFile);

	std::map<int, ASecureSocket::SSLSocket*> clients;

	int i = 0;

	ASecureSocket::SSLSocket* letzterClient = new ASecureSocket::SSLSocket();

	std::vector<int> wegdamit;

	while (lieferant->aktiv) {
		if (SecureTCPSSLServer->Listen(*letzterClient, 1000)) {
			clients.insert({i++, letzterClient});
			SecureTCPSSLServer->SetRcvTimeout(*letzterClient, 10);
			SecureTCPSSLServer->SetSndTimeout(*letzterClient, 10);
			letzterClient = new ASecureSocket::SSLSocket();
		}

		for(auto& c : clients){
			char r[NACHR_S] = {0};
			int res = SecureTCPSSLServer->Receive(*c.second, &(r[0]), NACHR_S);
			if(res > 0 && strlen(r) > 4){
				std::string na = std::string(r).substr(0, res);
				if(!na.compare(0, 4, "REG:")){
					std::string gek = na.substr(4);
					std::vector<std::string> zerl = string_split(gek, {0x7E});

					for(auto& s : zerl){
						if(lieferant->themaKunden.count(s)){
							lieferant->themaKunden.at(s).push_back(c.first);
						}
					}
				}else if(na.compare(0, 4, "PING")){
					if(!SecureTCPSSLServer->Send(*c.second, "PONG")){
						wegdamit.push_back(c.first);
					}
				}
			}
		}

		for(auto& t : lieferant->themaNachrichten){
			std::shared_ptr<std::string> nachr;
			if(t.second->hole(&nachr)){
				for(auto& c : lieferant->themaKunden.at(t.first)) {
					if(!SecureTCPSSLServer->Send(*clients.at(c), t.first + ":" + *nachr)){
						wegdamit.push_back(c);
					}
				}
			}
		}

		for(std::vector<int>::iterator in = wegdamit.begin(); in != wegdamit.end() && wegdamit.size(); ++in){
			clients.erase(*in);
			for(auto& t : lieferant->themaKunden){
				for(std::vector<int>::iterator it = t.second.begin(); it != t.second.end() && t.second.size(); ++it){
					if(*it == *in){
						t.second.erase(it);
					}
				}
			}
			wegdamit.erase(in);
		}
	}

	delete (letzterClient);

	for (auto& i : clients) {
		SecureTCPSSLServer->Disconnect(*i.second);
		delete (i.second);
	}

	delete (SecureTCPSSLServer);

	return NULL;
}

bool lieferant::KundenFuersThema(verteiler::lieferant* s, std::string thema){
	if (s->themaKunden.count(thema)) {
		return true;
	}

	return false;
}