
#include "Solider.h"
#include <string>
using namespace std;
class Sensor
{
public:
    Sensor();
    void Shot_Detection_And_warning(Solider & solider);//A function that generates every time we will generate data for the sensor and write to a file (the function generates a location within 3 meters of the soldier, direction and distance).
    string Receiving_And_Warning_From_The_Sensor();//A function that reads a line from the file and deletes and returns the read line.
    // Function to calculate the distance using Haversine formula in meters
    double haversine_distance(double lat1, double lon1, double lat2, double lon2);
    // Function to calculate the initial bearing in degrees with altitude
    double bearing_with_altitude(double lat1, double lon1, double lat2, double lon2);
};