//
// Created by root on 23.11.18.
//

#include "../lieferant/lieferant.hpp"

int main(int argc, char **argv) {

	verteiler::lieferant* s = new verteiler::lieferant("./cert.pem", "./key.pem", "8080");
	s->Start(s);

	std::vector<char> k = {1, 2, 3, 0};

	for(int i = 0; i < 20; ++i){
		sleep(1);

		verteiler::lieferant::Senden(s, "thema", "nachricht");
	}

	s->Beenden(s);

	delete(s);

	return 0;
}