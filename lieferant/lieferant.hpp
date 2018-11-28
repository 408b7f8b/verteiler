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
		std::string ausgehend;

		lieferant(std::string CertFile_, std::string KeyFile_, std::string Port_ = "8080");

		~lieferant();

		static void Start(lieferant* s);

		static void Beenden(lieferant* s);

		static void* Listen(void* s);

		static bool Senden(lieferant* s, std::string& str, unsigned int versuche = 10, unsigned int pause = 1000);
	};

}
#endif //MSBDIREKTKOMM_SERVER_HPP