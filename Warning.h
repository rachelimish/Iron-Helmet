#include <string>
#include <cmath>
#include <thread>
#include <SFML/Graphics.hpp>
#include <chrono>
using namespace std;
#pragma once
class Warning
{
	string Name;
	std::chrono::steady_clock::time_point TimeStart;
public:
	Warning();
	Warning(string NameAlert);
	std::chrono::steady_clock::time_point GetTimeStart();
	string GetName();
};

