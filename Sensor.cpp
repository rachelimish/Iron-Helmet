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
 void Sensor::Shot_Detection_And_warning(Solider& solider)
{
	 random_device rd;
	 mt19937 gen(rd());
	 int When;

	 

	 while (true)
	 {
		 uniform_int_distribution<int> dis3(1, 60);
		 When = dis3(gen);
		 cout << When << endl;
		 std::this_thread::sleep_for(std::chrono::seconds(When));
		 cout << "i woke uppppppppppppp!!!" << endl;

		 // Get the updated soldier position in each iteration
		 double max_range_detectionX = solider.Get()[0] + (100 / 111000);
		 double min_range_detectionX = solider.Get()[0] - (100 / 111000);
		 double max_range_detectionY = solider.Get()[1] + (100 / 111000);
		 double min_range_detectionY = solider.Get()[1] - (100 / 111000);

		 uniform_real_distribution<double> disX(min_range_detectionX, max_range_detectionX);
		 uniform_real_distribution<double> disY(min_range_detectionY, max_range_detectionY);

		 double LocationX = disX(gen);
		 double LocationY = disY(gen);

		double DirectionOfTheShotFromTheSoldier = bearing_with_altitude(solider.Get()[0], solider.Get()[1], LocationX, LocationY);

		double distance = haversine_distance(solider.Get()[0], solider.Get()[1],LocationX,LocationY);
		mtx.lock();
		ofstream outputFile("./Src/data.txt", ios::app);

		if (outputFile.is_open())
		{
			outputFile << LocationX << "," << LocationY << "," << DirectionOfTheShotFromTheSoldier << "," << distance << "," << endl;

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

string Sensor::Receiving_And_Warning_From_The_Sensor()
{
	string alert = "";
	mtx.lock();
	ifstream file("./Src/data.txt");

	//file.is_open();
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
	std::ofstream outputFile("./Src/data.txt");
	for (const string& updatedLine : lines) {
		outputFile << updatedLine << std::endl;
	}
	outputFile.close();

	std::cout << "Line " << 1 << " deleted successfully from file." << std::endl;
	mtx.unlock();
	return alert;
}
// Function to calculate the distance between two points in 2 dimensions using Haversine formula in meters
double Sensor::haversine_distance(double lat1, double lon1, double lat2, double lon2)
{
	const double R = 6371000.0; // Earth radius in meters

	lat1 = M_PI * lat1 / 180.0;
	lon1 = M_PI * lon1 / 180.0;
	lat2 = M_PI * lat2 / 180.0;
	lon2 = M_PI * lon2 / 180.0;

	double dlat = lat2 - lat1;
	double dlon = lon2 - lon1;

	double a = sin(dlat / 2) * sin(dlat / 2) + cos(lat1) * cos(lat2) * sin(dlon / 2) * sin(dlon / 2);
	double c = 2 * atan2(sqrt(a), sqrt(1 - a));

	double distance = R * c;

	return distance;
}

// Function to calculate the initial bearing in degrees without altitude
double Sensor::bearing_with_altitude(double lat1, double lon1, double lat2, double lon2) {
	// Convert latitude and longitude from degrees to radians
	lat1 = lat1 * M_PI / 180.0;
	lon1 = lon1 * M_PI / 180.0;
	lat2 = lat2 * M_PI / 180.0;
	lon2 = lon2 * M_PI / 180.0;

	// Calculate differences in latitude and longitude
	double dlon = lon2 - lon1;

	// Calculate x, y components for bearing calculation
	double y = sin(dlon) * cos(lat2);
	double x = cos(lat1) * sin(lat2) - sin(lat1) * cos(lat2) * cos(dlon);

	// Calculate initial bearing in degrees
	double initial_bearing = atan2(y, x) * 180.0 / M_PI;
	initial_bearing = fmod(initial_bearing + 360, 360); // Normalize to [0, 360] degrees

	return initial_bearing;
}

