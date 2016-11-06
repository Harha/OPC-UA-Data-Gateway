#ifndef CLIENT_HTTP_H
#define CLIENT_HTTP_H

#include <string>

namespace gateway
{

	class HTTP_Client
	{
	public:
		HTTP_Client(
			const std::string & endpoint = "http://localhost:9090",
			const std::string & username = "",
			const std::string & password = ""
		);
		~HTTP_Client();
		void update();
	private:
		std::string m_endpoint;
		std::string m_username;
		std::string m_password;
	};

}

#endif // CLIENT_HTTP_H