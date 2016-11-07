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
		const std::string & password,
		const std::string & output,
		bool verbose
	) :
		m_endpoint(endpoint),
		m_username(username),
		m_password(password),
		m_outputFile(output, std::ofstream::out | std::ofstream::binary | std::ofstream::app),
		m_verbose(verbose)
	{
		LOG("HTTP_Client initializing... endpoint URL: %s, output: %s\n", UA_DateTime_now(), m_endpoint.c_str(), output.c_str());
	}

	HTTP_Client::~HTTP_Client()
	{
		m_outputFile.close();
	}

	json HTTP_Client::getJSON(const std::string & path)
	{
		// Store request variables
		std::string url_str(m_endpoint + path);
		json result = "{}"_json;

		// Input stream for request output
		std::ostringstream sstream;
		curl_ios<std::ostringstream> cwriter(sstream);

		// Create request header
		curl_header cheader;

		// Add request headers
		cheader.add("Accept: application/json");

		// Use curl_easy request interface
		curl_easy ceasy(cwriter);

		// Add request payload
		ceasy.add<CURLOPT_HTTPHEADER>(cheader.get());
		ceasy.add<CURLOPT_URL>(url_str.c_str());
		ceasy.add<CURLOPT_FOLLOWLOCATION>(1L);
		ceasy.add<CURLOPT_VERBOSE>((m_verbose == true) ? 1L : 0L);

		try
		{
			// Excecute the request
			ceasy.perform();

			if (sstream.str().size() > 2)
			{
				// Get the result JSON
				result = json::parse(sstream.str());
			}
		}
		catch (const curl_easy_exception & e)
		{
			ERR("libcurl Excpetion: %s\n", UA_DateTime_now(), e.what());
		}
		catch (const std::exception & e)
		{
			ERR("normal Excpetion: %s\n", UA_DateTime_now(), e.what());
		}

		return result;
	}

	void HTTP_Client::sendJSON(const std::string & path, HTTP_Request_t request, json & data)
	{
		// Store request variables
		std::string url_str(m_endpoint + path);
		std::string data_str = data.dump();

		// Input stream for request output
		curl_ios<std::ostream> cwriter(m_outputFile);

		// Create request header
		curl_header cheader;

		// Add request headers
		cheader.add("Content-Type: application/json");

		// Use curl_easy request interface
		curl_easy ceasy(cwriter);

		// Add request payload
		ceasy.add<CURLOPT_HTTPHEADER>(cheader.get());
		ceasy.add<CURLOPT_URL>(url_str.c_str());
		ceasy.add<CURLOPT_CUSTOMREQUEST>((request == HTTP_POST) ? "POST" : "PUT");
		ceasy.add<CURLOPT_POSTFIELDS>(data_str.c_str());
		ceasy.add<CURLOPT_POSTFIELDSIZE>(-1L);
		ceasy.add<CURLOPT_VERBOSE>((m_verbose == true) ? 1L : 0L);

		try
		{
			// Excecute the request
			ceasy.perform();
		}
		catch (const curl_easy_exception & e)
		{
			ERR("libcurl Excpetion: %s\n", UA_DateTime_now(), e.what());
		}
		catch (const std::exception & e)
		{
			ERR("normal Excpetion: %s\n", UA_DateTime_now(), e.what());
		}
	}

	void HTTP_Client::sendREQ(const std::string & path, HTTP_Request_t request)
	{
		// Store request variables
		std::string url_str(m_endpoint + path);

		// Input stream for request output
		curl_ios<std::ostream> cwriter(m_outputFile);

		// Create request header
		curl_header cheader;

		// Add request headers
		cheader.add("Accept: application/json");

		// Use curl_easy request interface
		curl_easy ceasy(cwriter);

		// Add request payload
		ceasy.add<CURLOPT_HTTPHEADER>(cheader.get());
		ceasy.add<CURLOPT_URL>(url_str.c_str());
		ceasy.add<CURLOPT_CUSTOMREQUEST>((request == HTTP_DELETE) ? "DELETE" : "GET");
		ceasy.add<CURLOPT_FOLLOWLOCATION>(1L);
		ceasy.add<CURLOPT_VERBOSE>((m_verbose == true) ? 1L : 0L);

		try
		{
			// Excecute the request
			ceasy.perform();
		}
		catch (const curl_easy_exception & e)
		{
			ERR("libcurl Excpetion: %s\n", UA_DateTime_now(), e.what());
		}
		catch (const std::exception & e)
		{
			ERR("normal Excpetion: %s\n", UA_DateTime_now(), e.what());
		}
	}

	bool HTTP_Client::isVerbose() const
	{
		return m_verbose;
	}

}