/*Copyright 2019, 2019 David A. Breunig

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

		http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#include "../Sender/Sender.hpp"
#include <unistd.h>

//Build certificates with openssl req -newkey rsa:2048 -new -nodes -x509 -days 3650 -keyout key.pem -out cert.pem

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