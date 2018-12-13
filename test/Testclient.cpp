#include "../Receiver/Receiver.hpp"

void funktion(Verteiler::Receiver* c, std::string thema, std::string inhalt){
	std::cout << "Thema/Topic: " << thema << std::endl << "Inhalt: " << inhalt << std::endl;
}

int main(int argc, char **argv) {

	Verteiler::Receiver* cl = new Verteiler::Receiver(std::string("127.0.0.1"), std::string("8080"), funktion);

	cl->Run();

	for(int i = 0; i < 100; ++i){
		if(cl->currState == Verteiler::Receiver::CONNECTED){
			cl->RegisterToTopic("thema");
		}

		sleep(1);
	}

	sleep(100);

	cl->Halt();

	return 0;
}