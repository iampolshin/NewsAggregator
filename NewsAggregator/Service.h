#pragma once
#include <vector>
#include <map>
#include <unordered_set>
#include <mutex>
#include "Article.h"

class Service {
private:
	static unordered_set<Article, Article::ArtileHash> feed;
	static map<string, string> store;
	static mutex createFileMtx;
	static mutex addAriticleMtx;

	Service() {};
	Service(const Service&);

	static bool writeConfigFile();
	static bool readConfigFile();
	static void getResponse(string, string);
	static void updateNews();
	static void saveResponse(string);
	static bool isValidResource(string, string);

	Service& operator=(Service&);
public:
	static const char* CONFIG_PATH;
	static Service& getInstance();
	static void init();
	static void run();
	static unordered_set<Article, Article::ArtileHash> getMatchingArticles(string);
};