#include "../Receiver/Receiver.hpp"

#include <iostream>
#include <unistd.h>

void funktion(const std::string& receiver, const std::string& thema, const std::string& inhalt) {
	std::cout << "Receiver " << receiver  << ", Thema/Topic: " << thema << std::endl << "Inhalt: " << inhalt << std::endl;
}

int main(int argc, char** argv) {

	Verteiler::Receiver* cl = new Verteiler::Receiver(std::string("127.0.0.1"), std::string("8080"), funktion);

	cl->Run();

	for (int i = 0; i < 100; ++i) {
		if (cl->currState == Verteiler::Receiver::CONNECTED) {
			cl->RegisterToTopic("thema");
		}

		sleep(1);
	}

	sleep(100);

	cl->Halt();

	return 0;
}