
#include <string>
using namespace std;
class Sensor
{
    double* locationSensorOfSoldier= new double[3];//Position of the sensor on the soldier
   // int direction;
public:
    Sensor();
    Sensor(const double* locationSensorOfSoldier);
    void LocationUpdate(double* locationSensorOfSoldier);
    void ShotDetectionAndAlert();//A function that generates every time we will generate data for the sensor and write to a file (the function generates a location within 3 meters of the soldier, direction and distance).
    double* GettingTheLocation();//A function returned the position of the soldier
    string ReceivingAnAlertFromTheSensor();//A function that reads a line from the file and deletes and returns the read line.
    // Function to calculate the distance using Haversine formula in meters
    double haversine_distance(double lat1, double lon1, double elev1, double lat2, double lon2, double elev2);
    // Function to calculate the initial bearing in degrees with altitude
    double bearing_with_altitude(double lat1, double lon1, double elev1, double lat2, double lon2, double elev2);
};