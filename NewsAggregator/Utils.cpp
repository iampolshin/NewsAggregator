#include "Utils.h"

void Utils::trim(string& str) {
	str.erase(str.begin(), find_if_not(str.begin(), str.end(), [](char c) { return isspace(c); }));
	str.erase(find_if_not(str.rbegin(), str.rend(), [](char c) {return isspace(c); }).base(), str.end());
}

string Utils::getDomainName(string url) {
	int slashCount = 0;
	string domainName;
	for (char ch : url) {
		if (slashCount == 2) {
			domainName += ch;
		}
		if (ch == '/') {
			++slashCount;
		}
		if (slashCount > 2) {
			break;
		}
	}
	return domainName.substr(0, domainName.size() - 1) + ".xml";
}