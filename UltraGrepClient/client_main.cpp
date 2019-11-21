
#include <string>
#include <iostream>
using namespace std;
#include "../UltraGrepServer/WinSockets.hpp"

int main(int argc, char* argv[]) {
	cout << "Working" << endl;


	{
		networking::WindowsSocketActivation wsa;
		networking::TCPClientSocket client("127.0.0.1", 55444);

		client.sendInfo<unsigned short>(8989);
		client.sendInfo<string>("Gimme something from client");

		unsigned short xx;
		string yy;
		client.receiveInfo<unsigned short>(xx);
		client.receiveInfo<string>(yy);
		cout << xx << endl << yy << endl;

		char ch;
		cin >> ch;

	}
}