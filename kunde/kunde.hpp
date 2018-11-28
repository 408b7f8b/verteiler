//
// Created by root on 23.11.18.
//

#ifndef MSBDIREKTKOMM_CLIENT_HPP
#define MSBDIREKTKOMM_CLIENT_HPP

#include <string>
#include <pthread.h>

#include "TCPSSLClient.h"
#include "../allg/standardlogg.hpp"

namespace verteiler {

	class kunde {
	public:
		std::string adr, port;
		bool logg_aktiv = true;

		pthread_t thread;
		bool aktiv;

		std::function<void(kunde*, std::string)> cbEingehend;

		kunde(std::string adr_, std::string port_, std::function<void(kunde*, std::string)> cbEingehend_);

		static void Start(kunde* c);

		static void* Betrieb(void* c);

		static void Beenden(kunde* cl);
	};

}


#endif //MSBDIREKTKOMM_CLIENT_HPP
