//Test Pipe @SiriusNEO

#include <iostream>

using namespace std;

int main() {
	string cmd;
	int a, b, debugCnt = 0;
	while (true) {
		++debugCnt;
		cin >> cmd >> a >> b;
		cerr << "debug" + to_string(debugCnt) << endl;

		if (cmd == "add") cout << to_string(a+b) << endl;
		if (cmd == "exit") {
			cout << "bye" << endl;
			return 0;
			}
		}

	return 0;
}
