#ifndef CLIENT_HTTP_H
#define CLIENT_HTTP_H

#include <string>
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
			const std::string & password = ""
		);
		~HTTP_Client();
		nlohmann::json getJSON(const std::string & path);
		void sendJSON(const std::string & path, HTTP_Request_t request, nlohmann::json & data);
	private:
		std::string m_endpoint;
		std::string m_username;
		std::string m_password;
	};

}

#endif // CLIENT_HTTP_H