#pragma once
#include "WinSockets.hpp"
#include <string>

namespace remote {

	enum CommandEnum {
		DROP,
		CONNECT,
		STOPSERVER,
		GREP
	};

	struct RemoteCommand {
	private:
		CommandEnum _commandType;
	public:
		RemoteCommand(CommandEnum commandSignifier) : _commandType(commandSignifier) {};
		virtual ~RemoteCommand() {}

		//perform action?

		//reception is handled by command factory
		void SendTo(networking::TCPSocket& socket) {
			socket.sendInfo<int>(_commandType);
			_sendTo(socket);
		};

	protected:
		virtual void _sendTo(networking::TCPSocket& socket) = 0;

		//	virtual 
	};

	struct DropCommand : public RemoteCommand {
		//DropCommand() : RemoteCommand(CommandEnum::DROP) {}
		DropCommand(networking::TCPSocket& socket) : RemoteCommand(CommandEnum::DROP) {
			
		}
	protected:
		virtual inline void _sendTo(networking::TCPSocket& socket) override {};
	};

	struct ConnectCommand : public RemoteCommand {
		std::string connectIp;
		ConnectCommand(std::string whereTo) : RemoteCommand(CommandEnum::CONNECT), connectIp(whereTo) {};
		ConnectCommand(networking::TCPSocket& socket) : RemoteCommand(CommandEnum::CONNECT) {
			socket.receiveInfo<std::string>(connectIp);
			//socket.sendInfo<bool>(true); //send acknowledgement
		}
	protected:
		virtual void _sendTo(networking::TCPSocket& socket) override {
			socket.sendInfo<std::string>(connectIp);
		};
	};

	struct RGrepCommandFactory {
		static RemoteCommand* generateCommand(networking::TCPSocket& socket) {
			CommandEnum commSignal;
			socket.receiveInfo<CommandEnum>(commSignal);

			switch (commSignal) {
			case CommandEnum::DROP:
				return &DropCommand(socket);
			case CommandEnum::CONNECT:
				return &ConnectCommand(socket);
			}
		}
	};
}