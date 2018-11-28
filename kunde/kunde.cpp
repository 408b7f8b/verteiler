//
// Created by root on 23.11.18.
//

#include "kunde.hpp"

using namespace verteiler;

kunde::kunde(std::string adr_, std::string port_, std::function<void(kunde*, std::string)> cbEingehend_) {
	adr = adr_;
	port = port_;
	cbEingehend = cbEingehend_;
}

void kunde::Start(kunde* c) {
	if (c == NULL)
		return;

	pthread_create(&(c->thread), NULL, kunde::Betrieb, c);
}

void* kunde::Betrieb(void* c) {
	kunde* cl = (kunde*) c;

	CTCPSSLClient* SecureTCPKunde;

	if(cl->logg_aktiv){
		SecureTCPKunde = new CTCPSSLClient(standardLogg);
	}else{
		SecureTCPKunde = new CTCPSSLClient(standardLogg, ASecureSocket::OpenSSLProtocol::TLS, ASocket::SettingsFlag::NO_FLAGS);
	}

	cl->aktiv = true;

	unsigned int versuche = 0;

	while (cl->aktiv) {
		if (SecureTCPKunde->Connect(cl->adr, cl->port)) {

			SecureTCPKunde->SetRcvTimeout(200);
			SecureTCPKunde->SetSndTimeout(200);
			versuche = 0;
			while (cl->aktiv) {
				char Buffer[1024] = {0};
				int l = SecureTCPKunde->Receive(Buffer, 1023);
				std::string s = std::string(Buffer).substr(0, l);
				if (l > 0) {
					cl->cbEingehend(cl, s);
				}

			}
			usleep(10000);
		} else {
			++versuche;
			sleep(5);

			if (versuche == 100)
				cl->aktiv = false;
		}
	}

	SecureTCPKunde->Disconnect();
	delete (SecureTCPKunde);

	return NULL;
}

void kunde::Beenden(kunde* cl) {
	cl->aktiv = false;
	pthread_join(cl->thread, NULL);
}