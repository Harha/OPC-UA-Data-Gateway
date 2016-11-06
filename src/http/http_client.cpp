#include "http_client.h"
#include <iostream>
#include <fstream>
#include <open62541.h>
#include <curl_easy.h>
#include <curl_exception.h>
#include "../macros.h"

namespace gateway
{

	HTTP_Client::HTTP_Client(
		const std::string & endpoint,
		const std::string & username,
		const std::string & password
	) :
		m_endpoint(endpoint),
		m_username(username),
		m_password(password)
	{
		LOG("HTTP_Client initializing... endpoint URL: %s\n", UA_DateTime_now(), m_endpoint.c_str());
	}

	HTTP_Client::~HTTP_Client()
	{

	}

	void HTTP_Client::update()
	{

	}

}