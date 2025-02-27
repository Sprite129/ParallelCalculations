#include <iostream>
using namespace std;

int main() {

	int choice = 0;

	cout << "Select matrix size" << endl;

	cin >> choice;

	switch (choice)
	{
	case 1:
		cout << "Size 120" << endl;
		break;

	case 2:
		cout << "Size 1200" << endl;
		break;

	case 3:
		cout << "Size 12000" << endl;
		break;

	default:
		cout << "invalid" << endl;
		break;
	}

	return 0;
}