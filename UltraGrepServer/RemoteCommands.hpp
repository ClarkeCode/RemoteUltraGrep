/*
Created by Robert Clarke and Evan Burgess
Date: 2019-12-08
*/
#pragma once
#include "WinSockets.hpp"
#include <string>

#include <algorithm>
namespace remote {

	enum CommandEnum {
		NOACTION,
		DROP,
		CONNECT,
		STOPSERVER,
		GREP,
		
		RESPONSE,
		RESPONSETERMINATION
	};

	struct RemoteCommand {
		CommandEnum _commandType;
		std::string commandSignifier;
		std::string arguments;
		bool isValid;

		RemoteCommand(std::string userInput, std::string commandSig, CommandEnum commType = NOACTION) :
			commandSignifier(commandSig), _commandType(commType), isValid(true) {
			if (userInput == commandSignifier)
				arguments = "";
			else {
				arguments = trimString(string(
					mismatch(commandSignifier.begin(), commandSignifier.end(), userInput.begin(), userInput.end()).second, userInput.end()
				));
			}
		}

		inline static string trimString(string const& untrimmed) {
			string leftTrim(untrimmed.begin() + untrimmed.find_first_not_of(' '), untrimmed.end());
			return string(leftTrim.begin(), leftTrim.begin() + leftTrim.find_last_not_of(' ') + 1);
		}
	};

	struct DropCommand : public RemoteCommand {
		DropCommand(std::string userInput) : RemoteCommand(userInput, "drop", DROP) {}
	};
	struct StopServerCommand : public RemoteCommand {
		StopServerCommand(std::string userInput) : RemoteCommand(userInput, "stopserver", STOPSERVER) {}
	};
	struct ConnectCommand : public RemoteCommand {
		ConnectCommand(std::string userInput) : RemoteCommand(userInput, "connect", CONNECT) {
			isValid = regex_match(arguments, regex(R"ipv4format((?:\d{1,3}\.){3}\d{1,3})ipv4format"));
		}
	};
	struct GrepCommand : public RemoteCommand {
		GrepCommand(std::string userInput) : RemoteCommand(userInput, "grep", GREP) {
			//isValid = regex_match(arguments, regex("grep (?:-v )?\\S*(?: ?\\S*)"));
		}
	};

	/*
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


	struct AcknowledgementCommand : public RemoteCommand {
		AcknowledgementCommand() : RemoteCommand(CommandEnum::ACKNOWLEDGEMENT) {}
		AcknowledgementCommand(networking::TCPSocket& socket) : RemoteCommand(CommandEnum::ACKNOWLEDGEMENT) {}
	protected:
		virtual inline void _sendTo(networking::TCPSocket& socket) override {};
	};

	struct DropCommand : public RemoteCommand {
		DropCommand() : RemoteCommand(CommandEnum::DROP) {}
		DropCommand(networking::TCPSocket& socket) : RemoteCommand(CommandEnum::DROP) {}
	protected:
		virtual inline void _sendTo(networking::TCPSocket& socket) override {};
	};

	struct ConnectCommand : public RemoteCommand {
		std::string connectIp;
		ConnectCommand(std::string whereTo) : RemoteCommand(CommandEnum::CONNECT), connectIp(whereTo) {};
		ConnectCommand(networking::TCPSocket& socket) : RemoteCommand(CommandEnum::CONNECT) {
			socket.receiveInfo<std::string>(connectIp);
		}
	protected:
		virtual void _sendTo(networking::TCPSocket& socket) override {
			socket.sendInfo<std::string>(connectIp);
		};
	};

	struct StopServerCommand : public RemoteCommand {
		StopServerCommand() : RemoteCommand(CommandEnum::STOPSERVER) {}
		StopServerCommand(networking::TCPSocket& socket) : RemoteCommand(CommandEnum::STOPSERVER) {}
	protected:
		virtual inline void _sendTo(networking::TCPSocket& socket) override {};
	};

	struct GrepCommand : public RemoteCommand {
		std::string grepCommandString;
		GrepCommand(std::string inputString) : RemoteCommand(CommandEnum::GREP), grepCommandString(inputString) {}
		GrepCommand(networking::TCPSocket& socket) : RemoteCommand(CommandEnum::GREP) {
			socket.receiveInfo<std::string>(grepCommandString);
		}
	protected:
		virtual inline void _sendTo(networking::TCPSocket& socket) override {
			socket.sendInfo<std::string>(grepCommandString);
		};
	};



	struct RGrepCommandFactory { //return shared pointers to instances by the examples maps[]
		static RemoteCommand* generateCommand(networking::TCPSocket& socket) {
			CommandEnum commSignal;
			socket.receiveInfo<CommandEnum>(commSignal);

			switch (commSignal) {
			case CommandEnum::ACKNOWLEDGEMENT:
				return &AcknowledgementCommand(socket);
			case CommandEnum::DROP:
				return &DropCommand(socket);
			case CommandEnum::CONNECT:
				return &ConnectCommand(socket);
			case CommandEnum::STOPSERVER:
				return &StopServerCommand(socket);
			case CommandEnum::GREP:
				return &GrepCommand(socket);
			}
		}
	};
	*/
}