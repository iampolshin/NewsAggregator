#include <iostream>
#include <Windows.h>
#include <cstdio>
#include "Service.h"

int main()
{
	SetConsoleOutputCP(CP_UTF8);
	Service& service = Service::getInstance();
	service.init();
	service.run();
	return 0;
}