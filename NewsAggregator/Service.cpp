#include <iostream>
#include <fstream>
#include <sstream>
#include <cctype>
#include "../packages/curl/include/curl/curl.h"
#include "Service.h"
#include "Utils.h"

vector<thread> threads;
mutex Service::createFileMtx;
mutex Service::addAriticleMtx;
map<string, string> Service::store;
unordered_set<Article, Article::ArtileHash> Service::feed;
const char* Service::CONFIG_PATH = "config.ini";

Service& Service::getInstance() {
	static Service instance;
	return instance;
}

void Service::init() {
	while (!Service::readConfigFile()) {
		cout << "The configuration file does not contain valid links." << endl;
		cout << "Without it, it is impossible to continue working." << endl;
		cout << "Want to create your own configuration? (1)" << endl;
		bool input;
		cin >> input;
		if (!input || !writeConfigFile()) {
			return;
		}
	}
}

void Service::run() {
	unordered_set<Article, Article::ArtileHash> matches;
	string keyWord;
	int counter = 0;
	while (true) {
		cout << "\nEnter a word to search for news (or an empty string to exit): ";
		getline(cin, keyWord);
		Utils::trim(keyWord);
		if (keyWord.empty()) {
			break;
		}

		if (++counter % 2 == 0) {
			cout << "Updating news..." << endl;
			updateNews();
		}

		matches = Service::getMatchingArticles(keyWord);
		if (matches.empty()) {
			cout << "No news by word \"" << keyWord << "\"." << endl;
			continue;
		}

		cout << matches.size() << " news found for this word:" << endl;
		unordered_set<Article, Article::ArtileHash>::iterator it = matches.begin();
		for (int i = 1; it != matches.end(); ++it, ++i) {
			cout << i << ".) \"" << (*it).title << "\" [" << (*it).url << "]" << endl;
		}
	};

	for (auto resource : store) {
		remove(resource.second.c_str());
	}
}

bool Service::readConfigFile() {
	ifstream fileSource(CONFIG_PATH);
	if (!fileSource) {
		cout << "Configuration file not found." << endl;
		cout << "Without it, it is impossible to continue working." << endl;
		cout << "Want to create your own configuration? (1)" << endl;
		bool input;
		cin >> input;
		if (!input || !writeConfigFile()) {
			return false;
		}
		fileSource.open(CONFIG_PATH);
	}

	string url;
	string fileName;
	string buffer;
	int resourcesCount = 0;
	cout << "Please wait until we upload the necessary files." << endl;
	while (fileSource >> buffer)
	{
		url = buffer;
		fileName = Utils::getDomainName(url);
		if (isValidResource(url, fileName)) {
			++resourcesCount;
			store[url] = fileName;
		}
	}
	fileSource.close();
	cout << "Resources available: " << resourcesCount << endl;
	return resourcesCount != 0;
}

bool Service::writeConfigFile() {
	cout << "On a new line, enter a link to RSS. Example:" << endl;
	cout << "https://rss.nytimes.com/services/xml/rss/nyt/Europe.xml" << endl;

	ofstream fileSink(CONFIG_PATH);
	if (!fileSink) {
		cout << "Failed to create configuration file. Shutdown..." << endl;
		return false;
	}

	cout << "Enter 0 to complete the input!" << endl;
	string output;
	do {
		getline(cin, output);
		fileSink << output << endl;
	} while (output[0] != '0');
	return true;
}

bool Service::isValidResource(string url, string path) {
	getResponse(url, path);
	saveResponse(path);
	std::ifstream in(path, std::ifstream::ate | std::ifstream::binary);
	return in.tellg();
}

unordered_set<Article, Article::ArtileHash> Service::getMatchingArticles(string keyWord) {
	unordered_set<Article, Article::ArtileHash> matches;
	Article tempArticle;
	for (Article article : feed) {
		if (article.title.find(keyWord) != string::npos || article.desc.find(keyWord) != string::npos) {
			matches.insert(article);
		}

		tempArticle = article;
		transform(tempArticle.title.begin(), tempArticle.title.end(), tempArticle.title.begin(), ::tolower);
		transform(tempArticle.desc.begin(), tempArticle.desc.end(), tempArticle.desc.begin(), ::tolower);
		if (tempArticle.title.find(keyWord) != string::npos || tempArticle.desc.find(keyWord) != string::npos) {
			matches.insert(article);
		}
	}
	return matches;
}

void Service::getResponse(string url, string path)
{
	CURL* curl = curl_easy_init();
	curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
	FILE* file;
	fopen_s(&file, path.c_str(), "w");
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, file);
	curl_easy_perform(curl);
	curl_easy_cleanup(curl);
	if (file) {
		fclose(file);
	}
}

void Service::saveResponse(string path) {
	pugi::xml_document document;
	document.load_file(path.c_str());
	for (pugi::xml_node node : document.child("rss").child("channel").children()) {
		if (strcmp("item", string(node.name()).c_str()) == 0) {
			addAriticleMtx.lock();
			feed.insert(Article(node));
			addAriticleMtx.unlock();
		}
	}
}

void Service::updateNews() {
	feed.clear();
	threads.clear();

	string url;
	string fileName;
	ostringstream os;
	for (auto resource : store) {
		url = resource.first;
		fileName = resource.second;
		threads.push_back(thread([url, fileName]() {
			getResponse(url, fileName);
		saveResponse(fileName);
			}));
	}

	for (int i = 0; i < threads.size(); i++) {
		threads[i].join();
	}
}
