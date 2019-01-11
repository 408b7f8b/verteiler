#ifndef VERTEILER_RECEIVER_HPP
#define VERTEILER_RECEIVER_HPP

#include <string>
#include <functional>

#include <pthread.h>

#include "../Common/Common.hpp"

#define MAX_NUMBER_CHAR_RCV 1024

namespace Verteiler {

	class Receiver {

		std::string address;
		std::string port;

		std::function<void(const std::string&)> logging_callback = standard_logging;

		ThreadFIFO<std::string> msg_outgoing;
		std::function<void(const std::string&, const std::string&, const std::string&)> callback_incoming_msg;
		std::vector<std::string> topics;

		pthread_t thread;
		bool active;

		static void* thread_main(void* c);

	public:

		std::string identifier;

		std::uint8_t pingpong_interval_sec = 10;
		std::uint8_t conn_timeout_sec = 60;
		std::uint32_t conn_try_interval_usec = 1000000;
		std::uint16_t rcv_snd_timeout_msec = 200;
		std::uint32_t automat_interval_usec = 10000;

		bool logging_active = false;

		enum state {
			DISCONNECTED, CONNECTING, CONNECTED
		} currState;

		Receiver(const std::string& address_, const std::string& port_,
				 std::function<void(const std::string&, const std::string&, const std::string&)> callback_incoming_msg_,
				 const std::string& identifier_ = "myreceiver");

		void Run();

		void Halt();

		void RegisterToTopic(const std::string& topic);

		void RegisterToTopic(const std::vector<std::string>& topics_);

		void UnregisterFromTopic(const std::string& topic);

		void UnregisterFromTopic(const std::vector<std::string>& topics_);

	};
}


#endif //VERTEILER_RECEIVER_HPP
