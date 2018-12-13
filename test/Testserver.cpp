#include "../Sender/Sender.hpp"
#include <unistd.h>

int main(int argc, char** argv) {

	std::unique_ptr<Verteiler::Sender> s(new Verteiler::Sender("./cert.pem", "./key.pem", "8080"));
	s->Start();

	s->CreateTopic("thema");

	for (int i = 0; i < 100; ++i) {
		if (s->HasTopicReceivers("thema")) {
			s->Send("thema", "nachricht");
		}

		sleep(1);
	}

	s->Halt();

	delete (s.get());

	return 0;

}