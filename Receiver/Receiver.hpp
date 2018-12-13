#ifndef VERTEILER_RECEIVER_HPP
#define VERTEILER_RECEIVER_HPP

#include <string>
#include <ctime>
#include <pthread.h>

#include "TCPSSLClient.h"
#include "../Common/standardlogg.hpp"
#include "../Common/ThreadFIFO.hpp"
#include "../Common/string_add.hpp"

#define MAX_NUMBER_CHAR 1024

namespace Verteiler {

	class Receiver {

		std::string address, port;

		bool logging_active = false;

		ThreadFIFO<std::string> msg_outgoing;
		std::function<void(Receiver*, std::string, std::string)> callback_incoming_msg;

		pthread_t thread;
		bool active;

		static void* thread_main(void* c);

	public:
		std::uint8_t pingpong_interval_sec = 10;
		std::uint8_t timeout_sec = 60;
		std::uint8_t conn_try_number = 100;
		std::uint32_t conn_try_interval_usec = 1000000;
		std::uint16_t rcv_snd_timeout_msec = 200;
		std::uint32_t automat_interval_usec = 10000;

		enum state {
			DISCONNECTED, CONNECTING, CONNECTED
		} currState;

		Receiver(std::string address_, std::string port_, std::function<void(Receiver*, std::string, std::string)> callback_incoming_msg_);

		void Run();
		void Halt();

		void RegisterToTopic(std::string topic);
		void RegisterToTopic(std::vector<std::string> topics);

	};
}


#endif //VERTEILER_RECEIVER_HPP
