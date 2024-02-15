#include <SFML/Graphics.hpp>
#include <cmath>
#include <iostream>
#include <mutex>
#include "Sensor.h"
int main()
{
    double arr[3];
    arr[0] =153.333;
    arr[1] = 253.2;
    arr[2] = 3.5;
    Sensor s1(arr);
    for(int i=0;i<3;i++)
        s1.ShotDetectionAndAlert();
    return 0;
}