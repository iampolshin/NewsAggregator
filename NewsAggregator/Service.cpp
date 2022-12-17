#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cctype>
#include "../packages/curl/include/curl/curl.h"
#include "Service.h"

static void trim(string& s);
static int number = 0;
static int counter = 0;

mutex Service::mtx;
vector<thread> threads;
vector<string> Service::resources;
vector<string> Service::tempFiles;
unordered_set<Article, Article::ArtileHash> Service::feed;
const char* Service::CONFIG_PATH = "config.ini";

Service& Service::getInstance() {
	static Service instance;
	return instance;
}


void Service::init() {
	if (Service::readConfigFile()) {
		updateNews();
	}
}

void Service::updateNews() {
	feed.clear();
	threads.clear();
	number = 0;

	string fileName;
	ostringstream os;
	for (string url : resources) {
		threads.push_back(thread([&, url]() {
			fileName = createTempFile(os);
		getResponse(url, fileName);
		saveResponse(fileName);
			}));
	}
	for (int i = 0; i < threads.size(); i++) {
		threads[i].join();
	}
}

void Service::run() {
	string keyWord;
	unordered_set<Article, Article::ArtileHash> matches;
	while (true) {
		cout << "\nEnter a word to search for news (or an empty string to exit): ";
		getline(cin, keyWord);
		cin >> keyWord;
		trim(keyWord);
		if (keyWord.empty()) {
			break;
		}

		if (++counter % 2 == 0) {
			cout << "Updating..." << endl;
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

	for (string file : tempFiles) {
		remove(file.c_str());
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
			cout << "Shutdown..." << endl;
			return false;
		}
	}

	int resourceCount = 0;
	string buffer;
	while (fileSource >> buffer)
	{
		if (isValidResource(buffer)) {
			++resourceCount;
			resources.push_back(buffer);
		}
	}

	cout << "Resources available: " << resourceCount << endl;
	return resourceCount != 0;
}

bool Service::writeConfigFile() {
	cout << "On a new line, enter a link to RSS. Example:" << endl;
	cout << "https://rss.nytimes.com/services/xml/rss/nyt/Europe.xml" << endl;

	ofstream fileSink(CONFIG_PATH);

	if (!fileSink) {
		cout << "Failed to create configuration file. Shutdown..." << endl;
		return false;
	}

	cout << "Enter an empty string to complete input!" << endl;
	string output;
	cin >> output;
	while (!output.empty()) {
		fileSink << output << endl;
		getline(cin, output);
	}
	return true;
}

bool Service::isValidResource(string resource) {
	return resource.find("xml") != string::npos || resource.find("rss") != string::npos;
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

string Service::createTempFile(ostringstream& os) {
	mtx.lock();
	string fileName;
	fileName.append("temp");
	fileName.append(to_string(++number));
	fileName.append(".xml");
	tempFiles.push_back(fileName);
	os << fileName;
	mtx.unlock();
	return fileName;
}

void Service::saveResponse(string file) {
	pugi::xml_document document;
	document.load_file(file.c_str());
	for (pugi::xml_node node : document.child("rss").child("channel").children()) {
		if (strcmp("item", string(node.name()).c_str()) == 0) {
			mtx.lock();
			feed.insert(Article(node));
			mtx.unlock();
		}
	}
}

static void trim(string& s) {
	s.erase(s.begin(), find_if_not(s.begin(), s.end(), [](char c) { return isspace(c); }));
	s.erase(find_if_not(s.rbegin(), s.rend(), [](char c) { return isspace(c); }).base(), s.end());
}