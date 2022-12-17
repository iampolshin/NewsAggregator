#include <iostream>
#include <Windows.h>
#include <cstdio>
#include "Service.h"

int main()
{
	cout << "Use only English characters, please!" << endl;
	SetConsoleCP(CP_UTF8);
	SetConsoleOutputCP(CP_UTF8);
	Service& service = Service::getInstance();
	service.init();
	service.run();
	return 0;
}