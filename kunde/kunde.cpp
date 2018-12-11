//
// Created by root on 23.11.18.
//

#include "kunde.hpp"
#include <csignal>

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
	sigset_t blockedSignal;
	sigemptyset(&blockedSignal);
	sigaddset(&blockedSignal, SIGPIPE);
	pthread_sigmask(SIG_BLOCK, &blockedSignal, NULL);

	kunde* cl = (kunde*) c;

	CTCPSSLClient* SecureTCPKunde;

	cl->aktiv = true;

	std::clock_t start, lKontakt;

	while (cl->aktiv) {
		switch(cl->aktZustand){
			case NICHTVERBUNDEN: {
				if (cl->logg_aktiv) {
					SecureTCPKunde = new CTCPSSLClient(standardLogg);
				} else {
					SecureTCPKunde = new CTCPSSLClient(standardLogg, ASecureSocket::OpenSSLProtocol::TLS,
													   ASocket::SettingsFlag::NO_FLAGS);
				}

				cl->versuche = 255;
				cl->aktZustand = VERBINDE;

				break;
			}
			case VERBINDE:{
				if(SecureTCPKunde->Connect(cl->adr, cl->port)){
					cl->aktZustand = VERBUNDEN;

					SecureTCPKunde->SetRcvTimeout(cl->rcvSndTimeout);
					SecureTCPKunde->SetSndTimeout(cl->rcvSndTimeout);

					start = std::clock();
					lKontakt = start;
				}else{
					usleep(cl->versuche_dauer_us);
					--(cl->versuche);
				}

				break;
			}
			case VERBUNDEN: {
				std::clock_t jetzt = std::clock();

				char Buffer[MAXZEICHEN] = {0};

				int l = SecureTCPKunde->Receive(Buffer, MAXZEICHEN);

				if (l > 0) {
					std::vector<std::string> zerlegt = string_split(std::string(Buffer).substr(0, l), {':'});

					if(zerlegt.size() > 1){
						std::string thema = zerlegt[0];
						std::string inhalt = zerlegt[1];

						cl->cbEingehend(cl, thema, inhalt);
					}else if(zerlegt.size() == 1){
						if(zerlegt[0] == "PONG"){

						}
					}

					lKontakt = jetzt;
				}

				if((jetzt - lKontakt) / (double) CLOCKS_PER_SEC > cl->TIMEOUT ) {
					cl->versuche = 255;
					cl->aktZustand = VERBINDE;
				}

				std::shared_ptr<std::string> nachr;
				if (cl->nachrA.hole(&nachr)) {
					std::string tmp = *nachr;
					if(!SecureTCPKunde->Send(tmp)){
						cl->versuche = 255;
						cl->aktZustand = VERBINDE;
					}
					start = jetzt;
				} else if ((jetzt - start) / (double) CLOCKS_PER_SEC > cl->PP_INTERVALL) {
					std::string tmp = "PING";
					if(!SecureTCPKunde->Send(tmp)){
						cl->versuche = 255;
						cl->aktZustand = VERBINDE;
					}
				}

				break;
			}
		}

		usleep(cl->intervall_automat);
	}

	SecureTCPKunde->Disconnect();
	delete (SecureTCPKunde);

	return NULL;
}

void kunde::Beenden(kunde* cl) {
	cl->aktiv = false;
	pthread_join(cl->thread, NULL);
}