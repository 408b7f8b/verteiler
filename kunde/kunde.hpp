//
// Created by root on 23.11.18.
//

#ifndef MSBDIREKTKOMM_CLIENT_HPP
#define MSBDIREKTKOMM_CLIENT_HPP

#include <string>
#include <ctime>
#include <pthread.h>

#include "TCPSSLClient.h"
#include "../allg/standardlogg.hpp"
#include "../allg/ThreadFIFO.hpp"
#include "../allg/string_add.hpp"

#define NACHR_S 300
#define PP_INTERVALL 10.0

namespace verteiler {

	class kunde {
	public:
		enum Zustand {
			NICHTVERBUNDEN, VERBINDE, VERBUNDEN
		} aktZustand;

		std::string adr, port;
		bool logg_aktiv = true;

		pthread_t thread;
		bool aktiv;

		ThreadFIFO<std::string> nachrA;

		std::function<void(kunde*, std::string, std::string)> cbEingehend;

		kunde(std::string adr_, std::string port_, std::function<void(kunde*, std::string, std::string)> cbEingehend_);

		static void Start(kunde* c);

		static void* Betrieb(void* c);

		static void Beenden(kunde* cl);

		static void Anmelden(kunde* c, std::string thema);

		static void Anmelden(kunde* c, std::vector<std::string> themen);

	};
}


#endif //MSBDIREKTKOMM_CLIENT_HPP
