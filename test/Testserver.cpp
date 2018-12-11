//
// Created by root on 23.11.18.
//

#include <signal.h>
#include "../lieferant/lieferant.hpp"


int main(int argc, char **argv) {

	std::unique_ptr<verteiler::lieferant> s(new verteiler::lieferant("./cert.pem", "./key.pem", "8080"));
	s->Start(s.get());

	verteiler::lieferant::ThemaAnlegen(s.get(), "thema");

	for(int i = 0; i < 100; ++i){
		if(verteiler::lieferant::KundenFuersThema(s.get(), "thema")){
			verteiler::lieferant::Senden(s.get(), "thema", "nachricht");
		}

		sleep(1);
	}

	s->Beenden(s.get());

	delete(s.get());

	return 0;
}