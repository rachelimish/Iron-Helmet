#pragma once
#include <string>
using namespace std;
class Solider
{
	double* location = new double[3];//Position of the sensor on the soldier
public:
    Solider();
    Solider(double* location);
    double* Get();//A function returned the location
    void Update(double* location);
};

