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

#ifndef VERTEILER_SENDER_HPP
#define VERTEILER_SENDER_HPP

#include <functional>
#include <string>
#include <map>

#include <pthread.h>

#include "../Common/Common.hpp"

#define MAX_NUMBER_CHAR_SND 1024

namespace Verteiler {

	class Sender {

		std::string certFile;
		std::string keyFile;
		std::string port;

		std::function<void(const std::string&)> logging_callback = standard_logging;

		std::map<std::string, std::vector<int>> topics_andtheirreceivers;
		std::map<std::string, std::unique_ptr<ThreadFIFO<std::string>>> topics_andtheirmessages;

		pthread_t thread;
		bool active;

		static void* thread_main(void* s);

	public:

		std::uint16_t listen_timeout_msec = 100;
		std::uint16_t rcv_snd_timeout_msec = 10;

		bool logging_active = false;

		Sender(const std::string& certFile_, const std::string& keyFile_, const std::string& port_ = "8080");

		~Sender();

		void Start();

		void Halt();

		void CreateTopic(const std::string& topic);

		bool HasTopicReceivers(const std::string& topic);

		bool Send(const std::string& topic, const std::string& message);

	};

}
#endif //VERTEILER_SENDER_HPP