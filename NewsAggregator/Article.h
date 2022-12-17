#pragma once
#include <string>
#include <pugixml.hpp>

using namespace std;

struct Article
{
	struct ArtileHash
	{
		int operator()(const Article& article) const
		{
			return hash<string>()(article.url + article.title + article.desc);
		}
	};

	string url;
	string title;
	string desc;

	Article() {};
	Article(pugi::xml_node node) {
		url = node.child("link").text().as_string();
		title = node.child("title").text().as_string();
		desc = node.child("description").text().as_string();
	}

	bool operator== (const Article& other) const
	{
		string otherStr = other.title + other.url + other.desc;
		string currStr = title + url + desc;
		return currStr == otherStr;
	}
};