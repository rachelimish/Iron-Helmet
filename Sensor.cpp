#include "Sensor.h"
#include <mutex>
#include <iostream>
#include <fstream>
#include <random>
std::mutex mtx;
using namespace std;
//	string line = "";
//	ifstream file("../Src/parameterim.txt");
//	file.is_open();
//	if (getline(file, line)) { // Read one line from the file
//		cout << "Read line: " << line << std::endl;
//	}
//	else {
//		cout << "Failed to read line from the file." << std::endl;
//	}
//	char delimiter = ',';
//	vector<string> outputStrings = {};
//	size_t pos = 0;
//	string token;
//	while ((pos = line.find(delimiter)) != string::npos) {
//		token = line.substr(0, pos);
//		outputStrings.push_back(token);
//		line.erase(0, pos + 1);
//	}
//	//return outputStrings;
//	//string Type = outputStrings.at(4);
//	//getline(file, line);
//	//cout << "Read line: " << outputStrings << std::endl;
//
//	//std::thread first(foo); 
//
//	//std::thread second(bar, "racheli");  // spawn new thread that calls bar(0)
//
//	//std::cout << "main, foo and bar now execute concurrently...\n";
//
//	// synchronize threads:
//	//second.detach();         // pauses until first finishes
//	//second.join(); 
//	//first.join(); // pauses until second finishes
//
//	//std::cout << "foo and bar completed.\n";
Sensor::Sensor(double* locationSensorOfSoldier)
{
	this->locationSensorOfSoldier[0] =locationSensorOfSoldier[0];
	this->locationSensorOfSoldier[1] =locationSensorOfSoldier[1];
	this->locationSensorOfSoldier[2] = locationSensorOfSoldier[2];
}

void Sensor::LocationUpdate(double* locationSensorOfSoldier)
{
	this->locationSensorOfSoldier[0] = locationSensorOfSoldier[0];
	this->locationSensorOfSoldier[1] = locationSensorOfSoldier[1];
	this->locationSensorOfSoldier[2] = locationSensorOfSoldier[2];
}

void Sensor::ShotDetectionAndAlert()
{
	random_device rd;
	mt19937 gen(rd());
	uniform_real_distribution<double> dis(this->locationSensorOfSoldier[0]-3, this->locationSensorOfSoldier[0]+3);
	double LocationX = dis(gen);
	uniform_real_distribution<double> dis1(this->locationSensorOfSoldier[1]-3, this->locationSensorOfSoldier[1] + 3);
	double LocationY = dis1(gen);
	uniform_real_distribution<double> dis2(this->locationSensorOfSoldier[2]-3, this->locationSensorOfSoldier[2] + 3);
	double LocationZ = dis2(gen);
	uniform_int_distribution<int> dis3(-180, 180);
	int DirectionOfTheShotFromTheToldier= dis3(gen);
	uniform_int_distribution<int> dis4(0, 3);
	int distance= dis4(gen);
	ofstream outputFile("../Src/parameterim.txt",ios::app);
	mtx.lock();
	if (outputFile.is_open())
	{
		outputFile << LocationX<<","<< LocationY<<","<< LocationZ<<","<< DirectionOfTheShotFromTheToldier<<"," << endl;

		outputFile.close();
		cout << "Data written to file successfully." << endl;
	}
	else 
	{
	 cerr << "Error opening the file for writing." << endl;
	}
	mtx.unlock();

}

double* Sensor::GettingTheLocation()
{
	return this->locationSensorOfSoldier;
}
