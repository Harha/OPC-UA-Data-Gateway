#include "http_client.h"
#include <iostream>
#include <fstream>
#include <open62541.h>
#include <curl_easy.h>
#include <curl_ios.h>
#include <curl_exception.h>
#include <curl_header.h>
#include "../macros.h"

// For convenience
using json = nlohmann::json;
using curl::curl_easy;
using curl::curl_ios;
using curl::curl_easy_exception;
using curl::curlcpp_traceback;
using curl::curl_header;

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

	json HTTP_Client::getJSON(const std::string & path)
	{
		// Input stream for request output
		/*std::ostringstream sstream;
		curl_ios<std::ostringstream> ios(sstream);


		// Create request header
		curl_header cheader;

		// Add request headers
		cheader.add("Content-Type: application/json");

		// Use curl_easy request interface
		curl_easy ceasy;

		// Add request payload
		ceasy.add<CURLOPT_HTTPHEADER>(cheader.get());
		ceasy.add<CURLOPT_URL>((m_endpoint + path).c_str());
		ceasy.add<CURLOPT_FOLLOWLOCATION>(1L);

		// Excecute the request
		try
		{
			ceasy.perform();
		}
		catch (const curl_easy_exception & e)
		{
			ERR("Excpetion: %s\n", UA_DateTime_now(), e.what());
		}

		// Return the result
		json result;
		sstream << result;
		return result;*/
		return "{}"_json;
	}

	void HTTP_Client::postJSON(const std::string & path, json data)
	{

	}

}