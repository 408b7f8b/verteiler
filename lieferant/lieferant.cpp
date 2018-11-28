//
// Created by root on 23.11.18.
//

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
	bool ret = false;

	s->mut.lock();
	while (s->zusenden = true) {}
	if (s->themaKunden.count(thema)) {
		s->zusenden_thema = thema;
		s->zusenden_nachricht = nachricht;
		s->zusenden = true;
		ret = true;
	}
	s->mut.unlock();

	return ret;
}

void* lieferant::Listen(void* s) {
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

	SecureTCPSSLServer->SetRcvTimeout(*letzterClient, 10);
	SecureTCPSSLServer->SetSndTimeout(*letzterClient, 10);

	while (lieferant->aktiv) {
		if (SecureTCPSSLServer->Listen(*letzterClient, 1000)) {
			clients.insert({i++, letzterClient});
			SecureTCPSSLServer->SetRcvTimeout(*letzterClient, 10);
			SecureTCPSSLServer->SetSndTimeout(*letzterClient, 10);
			letzterClient = new ASecureSocket::SSLSocket();
		}

		lieferant->mut.lock();
		if (lieferant->zusenden) {
			for(auto& k : lieferant->themaKunden.at(lieferant->zusenden_thema)){
				SecureTCPSSLServer->Send(*k, lieferant->zusenden_nachricht);
			}
			lieferant->zusenden = false;
		}
		lieferant->mut.unlock();

		for(auto& c : clients){
			char r[nachr_s] = {0};
			int res = SecureTCPSSLServer->Receive(*c.second, &(r[0]), nachr_s);
			if(res > 0){
				std::string na = std::string(r);
				if(na.substr(0, 3) == "ANM"){
					std::string gek = na.substr(3);
					std::vector<std::string> zerl = string_split(na, {'a', 'b'});
				}
			}
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