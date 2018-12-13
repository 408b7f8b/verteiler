#ifndef VERTEILER_SENDER_HPP
#define VERTEILER_SENDER_HPP

#include <string>
#include <map>
#include <pthread.h>

#include "TCPSSLServer.h"

#include "../Common/standardlogg.hpp"
#include "../Common/ThreadFIFO.hpp"

#define MAX_NUMBER_CHAR 1024

namespace Verteiler {

	class Sender {

		std::string certFile;
		std::string keyFile;
		std::string port;

		ASecureSocket::LogFnCallback logging_callback = standard_logging;
		bool logging_active = false;

		std::map<std::string, std::vector<int>> topics_andtheirreceivers;
		std::map<std::string, std::unique_ptr<ThreadFIFO<std::string>>> topics_andtheirmessages;

		pthread_t thread;
		bool active;

		static void* thread_main(void* s);

	public:

		std::uint16_t listen_timeout_msec = 100;
		std::uint16_t rcv_snd_timeout_msec = 10;

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