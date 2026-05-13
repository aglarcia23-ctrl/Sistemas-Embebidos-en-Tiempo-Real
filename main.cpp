//Code for find the maximum of 2 numbers
#include <iostream>
using namespace std;

int main()
{
	int x, y;
	cout << "Enter 2 numbers : " << endl;
	cin >> x; cout << endl;
	cin >> y; cout << endl;

	if(x > y){
		cout << "The maximum number is: " << x;
	}
	else {
		cout << "The maximum number is: " << y;
	}
	return 0;
}
