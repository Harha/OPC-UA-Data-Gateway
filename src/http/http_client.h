#ifndef CLIENT_HTTP_H
#define CLIENT_HTTP_H

#include <iostream>
#include <fstream>
#include <string>
#include <curl_easy.h>
#include <curl_ios.h>
#include <curl_exception.h>
#include <curl_header.h>
#include "../3rdparty/json.hpp"

// For convenience
using json = nlohmann::json;
using curl::curl_easy;
using curl::curl_ios;
using curl::curl_easy_exception;
using curl::curlcpp_traceback;
using curl::curl_header;

namespace gateway
{

	enum HTTP_Request_t
	{
		HTTP_GET,
		HTTP_POST,
		HTTP_PUT,
		HTTP_DELETE
	};

	class HTTP_Client
	{
	public:
		HTTP_Client(
			const std::string & jsonConfig
		);
		~HTTP_Client();
		nlohmann::json getJSON(const std::string & path);
		void sendJSON(const std::string & path, HTTP_Request_t request, nlohmann::json & data);
		void sendREQ(const std::string & path, HTTP_Request_t request);
		bool isVerbose() const;
	private:
		std::string m_jsonConfig;
		std::string m_endpoint;
		std::string m_username;
		std::string m_password;
		std::ofstream m_outputFile;
		bool m_verbose;
	};

}

#endif // CLIENT_HTTP_H