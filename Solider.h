#pragma once
#include <string>
#include <iostream>
#include <cmath> 
#include <random>
#include <thread>
#include <chrono>
#include <nlohmann/json.hpp>
#include <corecrt_math_defines.h>
//#include <wininet.h>
#include <curl/curl.h>
#include <Windows.h>
#include <winhttp.h>
#include <fstream>
//#pragma comment(lib, "wininet.lib")
using json = nlohmann::json;
using namespace std;
class Solider
{
	double* location = new double[2];//Position of the sensor on the soldier
    string apiKey = "";
public:
    // Static member variable to store the response data
    Solider();
    Solider(double* location);
    double* Get();//A function returned the location
    void Update();
    static size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* data);
    void Thread_location();
};

