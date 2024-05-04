#include "Sensor.h"
#include <mutex>
#include <iostream>
#include <fstream>
#include <random>
#include <cmath>
using namespace std;
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
std::mutex mtx;
Sensor::Sensor()
{
}
Sensor::Sensor(const double* locationSensorOfSoldier)
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
		uniform_int_distribution<int> dis3(1, 60);
		int When = dis3(gen);
		cout << When << endl;
		std::this_thread::sleep_for(std::chrono::seconds(When));
		cout << "i wake uppppppppppppp!!!" << endl;
		/*std::chrono::steady_clock::time_point current_time = std::chrono::steady_clock::now();
		std::chrono::seconds duration(When);
		std::chrono::steady_clock::time_point newTime = current_time + duration;
		auto elapsed_time = std::chrono::duration_cast<std::chrono::seconds>(newTime-current_time).count();
		while (elapsed_time > 0)
		{
			std::chrono::steady_clock::time_point current_time = std::chrono::steady_clock::now();
			elapsed_time = std::chrono::duration_cast<std::chrono::seconds>(newTime - current_time).count();
		}*/
		double max_range_detectionX = this->locationSensorOfSoldier[0] +(100/111000);
		double min_range_detectionX = this->locationSensorOfSoldier[0] -(100 / 111000);
		double max_range_detectionY = this->locationSensorOfSoldier[1] + (100 / 111000);
		double min_range_detectionY = this->locationSensorOfSoldier[1] - (100 / 111000);
		double max_range_detectionZ = this->locationSensorOfSoldier[2] + 100;
		double min_range_detectionZ = this->locationSensorOfSoldier[2] - 100;

		uniform_real_distribution<double> dis(min_range_detectionX, max_range_detectionX);
		double LocationX = dis(gen);
		uniform_real_distribution<double> dis1(min_range_detectionY, max_range_detectionY);
		double LocationY = dis1(gen);
		uniform_real_distribution<double> dis2(min_range_detectionZ, max_range_detectionZ);
		double LocationZ = dis2(gen);

		double DirectionOfTheShotFromTheSoldier = bearing_with_altitude(locationSensorOfSoldier[0], locationSensorOfSoldier[1], locationSensorOfSoldier[2], LocationX, LocationY, LocationZ);

		double distance = haversine_distance(locationSensorOfSoldier[0], locationSensorOfSoldier[1], locationSensorOfSoldier[2],LocationX,LocationY,LocationZ);
		
		ofstream outputFile("../Src/data.txt", ios::app);
		mtx.lock();
		if (outputFile.is_open())
		{
			outputFile << LocationX << "," << LocationY << "," << LocationZ << "," << DirectionOfTheShotFromTheSoldier << "," << distance << "," << endl;

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
	ifstream file("../Src/data.txt");
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
	std::ofstream outputFile("../Src/data.txt");
	for (const string& updatedLine : lines) {
		outputFile << updatedLine << std::endl;
	}
	outputFile.close();

	std::cout << "Line " << 1 << " deleted successfully from file." << std::endl;
	mtx.unlock();
	return alert;
}

// Function to calculate the distance using Haversine formula in meters
double Sensor::haversine_distance(double lat1, double lon1, double elev1, double lat2, double lon2, double elev2)
{
	
	const double R = 6371000.0; // Earth radius in meters

	lat1 = M_PI * lat1 / 180.0;
	lon1 = M_PI * lon1 / 180.0;
	lat2 = M_PI * lat2 / 180.0;
	lon2 = M_PI * lon2 / 180.0;

	double dlat = lat2 - lat1;
	double dlon = lon2 - lon1;
	double dh = elev2 - elev1;

	double a = sin(dlat / 2) * sin(dlat / 2) + cos(lat1) * cos(lat2) * sin(dlon / 2) * sin(dlon / 2);
	double c = 2 * atan2(sqrt(a), sqrt(1 - a));

	double distance = R * c;

	return distance;
}
// Function to calculate the initial bearing in degrees with altitude
double bearing_with_altitude(double lat1, double lon1, double elev1, double lat2, double lon2, double elev2) {
	// Convert latitude and longitude from degrees to radians
	lat1 = lat1 * M_PI / 180.0;
	lon1 = lon1 * M_PI / 180.0;
	lat2 = lat2 * M_PI / 180.0;
	lon2 = lon2 * M_PI / 180.0;

	// Calculate differences in latitude, longitude, and altitude
	double dlon = lon2 - lon1;

	// Calculate x, y components for bearing calculation
	double y = sin(dlon) * cos(lat2);
	double x = cos(lat1) * sin(lat2) - sin(lat1) * cos(lat2) * cos(dlon);

	// Calculate initial bearing in degrees
	double initial_bearing = atan2(y, x) * 180.0 / M_PI;
	initial_bearing = fmod(initial_bearing + 360, 360); // Normalize to [0, 360] degrees

	return initial_bearing;
}
