#pragma once


namespace networking {
#include <string>
#include <stdexcept>
#include <WinSock2.h>
#include <WS2tcpip.h>
#pragma comment (lib,"ws2_32.lib")

	enum NetworkFamily {
		IPv4,
		IPv6
	};

	class SocketException : public std::runtime_error {
	public:
		SocketException(std::string const& description, int errorCode) :
			runtime_error("[Socket Exception" + (errorCode == 0 ? "" : " - Code #" + std::to_string(errorCode))
				+ "]" + (description.empty() ? "" : ": " + description)) {}
		SocketException(std::string const& description = "") : SocketException(description, 0) {}
	};


	class WindowsSocketActivation {
		WSAData wsaData;
	public:
		WindowsSocketActivation() {
			int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
			if (result != 0) throw SocketException("WSAStartup failed", result);
		}
		~WindowsSocketActivation() { WSACleanup(); }
	};


	//Winsocket abstract class
	//=================================
	class WinSocket {
	protected:
		using byte_t = char;
		SOCKET _socket;
		int _family;

		std::string _ipAddrString;
		unsigned short _portNum;
		NetworkFamily _chosenProtocol;

	public:
		WinSocket(
			std::string const& ipAddress, 
			unsigned short portNumber, 
			NetworkFamily inetProtocol = NetworkFamily::IPv4) : 
			_socket(SOCKET_ERROR), _ipAddrString(ipAddress), _portNum(portNumber), _chosenProtocol(inetProtocol) {

			if (inetProtocol == NetworkFamily::IPv4)
				_family = AF_INET;
			else if (inetProtocol == NetworkFamily::IPv6)
				_family = AF_INET6;
			else
				throw SocketException("Invalid networking family");
		}
		virtual ~WinSocket() { closesocket(_socket); }

	protected:
		virtual void _registerIpAddress(std::string const& ipAddress, unsigned short portNumber) = 0;

	public:
		//Send/Receive info return the count of bytes received/sent
		template<typename T>
		int sendInfo(T& item) = delete;

		template<typename T>
		int receiveInfo(T& item) = delete;
	};


	//TCPSocket inherits from WinSocket
	//=================================
	class TCPSocket : public WinSocket {
	protected:
		sockaddr_in _inet4addr;
		sockaddr_in6 _inet6addr;
		
	public:
		TCPSocket(std::string const& ipAddress, unsigned short portNumber, NetworkFamily inetProtocol) :
			WinSocket(ipAddress, portNumber, inetProtocol), _inet4addr(), _inet6addr() {
			_registerIpAddress(ipAddress, portNumber);
		}
	protected:
		//Will be called in TCPSocket constructor
		virtual void _registerIpAddress(std::string const& ipAddress, unsigned short portNumber) override {
			_socket = socket(_family, SOCK_STREAM, IPPROTO_TCP);

			if (_family == AF_INET) {
				_inet4addr.sin_family = _family;
				_inet4addr.sin_port = htons(portNumber);
				inet_pton(_family, ipAddress.c_str(), &(_inet4addr.sin_addr));
			}
			else if (_family == AF_INET6) {
				_inet6addr.sin6_family = _family;
				_inet6addr.sin6_port = htons(portNumber);
				inet_pton(_family, ipAddress.c_str(), &(_inet6addr.sin6_addr));
			}
		}

	public:
		template<typename T>
		int sendInfo(T const& item) {
			return send(_socket, (byte_t*)&item, sizeof(item), 0);
		}

		template<typename T>
		int receiveInfo(T& item) {
			return recv(_socket, (byte_t*)&item, sizeof(item), 0);
		}
	};


	//Client TCP Sock
	class TCPClientSocket : public TCPSocket {
	public:
		TCPClientSocket(std::string const& ipAddress, unsigned short portNumber, NetworkFamily inetProtocol = NetworkFamily::IPv4) :
			TCPSocket(ipAddress, portNumber, inetProtocol) {
			_connectToServer();
		}
	protected:
		//Should be called in constructor, will block until connection is established
		void _connectToServer() {
			if (_family == AF_INET) {
				if (connect(_socket, (SOCKADDR*)&_inet4addr, sizeof(_inet4addr)) == SOCKET_ERROR)
					throw SocketException("Could not connect to the server", WSAGetLastError());
			}
			else if (_family == AF_INET6) {
				if (connect(_socket, (SOCKADDR*)&_inet6addr, sizeof(_inet6addr)) == SOCKET_ERROR)
					throw SocketException("Could not connect to the server", WSAGetLastError());
			}
		}

		//Alternate constructor for the WaitForConnection in the TCPServerSocket
		TCPClientSocket(SOCKET socket = SOCKET_ERROR, std::string const& ipAddress = "127.0.0.1", 
			unsigned short portNumber = 99999, NetworkFamily inetProtocol = NetworkFamily::IPv4) :
			TCPSocket(ipAddress, portNumber, inetProtocol) {
			_socket = socket;
		}
		friend class TCPServerSocket;
	};


	//Server TCP Sock
	class TCPServerSocket : public TCPSocket {
	public:
		TCPServerSocket(
			std::string const& ipAddress, unsigned short portNumber, NetworkFamily inetProtocol = NetworkFamily::IPv4, 
			int connectionQueueLength = 1) : TCPSocket(ipAddress, portNumber, inetProtocol) {
			_startListening(connectionQueueLength);
		}

		TCPClientSocket WaitForConnection() {
			SOCKET acceptedSocketHandle = SOCKET_ERROR;
			while (acceptedSocketHandle == SOCKET_ERROR)
				acceptedSocketHandle = accept(_socket, NULL, NULL);
			return TCPClientSocket(acceptedSocketHandle, _ipAddrString, _portNum, _chosenProtocol);
		}

	private:
		//Gets called in constructor
		void _startListening(int connectionQueueLength) {
			//Bind server address to the socket
			if (_family == AF_INET) {
				if (bind(_socket, (SOCKADDR*)&_inet4addr, sizeof(_inet4addr)) == SOCKET_ERROR)
					throw SocketException("Server address could not be bound to socket", WSAGetLastError());
			}
			else if (_family == AF_INET6) {
				if (bind(_socket, (SOCKADDR*)&_inet6addr, sizeof(_inet6addr)) == SOCKET_ERROR)
					throw SocketException("Server address could not be bound to socket", WSAGetLastError());
			}

			//Specify limit on the impending connection queue
			if (listen(_socket, connectionQueueLength) == SOCKET_ERROR)
				throw SocketException("Socket failed to listen for incoming calls", WSAGetLastError());
		}
	};
}