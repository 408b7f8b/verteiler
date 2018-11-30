//
// Created by root on 23.11.18.
//

#include "kunde.hpp"

using namespace verteiler;

kunde::kunde(std::string adr_, std::string port_, std::function<void(kunde*, std::string, std::string)> cbEingehend_) {
	adr = adr_;
	port = port_;
	cbEingehend = cbEingehend_;
	aktZustand = NICHTVERBUNDEN;
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

	for (auto& s : themen) {
		agr += s + ";";
	}

	kunde::Anmelden(c, agr);
}

void* kunde::Betrieb(void* c) {
	kunde* cl = (kunde*) c;

	CTCPSSLClient* SecureTCPKunde;

	cl->aktiv = true;

	std::clock_t start;
	double duration;

	while (cl->aktiv) {
		switch(cl->aktZustand){
			case NICHTVERBUNDEN: {
				if (cl->logg_aktiv) {
					SecureTCPKunde = new CTCPSSLClient(standardLogg);
				} else {
					SecureTCPKunde = new CTCPSSLClient(standardLogg, ASecureSocket::OpenSSLProtocol::TLS,
													   ASocket::SettingsFlag::NO_FLAGS);
				}

				cl->aktZustand = VERBINDE;

				break;
			}
			case VERBINDE:{
				SecureTCPKunde->Connect(cl->adr, cl->port); //timeout?

				cl->aktZustand = VERBUNDEN;

				SecureTCPKunde->SetRcvTimeout(200);
				SecureTCPKunde->SetSndTimeout(200);

				start = std::clock();

				break;
			}
			case VERBUNDEN: {

				char Buffer[NACHR_S] = {0};
				int l = SecureTCPKunde->Receive(Buffer, NACHR_S);

				std::string thema = string_split(std::string(Buffer).substr(0, l), {':'})[0];
				std::string inhalt = std::string(Buffer).substr(thema.size() + 1);

				if (l > 0) {
					cl->cbEingehend(cl, thema, inhalt);
				}

				std::shared_ptr<std::string> nachr;
				if (cl->nachrA.hole(&nachr)) {
					std::string tmp = *nachr;
					if(!SecureTCPKunde->Send(tmp)){
						cl->aktZustand = VERBINDE;
					}
					start = std::clock();
				} else if ((std::clock() - start) / (double) CLOCKS_PER_SEC > PP_INTERVALL) {
					std::string tmp = "PING";
					if(!SecureTCPKunde->Send(tmp)){
						cl->aktZustand = VERBINDE;
					}
				}

				break;
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