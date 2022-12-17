#pragma once
#include <vector>
#include <unordered_set>
#include <mutex>
#include "Article.h"

class Service {
private:
	static unordered_set<Article, Article::ArtileHash> feed;
	static vector<string> resources;
	static vector<string> tempFiles;
	static mutex mtx;

	Service() {}
	Service(const Service&);
	Service& operator=(Service&);

	static bool writeConfigFile();
	static bool readConfigFile();
	static void getResponse(string, string);
	static string createTempFile(ostringstream&);
	static void updateNews();
	static void saveResponse(string);
	static bool isValidResource(string);
public:
	static const char* CONFIG_PATH;
	static Service& getInstance();
	static void init();
	static void run();
	static unordered_set<Article, Article::ArtileHash> getMatchingArticles(string);
};