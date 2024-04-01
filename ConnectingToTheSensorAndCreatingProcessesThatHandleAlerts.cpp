#include <SFML/Graphics.hpp>
#include <cmath>
#include <iostream>
#include <mutex>
#include "Sensor.h"
void WhichWillEditALocation(string alert);
int main()
{
    int mone = 0;
    double arr[3];
    arr[0] =153.333;
    arr[1] = 253.2;
    arr[2] = 3.5;
    Sensor s1(arr);
   /* for(int i=0;i<10;i++)*/
      /*  s1.ShotDetectionAndAlert();*/
    string line="";
    string WarningName;
    while ((line=s1.ReceivingAnAlertFromTheSensor() )!= "")
    {
        WarningName = "Warning" +to_string(mone);
        thread WarningName(WhichWillEditALocation, line);
        mone++;
    }
}
void WhichWillEditALocation(string Warning) 
{
    char delimiter = ',';
   	vector<double> outputStrings = {};
   	size_t pos = 0;
   	string token;
   	while ((pos = Warning.find(delimiter)) != string::npos) {
   		token = Warning.substr(0, pos);
   		outputStrings.push_back(stod(token));
        Warning.erase(0, pos + 1);
        cout << Warning;
   	}

}