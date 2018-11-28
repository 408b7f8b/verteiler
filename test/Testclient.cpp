//
// Created by root on 23.11.18.
//

#include "../kunde/kunde.hpp"
#include "../allg/string_add.hpp"

void funktion(verteiler::kunde* c, std::string kette){
	std::cout << kette << std::endl;
}

int main(int argc, char **argv) {

	verteiler::kunde* cl = new verteiler::kunde(std::string("127.0.0.1"), std::string("8080"), funktion);

	verteiler::kunde::Start(cl);

	sleep(100);

	verteiler::kunde::Beenden(cl);

	return 0;
}