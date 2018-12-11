//
// Created by root on 23.11.18.
//

#ifndef MSBDIREKTKOMM_SERVER_HPP
#define MSBDIREKTKOMM_SERVER_HPP

#include <string>
#include <map>
#include <pthread.h>

#include "TCPSSLServer.h"

#include "../allg/standardlogg.hpp"
#include "../allg/ThreadFIFO.hpp"

#define NACHR_S 300

namespace verteiler {

	class lieferant {
	public:
		std::string CertFile;
		std::string KeyFile;
		std::string Port;
		ASecureSocket::LogFnCallback Logg = standardLogg;
		bool logg_aktiv = true;

		pthread_t thread;
		bool aktiv;

		std::mutex mut;

		std::map<std::string, std::vector<int>> themaKunden;
		std::map<std::string, std::unique_ptr<ThreadFIFO<std::string>>> themaNachrichten;
		bool zusenden;

		lieferant(std::string CertFile_, std::string KeyFile_, std::string Port_ = "8080");

		~lieferant();

		static void Start(lieferant* s);

		static void Beenden(lieferant* s);

		static void* Listen(void* s);

		static bool Senden(lieferant* s, std::string thema, std::string nachricht);

		static void ThemaAnlegen(verteiler::lieferant* s, std::string thema);

		static bool KundenFuersThema(verteiler::lieferant* s, std::string thema);
	};

}
#endif //MSBDIREKTKOMM_SERVER_HPP