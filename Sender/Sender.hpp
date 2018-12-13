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

		Sender(std::string certFile_, std::string keyFile_, std::string port_ = "8080");

		~Sender();

		void Start();

		void Halt();

		void CreateTopic(std::string topic);

		bool HasTopicReceivers(std::string topic);

		bool Send(std::string topic, std::string message);

	};

}
#endif //VERTEILER_SENDER_HPP