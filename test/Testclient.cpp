//
// Created by root on 23.11.18.
//

#include "../kunde/kunde.hpp"
#include "../allg/string_add.hpp"
#include "../allg/ThreadFIFO.hpp"

void funktion(verteiler::kunde* c, std::string thema, std::string inhalt){
	std::cout << "Thema/Topic: " << thema << std::endl << "Inhalt: " << inhalt << std::endl;
}

int main(int argc, char **argv) {

	verteiler::kunde* cl = new verteiler::kunde(std::string("127.0.0.1"), std::string("8080"), funktion);

	verteiler::kunde::Start(cl);

	for(int i = 0; i < 100; ++i){
		if(cl->aktZustand == verteiler::kunde::VERBUNDEN){
			verteiler::kunde::Anmelden(cl, "thema");
			i == 9000;
		}

		sleep(1);
	}

	sleep(100);

	verteiler::kunde::Beenden(cl);

	return 0;
}