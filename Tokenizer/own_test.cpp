#include<iostream>
using namespace std;
int main(void)
{
	int a, b, c;
	cin >> a >> b >> c;
	if (a > b) {
		int tmp = a;
		a = b;
		b = tmp;
	} // a<=b 
	a = 1 + 1;
	b = c +1;
	return 0;
}