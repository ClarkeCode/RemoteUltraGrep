#pragma once
#include "WinSockets.hpp"

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

		void SendTo(networking::TCPSocket& socket) {
			socket.sendInfo<int>(_commandType);
			_sendTo(socket);
		};
		static void ReceiveUnknownCommand(RemoteCommand* commandPtr, networking::TCPSocket& socket) {
			//Make concrete command factory
		}

		//virtual void RecieveFrom(networking::TCPSocket& socket) = 0;
	protected:
		virtual void _sendTo(networking::TCPSocket& socket) = 0;

		//	virtual 
	};

	struct DropCommand : public RemoteCommand {
		DropCommand() : RemoteCommand(CommandEnum::DROP) {}
	protected:
		virtual inline void _sendTo(networking::TCPSocket& socket) override {};
	};

	struct RGrepCommandFactory {
		static RemoteCommand* generateCommand(CommandEnum signal) {
			switch (signal) {
				return &DropCommand();
			}
		}
	};
}