#include "Sensor.h"
#include <mutex>
#include <iostream>
#include <fstream>
#include <random>
using namespace std;
std::mutex mtx;
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
	while (true) 
	{
		random_device rd;
		mt19937 gen(rd());
		uniform_int_distribution<int> dis3(1, 70);
		int When = dis3(gen);
		std::chrono::steady_clock::time_point current_time = std::chrono::steady_clock::now();
		std::chrono::seconds duration(When);
		std::chrono::steady_clock::time_point newTime = current_time + duration;
		auto elapsed_time = std::chrono::duration_cast<std::chrono::seconds>(newTime-current_time).count();
		while (elapsed_time > 0)
		{
			std::chrono::steady_clock::time_point current_time = std::chrono::steady_clock::now();
			auto elapsed_time = std::chrono::duration_cast<std::chrono::seconds>(newTime - current_time).count();
		}
		uniform_real_distribution<double> dis(this->locationSensorOfSoldier[0] - 3, this->locationSensorOfSoldier[0] + 3);
		double LocationX = dis(gen);
		uniform_real_distribution<double> dis1(this->locationSensorOfSoldier[1] - 3, this->locationSensorOfSoldier[1] + 3);
		double LocationY = dis1(gen);
		uniform_real_distribution<double> dis2(this->locationSensorOfSoldier[2] - 3, this->locationSensorOfSoldier[2] + 3);
		double LocationZ = dis2(gen);
		uniform_int_distribution<int> dis3(-180, 180);
		int DirectionOfTheShotFromTheToldier = dis3(gen);
		uniform_int_distribution<int> dis4(0, 3);
		int distance = dis4(gen);
		ofstream outputFile("../Src/parameterim.txt", ios::app);
		mtx.lock();
		if (outputFile.is_open())
		{
			outputFile << LocationX << "," << LocationY << "," << LocationZ << "," << DirectionOfTheShotFromTheToldier << "," << endl;

			outputFile.close();
			cout << "Data written to file successfully." << endl;
		}
		else
		{
			cerr << "Error opening the file for writing." << endl;
		}
		mtx.unlock();

	}
}

double* Sensor::GettingTheLocation()
{
	return this->locationSensorOfSoldier;
}

string Sensor::ReceivingAnAlertFromTheSensor()
{
	string alert = "";
	ifstream file("../Src/parameterim.txt");
	mtx.lock();
	file.is_open();
	if (file.peek() == EOF) {
		mtx.unlock();
		/*std::cout << "There are no alerts available from the sensor" << endl;*/
		return "";
	}
	if (getline(file,alert)) { // Read one line from the file
		std::cout << "Read line: " << alert << endl;
	}
	else {
		mtx.unlock();
		std::cout << "Failed to read line from the file." << endl;
		return "";
	}
	vector<std::string> lines;
	string line;

	if (file.peek() == EOF)
	{
		lines.push_back(alert);
	}
	// Read all lines from the file
 	while (getline(file, line)) {
		lines.push_back(line);
	}
	file.close();

	// Remove the line from the vector
	lines.erase(lines.begin());

	// Write the updated content back to the file
	std::ofstream outputFile("../Src/parameterim.txt");
	for (const string& updatedLine : lines) {
		outputFile << updatedLine << std::endl;
	}
	outputFile.close();

	std::cout << "Line " << 1 << " deleted successfully from file." << std::endl;
	mtx.unlock();
	return alert;
}
