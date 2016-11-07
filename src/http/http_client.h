#ifndef CLIENT_HTTP_H
#define CLIENT_HTTP_H

#include <string>
#include <fstream>
#include "../3rdparty/json.hpp"

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
			const std::string & endpoint = "http://localhost:9090",
			const std::string & username = "",
			const std::string & password = "",
			const std::string & output = "./res/libcurl_log.log",
			bool verbose = true
		);
		~HTTP_Client();
		nlohmann::json getJSON(const std::string & path);
		void sendJSON(const std::string & path, HTTP_Request_t request, nlohmann::json & data);
		void sendREQ(const std::string & path, HTTP_Request_t request);
		bool isVerbose() const;
	private:
		std::string m_endpoint;
		std::string m_username;
		std::string m_password;
		std::ofstream m_outputFile;
		bool m_verbose;
	};

}

#endif // CLIENT_HTTP_H