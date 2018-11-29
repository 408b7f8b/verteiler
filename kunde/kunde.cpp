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

void kunde::Anmelden(kunde* c, std::string thema) {
	c->nachrA.setze("REG:" + thema);
}

void kunde::Anmelden(kunde* c, std::vector<std::string> themen) {
	std::string agr;

	for(auto& s : themen){
		agr += s + ";";
	}

	kunde::Anmelden(c, agr);
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

	SecureTCPKunde->Connect(cl->adr, cl->port);
	SecureTCPKunde->SetRcvTimeout(200);
	SecureTCPKunde->SetSndTimeout(200);

	while (cl->aktiv) {


			versuche = 0;
			while (cl->aktiv) {
				char Buffer[nachr_s] = {0};
				int l = SecureTCPKunde->Receive(Buffer, nachr_s);

				if (l > 0) {
					cl->cbEingehend(cl, std::string(Buffer).substr(0, l));
				}

				std::shared_ptr<std::string> nachr;
				if(cl->nachrA.hole(&nachr)){
					std::string tmp = *nachr;
					SecureTCPKunde->Send(tmp);
				}
			}
			usleep(10000);

	}

	SecureTCPKunde->Disconnect();
	delete (SecureTCPKunde);

	return NULL;
}

void kunde::Beenden(kunde* cl) {
	cl->aktiv = false;
	pthread_join(cl->thread, NULL);
}