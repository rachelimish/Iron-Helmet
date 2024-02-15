
#include <string>
using namespace std;
class Sensor
{
    float* locationSensorOfSoldier[3];//Position of the sensor on the soldier
   // int direction;
public:
    Sensor();
    Sensor(double* locationSensorOfSoldier);
    void LocationUpdate(double* locationSensorOfSoldier);
    void ShotDetectionAndAlert();//Lottery of location within the range of the soldier (up to 3 meters) and writing to the file of the identification of the shot.

};