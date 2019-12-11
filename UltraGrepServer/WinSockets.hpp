#pragma once
/*
Created by Robert Clarke and Evan Burgess
Date: 2019-12-08
*/

namespace networking {
#include <string>
#include <memory>
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
		SocketException(std::string const& description, int errorCode, std::string exceptionType = "Socket") :
			runtime_error("[" + exceptionType + " Exception" + (errorCode == 0 ? "" : " - Code #" + std::to_string(errorCode))
				+ "]" + (description.empty() ? "" : ": " + description)) {}
		SocketException(std::string const& description = "") : SocketException(description, 0) {}
	};

	class WsaException : public SocketException {
	public:
		WsaException(std::string description) : SocketException(description, WSAGetLastError(), "WSA") {}
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
		std::string getIpAddressString() const { return _ipAddrString; }
		std::string getIpPortString() const { return _ipAddrString + ':' + to_string(_portNum); }

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
			if (_socket == SOCKET_ERROR)
				throw WsaException("Could not generate a valid socket to '" + ipAddress + ":" + to_string(portNumber) + "'");

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
			int check = send(_socket, (byte_t*)&item, sizeof(item), 0);
			if (check == SOCKET_ERROR) throw WsaException("Could not sucessfully send data type '" + std::string(typeid(T).name()) + "'");
			return check;
		}

		template<>
		int sendInfo<std::string>(std::string const& item) {
			unsigned short holder = (unsigned short)item.size(); //can be unified with line below
			int bytesSent =  sendInfo<unsigned short>(holder);
			if (bytesSent == SOCKET_ERROR)
				throw SocketException("Encountered an error sending character sequence signaller", WSAGetLastError());

			bytesSent = 0;
			for (char const& ch : item) {
				bytesSent += sendInfo<char>(ch);
			}
			return bytesSent;
		}

		template<typename T>
		int receiveInfo(T& item) {
			int check = recv(_socket, (byte_t*)&item, sizeof(item), 0);
			if (check == SOCKET_ERROR) throw WsaException("Could not sucessfully receive data type '" + std::string(typeid(T).name()) + "'");
			return check;
		}

		template<>
		int receiveInfo<std::string>(std::string& item) {
			unsigned short handshake = 0;
			int hsCheck = receiveInfo<unsigned short>(handshake);
			if (handshake != 0 && hsCheck != 2)
				throw SocketException("Failed to properly register character transfer");

			std::string ss;
			int bytesRecv = 0;
			char ch;
			for (unsigned short x = 0; x < handshake; ++x) {
				bytesRecv += receiveInfo<char>(ch);
				ss += ch;
			}

			if (bytesRecv > 0) //Only overwrite the string if information has been recieved
				item = ss;
			return bytesRecv;
		}
	};


	//Client TCP Sock
	class TCPClientSocket : public TCPSocket {
	public:
		TCPClientSocket(std::string ipAddress, unsigned short portNumber, NetworkFamily inetProtocol = NetworkFamily::IPv4) :
			TCPSocket(ipAddress, portNumber, inetProtocol) {
			_connectToServer();
		}

		//Alternate constructor for the WaitForConnection in the TCPServerSocket
		TCPClientSocket(SOCKET socket = SOCKET_ERROR, std::string const& ipAddress = "127.0.0.1",
			unsigned short portNumber = -1, NetworkFamily inetProtocol = NetworkFamily::IPv4) :
			TCPSocket(ipAddress, portNumber, inetProtocol) {
			_socket = socket;
		}
	protected:
		//Should be called in constructor, will block until connection is established
		void _connectToServer() {
			if (_family == AF_INET) {
				if (connect(_socket, (SOCKADDR*)&_inet4addr, sizeof(_inet4addr)) == SOCKET_ERROR)
					throw WsaException("Could not connect to the server");
			}
			else if (_family == AF_INET6) {
				if (connect(_socket, (SOCKADDR*)&_inet6addr, sizeof(_inet6addr)) == SOCKET_ERROR)
					throw WsaException("Could not connect to the server");
			}
		}

		
	};


	//Server TCP Sock
	class TCPServerSocket : public TCPSocket {
	public:
		TCPServerSocket(
			std::string const& ipAddress, unsigned short portNumber, NetworkFamily inetProtocol = NetworkFamily::IPv4, 
			int connectionQueueLength = 1) : TCPSocket(ipAddress, portNumber, inetProtocol) {
			_startListening(connectionQueueLength);
		}

		shared_ptr<TCPClientSocket> WaitForConnection() {
			SOCKET acceptedSocketHandle = SOCKET_ERROR;
			while (acceptedSocketHandle == SOCKET_ERROR)
				acceptedSocketHandle = accept(_socket, NULL, NULL);
			return make_shared<TCPClientSocket>(acceptedSocketHandle, _ipAddrString, _portNum, _chosenProtocol);
		}

	private:
		//Gets called in constructor
		void _startListening(int connectionQueueLength) {
			//Bind server address to the socket
			if (_family == AF_INET) {
				if (bind(_socket, (SOCKADDR*)&_inet4addr, sizeof(_inet4addr)) == SOCKET_ERROR)
					throw WsaException("Server address could not be bound to socket");
			}
			else if (_family == AF_INET6) {
				if (bind(_socket, (SOCKADDR*)&_inet6addr, sizeof(_inet6addr)) == SOCKET_ERROR)
					throw WsaException("Server address could not be bound to socket");
			}

			//Specify limit on the impending connection queue
			if (listen(_socket, connectionQueueLength) == SOCKET_ERROR)
				throw WsaException("Socket failed to listen for incoming calls");
		}
	};
}