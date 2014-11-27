#include "Poco/Net/HTTPServer.h"
#include "Poco/Net/HTTPRequestHandler.h"
#include "Poco/Net/HTTPRequestHandlerFactory.h"
#include "Poco/Net/HTTPServerParams.h"
#include "Poco/Net/HTTPServerRequest.h"
#include "Poco/Net/HTTPServerResponse.h"
#include "Poco/Net/ServerSocket.h"
#include "Poco/Net/WebSocket.h"
#include "Poco/Net/NetException.h"
#include "Poco/Util/ServerApplication.h"
//#include "Poco/Util/Option.h"
//#include "Poco/Util/OptionSet.h"
//#include "Poco/Util/HelpFormatter.h"
#include "Poco/Format.h"
#include <iostream>

using Poco::Net::ServerSocket;
using Poco::Net::WebSocket;
using Poco::Net::WebSocketException;
using Poco::Net::HTTPRequestHandler;
using Poco::Net::HTTPRequestHandlerFactory;
using Poco::Net::HTTPServer;
using Poco::Net::HTTPServerRequest;
using Poco::Net::HTTPResponse;
using Poco::Net::HTTPServerResponse;
using Poco::Net::HTTPServerParams;
using Poco::Timestamp;
using Poco::ThreadPool;
using Poco::Util::ServerApplication;
using Poco::Util::Application;
//using Poco::Util::Option;
//using Poco::Util::OptionSet;
//using Poco::Util::HelpFormatter;

// Handle a WebSocket connection.
class WebSocketRequestHandler : public HTTPRequestHandler {
 public:
	void handleRequest(HTTPServerRequest& request, HTTPServerResponse& response) {
		//Application& app = Application::instance();
		try {
			WebSocket ws(request, response);
			//app.logger().information("WebSocket connection established.");
      std::cout << std::string("WebSocket connection established.") << std::endl;
			char buffer[1024];
			int flags;
			int n;
			do {
				n = ws.receiveFrame(buffer, sizeof(buffer), flags);
				//app.logger().information(Poco::format("Frame received (length=%d, flags=0x%x).", n, unsigned(flags)));
        std::cout << Poco::format("Frame received (length=%d, flags=0x%x).", n, unsigned(flags)) << std::endl; // use boost format?
				ws.sendFrame(buffer, n, flags);
			} while (n > 0 || (flags & WebSocket::FRAME_OP_BITMASK) != WebSocket::FRAME_OP_CLOSE);

			//app.logger().information("WebSocket connection closed.");
      std::cout << "WebSocket connection closed." << std::endl;
		}
		catch (WebSocketException& exc) {
			//app.logger().log(exc);
      std::cout << exc.what() << std::endl;
			switch (exc.code()) {
			 case WebSocket::WS_ERR_HANDSHAKE_UNSUPPORTED_VERSION:
				response.set("Sec-WebSocket-Version", WebSocket::WEBSOCKET_VERSION);
				// fallthrough
			 case WebSocket::WS_ERR_NO_HANDSHAKE:
			 case WebSocket::WS_ERR_HANDSHAKE_NO_VERSION:
			 case WebSocket::WS_ERR_HANDSHAKE_NO_KEY:
				response.setStatusAndReason(HTTPResponse::HTTP_BAD_REQUEST);
				response.setContentLength(0);
				response.send();
				break;
			}
		}
	}
};

class RequestHandlerFactory: public HTTPRequestHandlerFactory
{
public:
	HTTPRequestHandler* createRequestHandler(const HTTPServerRequest& request)
	{
		//Application& app = Application::instance();
		//app.logger().information("Request from " 
    std::string str = "Request from " 
			+ request.clientAddress().toString()
			+ ": "
			+ request.getMethod()
			+ " "
			+ request.getURI()
			+ " "
			+ request.getVersion();

    std::cout << str << std::endl;
			
		for (HTTPServerRequest::ConstIterator it = request.begin(); it != request.end(); ++it) {
			//app.logger().information(it->first + ": " + it->second);
      str = (it->first + ": " + it->second);
      std::cout << str << std::endl;
		}
		
		//if (request.getURI() == "/ws")
			return new WebSocketRequestHandler;
		//else
			//return new PageRequestHandler;
	}
};

class WebSocketServer /*: public Poco::Util::ServerApplication*/ {
public:
	WebSocketServer() /* : _helpRequested(false)*/ {
	}
	
	~WebSocketServer() {
	}

protected:
	/*void initialize(Application& self) {
		loadConfiguration(); // load default configuration files, if present
		ServerApplication::initialize(self);
	}
		
	void uninitialize() {
		ServerApplication::uninitialize();
	}*/

	/*void defineOptions(OptionSet& options) {
		ServerApplication::defineOptions(options);
		
		options.addOption(
			Option("help", "h", "display help information on command line arguments")
				.required(false)
				.repeatable(false));
	}

	void handleOption(const std::string& name, const std::string& value) {
		ServerApplication::handleOption(name, value);

		if (name == "help")
			_helpRequested = true;
	}

	void displayHelp() {
		HelpFormatter helpFormatter(options());
		helpFormatter.setCommand(commandName());
		helpFormatter.setUsage("OPTIONS");
		helpFormatter.setHeader("A sample HTTP server supporting the WebSocket protocol.");
		helpFormatter.format(std::cout);
	}*/

	
private:
	//bool _helpRequested;
};

#include <signal.h>
int main(int argc, char** argv) {
  //if (_helpRequested) {
  //	displayHelp();
  //}
  //else {
    // get parameters from configuration file
    unsigned short port = 9980;
    auto msg = "C++11 OK";
    std::cout << msg << std::endl;
    
    // set-up a server socket
    ServerSocket svs(port);
    // set-up a HTTPServer instance
    HTTPServer srv(new RequestHandlerFactory, svs, new HTTPServerParams);
    // start the HTTPServer
    srv.start();
    // wait for CTRL-C or kill
    //waitForTerminationRequest(); // req ServerApp (too much!)
    sigset_t sset; // use asio wait instead..? (portable).
    sigemptyset(&sset);
    if (!std::getenv("POCO_ENABLE_DEBUGGER"))
    {
      sigaddset(&sset, SIGINT);
    }
    sigaddset(&sset, SIGQUIT);
    sigaddset(&sset, SIGTERM);
    sigprocmask(SIG_BLOCK, &sset, NULL);
    int sig;
    sigwait(&sset, &sig);

    // Stop the HTTPServer
    srv.stop();
  //}
  return Application::EXIT_OK;
}
//POCO_SERVER_MAIN(WebSocketServer)
