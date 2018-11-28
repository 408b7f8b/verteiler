//
// Created by root on 23.11.18.
//

#include "lieferant.hpp"

using namespace verteiler;

lieferant::lieferant(std::string CertFile_, std::string KeyFile_, std::string Port_) {
	CertFile = CertFile_;
	KeyFile = KeyFile_;
	Port = Port_;
}

lieferant::~lieferant() {
	if(aktiv){
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

bool lieferant::Senden(lieferant* s, std::string& str, unsigned int versuche, unsigned int pause) {
	bool ret = false;

	s->mut.lock();
	for (int i = 0; i < versuche; ++i) {
		if (s->ausgehend.empty()) {
			s->ausgehend = str;
			ret = true;
			break;
		}
		usleep(pause);
	}
	s->mut.unlock();

	return ret;
}

void* lieferant::Listen(void* s) {
	lieferant* lieferant = (verteiler::lieferant*) s;

	CTCPSSLServer* SecureTCPSSLServer;

	if(lieferant->logg_aktiv){
		SecureTCPSSLServer = new CTCPSSLServer(lieferant->Logg, lieferant->Port);
	}else{
		SecureTCPSSLServer = new CTCPSSLServer(lieferant->Logg, lieferant->Port, ASecureSocket::OpenSSLProtocol::TLS, ASocket::SettingsFlag::NO_FLAGS);
	}

	SecureTCPSSLServer->SetSSLCertFile(lieferant->CertFile);
	SecureTCPSSLServer->SetSSLKeyFile(lieferant->KeyFile);

	std::map<int, ASecureSocket::SSLSocket*> clients;

	int i = 0;

	ASecureSocket::SSLSocket* letzterClient = new ASecureSocket::SSLSocket();

	while (lieferant->aktiv) {
		if (SecureTCPSSLServer->Listen(*letzterClient, 1000)) {
			clients.insert({i++, letzterClient});
			struct timeval t;

			t.tv_sec = 1;
			t.tv_usec = 0;

			SecureTCPSSLServer->SetRcvTimeout(*letzterClient, t);
			letzterClient = new ASecureSocket::SSLSocket();
		}

		lieferant->mut.lock();
		if (!lieferant->ausgehend.empty()) {
			for (auto& c : clients) {
				SecureTCPSSLServer->Send(*c.second, lieferant->ausgehend);
			}
			lieferant->ausgehend.clear();
		}
		lieferant->mut.unlock();
	}

	delete (letzterClient);

	for (auto& i : clients) {
		SecureTCPSSLServer->Disconnect(*i.second);
		delete (i.second);
	}

	delete (SecureTCPSSLServer);

	return NULL;
}