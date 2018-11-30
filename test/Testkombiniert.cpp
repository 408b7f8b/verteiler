//
// Created by root on 24.11.18.
//

#include "../lieferant/lieferant.hpp"
#include "../kunde/kunde.hpp"


void funktion(verteiler::kunde* c, std::string thema, std::string inhalt){
	std::cout << "Thema/Topic: " << thema << std::endl << "Inhalt: " << inhalt << std::endl;
}

int main(int argc, char **argv) {

	verteiler::lieferant* s = new verteiler::lieferant("./cert.pem", "./key.pem", "8080");
	s->logg_aktiv = false;

	s->Start(s);

	sleep(1);

	verteiler::kunde* cl = new verteiler::kunde("127.0.0.1", "8080", funktion);
	cl->logg_aktiv = false;

	verteiler::kunde::Start(cl);

	sleep(5);

	for(int i = 0; i < 5; ++i){

		verteiler::lieferant::ThemaAnlegen(s, "thema" + std::to_string(i));

		sleep(1);

		verteiler::kunde::Anmelden(cl, "thema" + std::to_string(i));

		sleep(1);

		verteiler::lieferant::Senden(s, "thema" + std::to_string(i), "nachricht");

		sleep(1);
	}

	verteiler::kunde::Beenden(cl);

	delete(cl);

	verteiler::lieferant::Beenden(s);

	delete(s);

	return 0;
}