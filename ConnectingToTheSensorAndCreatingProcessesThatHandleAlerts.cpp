#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <SFML/Audio/SoundBuffer.hpp>
#include <SFML/Audio/Sound.hpp>
#include <cmath>
#include <iostream>
#include <mutex>
#include <vector>
#include "Sensor.h"
#include <corecrt_math_defines.h>
#include "warning.h"
#include <string>
#include <GL/freeglut.h>
#include "Solider.h"
#include <gmp.h>
#include <Windows.h>
#include <winhttp.h>
#include <fstream>
#include <nlohmann/json.hpp>
#include <wininet.h>
#include <curl/curl.h>
#include <functional>
#include <limits>
#include <map>
#include <future>
#include <shared_mutex>
void idle();
void display();
void init();
void drawGreenDot(); 
void drawRedDots();
void drawYellowDots();
void Remove_Value(std::vector<GLfloat> newRedDot);
void Add_Value(std::vector<GLfloat> newRedDot);
static double bearing_with_altitude(double lat1, double lon1, double lat2, double lon2);
bool if_in_the_firing_line(Solider& solider, double locationX, double locationY, double LocationX2, double LocationY2);
static double haversine_distance(double lat1, double lon1, double lat2, double lon2);
static void Receiving_An_Alert_And_Calculating_Additional_Data(string alert, Solider& solider);
void Create_A_Thread_For_Each_Warning(Solider& solider, Sensor& sensor);
std::vector<double> calculate_Bullet_Position(Solider& solider, double x0, double y0, double initVelocityX, double time, double Distance);
std::vector<GLfloat> farthestPoint(Solider& soldier, double shootingAngle,double distance2,double timeOfFlight, double totalVelocity);
std::vector<double> calculate_next_p(Solider& solider, double p1X, double p1Y, double k, double l);
static double velocity;
double X_Map(double pointX, Solider& solider);
double Y_Map(double pointY, Solider& solider);
std::vector<std::vector<GLfloat>> redDotPositions;
std::vector<std::vector<GLfloat>> yellowDotPositions;
std::shared_mutex mtx_GL;
static string warning;
std::mutex mtx_GL2;
static std::vector<GLfloat> yellowPoint = {(0.0,0.0,0.0)};
int main(int argc, char** argv)
{
    string READ_velocity = "";
    ifstream file("./Src/velocity.txt");
    /*file.is_open();*/
    if (file.peek() == EOF) {
         std::cout << "There are no velocity available " << endl;
        return 0;
    }
    if (getline(file, READ_velocity)) { // Read one line from the file
        std::cout << "velocity: " << READ_velocity << endl;
        velocity = std::stod(READ_velocity);

    }
    else {
        std::cout << "Failed to read velocity from the file." << endl;
        return 0;
    }
    file.close();
    Solider solider;
    std::thread solider_location([&solider] {
    solider.Thread_location();
    });
    Sensor sensor;
    std::thread sensorThread([&sensor, &solider]() {
        sensor.Shot_Detection_And_warning(solider);
        });

    std::thread Receiving_data_from_the_sensor([&solider, &sensor]() {
        Create_A_Thread_For_Each_Warning(solider, sensor);
        });

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(800, 600);
    glutCreateWindow("Simulation");

    glEnable(GL_DEPTH_TEST); // Enable depth testing for 3D rendering


    init();

    glutDisplayFunc(display);

    glutIdleFunc(idle);
    glutMainLoop();

    solider_location.join();
    sensorThread.join();
    Receiving_data_from_the_sensor.join();
        return 0;
    }


void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, 1.0, 1.0, 100.0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(0, 0, 5, 0, 0, 0, 0, 1, 0);
    drawGreenDot();
    drawRedDots();
    drawYellowDots();
    glutSwapBuffers();
}
void idle() {
    display(); // Request a redraw
}

void drawGreenDot() {
    glColor3f(0.0, 1.0, 0.0); // Green color
    glPointSize(5.0);
    glBegin(GL_POINTS);
    glVertex3f(0, 0, 0);
    glEnd();
}

void drawRedDots() {
    glColor3f(1.0, 0.0, 0.0); // Red color
    glPointSize(5.0);
   std::shared_lock<std::shared_mutex> lock(mtx_GL);
    for (const auto& position : redDotPositions) {
        glBegin(GL_POINTS);
        glVertex3f(position[0], position[1], position[2]);
        glEnd();
    }

}
void drawYellowDots() {
    glColor3f(1.0, 1.0, 0.0); // Yellow color (R, G, B values)
    glPointSize(5.0);
    
    if (yellowPoint[0] != 0) {
        glBegin(GL_POINTS);
        glVertex3f(yellowPoint[0], yellowPoint[1], yellowPoint[2]);
        glEnd();
    }


}
void init() {
    glClearColor(0.0, 0.0, 0.0, 1.0);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, 1.0, 1.0, 100.0); // Adjusted perspective settings
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}
double X_Map(double pointX, Solider& solider)
{
    double X=0.0;
    X = pointX - (solider.Get()[0]);
    X = X * 111000;//Convert the distance to meters
    X = X /250;
    return X;

}
double Y_Map(double pointY, Solider& solider)
{
    double Y=0.0;
    Y = pointY - (solider.Get()[1]);
    Y = Y * 111000;//Convert the distance to meters
    Y = Y / 250;
    return Y;
}

void Add_Value(std::vector<GLfloat> newRedDot)
{
    std::unique_lock<std::shared_mutex> lock(mtx_GL);
    redDotPositions.push_back(newRedDot);
}
void Remove_Value(std::vector<GLfloat> newRedDot) 
{
    std::unique_lock<std::shared_mutex> lock(mtx_GL);
    redDotPositions.erase(std::remove(redDotPositions.begin(), redDotPositions.end(), newRedDot), redDotPositions.end());
}

void Receiving_An_Alert_And_Calculating_Additional_Data(string Warning, Solider& solider)
{
    if (Warning == "") {
        cout << "Warning is null" << endl;
        return;
    }
    char delimiter = ',';
    vector<double> outputStrings = {};
    size_t pos = 0;
    string token;
    while ((pos = Warning.find(delimiter)) != string::npos) {
        token = Warning.substr(0, pos);
        outputStrings.push_back(stod(token));
        Warning.erase(0, pos + 1);
        cout << Warning<<endl;
    }
   
    double locationX = outputStrings[0];
    double locationY = outputStrings[1];
    std::vector<GLfloat> newRedDot = { static_cast<float>(X_Map(locationX,solider)), static_cast<float>(Y_Map(locationY,solider)), 0};
    Add_Value(newRedDot);
    double Direction = outputStrings[2];
    double distance = outputStrings[3];
    string radius = "";
    ifstream file("./Src/radius.txt");
    if (file.eof()) {
        std::cout << "There are no radius available " << endl;
        return;
    }
    if (getline(file, radius)) { // Read one line from the file
        std::cout << "radius: " << radius << endl;
    }
    else {
        std::cout << "Failed to read radius from the file." << endl;
        return;
    }
    file.close();

    random_device rd;
    mt19937 gen(rd());
    double max_range_detectionX = std::stod(radius);
    double min_range_detectionX = std::stod(radius) *-1;
    double max_range_detectionY = std::stod(radius);
    double min_range_detectionY = std::stod(radius)  *-1;
    uniform_real_distribution<double> disX(min_range_detectionX, max_range_detectionX);
    uniform_real_distribution<double> disY(min_range_detectionY, max_range_detectionY);

    double LocationX2 = (disX(gen) / 111000) + locationX;
    double LocationY2 = (disY(gen) / 111000) + locationY;
    std::vector<GLfloat> newRedDot2 = { static_cast<float>(X_Map(LocationX2,solider)), static_cast<float>(Y_Map(LocationY2,solider)), 0 };
    Add_Value(newRedDot2);

    if (if_in_the_firing_line(solider, locationX, locationY, LocationX2, LocationY2))
    {
        double Direction2 = bearing_with_altitude(solider.Get()[0], solider.Get()[1], LocationX2, LocationY2);
        double distance2 = haversine_distance(solider.Get()[0], solider.Get()[1], LocationX2, LocationY2);

        double shotDirection = Direction2 + 180.0;
        while (shotDirection >= 360.0) {
            shotDirection -= 360.0;
        }
      
        double initVelocityX = cos(shotDirection)* velocity; // Calculate initial velocity component in the x-direction
        cout << "initVelocityX" << initVelocityX;
        double initVelocityY = velocity * sin(shotDirection); // Calculate initial velocity component in the y-direction
        cout << "initVelocityY" << initVelocityY;
        double totalVelocity = sqrt((initVelocityX * initVelocityX) + (initVelocityY * initVelocityY));

        double timeOfFlight = distance2 / totalVelocity;
          
           string leaser = "";
           ifstream file("./Src/leaser.txt");
           if (file.peek() == EOF) {
               std::cout << "There are no leaser available " << endl;
               return ;
           }
           if (getline(file, leaser)) { // Read one line from the file
               std::cout << "leaser: " << leaser << endl;
           }
           else {
               std::cout << "Failed to read leaser from the file." << endl;
               return ;
           }
           file.close();
           double laserDistance = std::stod(leaser);
           double Slope= (LocationY2 - locationY) / (LocationX2 - locationX);
           std::vector<std::vector<GLfloat>> trajectoryResults;
           std::vector<GLfloat> interception_point = { 0.0,0.0,0.0 };
           std::vector<GLfloat> newRedDot;
           std::vector<double> bulletPosition = { 0.0,0.0 };
           double bulletPosition_direction = 0.0;
           double bulletPosition_distance = 0.0;
           double timeStep = 0.01;
           double t = laserDistance /velocity;
           double location1 = LocationX2;
           double location2 = LocationY2;
           if (distance2 < laserDistance) {
               interception_point = { static_cast<float>(X_Map(bulletPosition[0],solider)), static_cast<float>(Y_Map(bulletPosition[1],solider)), 0 };
               /*laserDistance = distance2;
               interception_point = Intercept_point_calculation(solider, laserDistance, Direction2);*/
           }
           else {
               for (double time = timeStep; time <= timeOfFlight; time += timeStep) {
                   bulletPosition = calculate_Bullet_Position(solider, location1, location2, velocity, t, distance2);
                   newRedDot = { static_cast<float>(X_Map(bulletPosition[0],solider)), static_cast<float>(Y_Map(bulletPosition[1],solider)), 0};
                   Add_Value(newRedDot);
                   trajectoryResults.push_back(newRedDot);
                   if (if_in_the_firing_line(solider, LocationX2, LocationY2, bulletPosition[0], bulletPosition[1])) {
                       bulletPosition_direction = bearing_with_altitude(solider.Get()[0], solider.Get()[1], bulletPosition[0], bulletPosition[1]);
                       bulletPosition_distance = haversine_distance(solider.Get()[0], solider.Get()[1], bulletPosition[0], bulletPosition[1]);
                       if (bulletPosition_distance < laserDistance)
                       {
                           interception_point = { static_cast<float>(X_Map(bulletPosition[0],solider)), static_cast<float>(Y_Map(bulletPosition[1],solider)), 0 };
                           break;
                       }
                       LocationX2 = bulletPosition[0];
                       LocationY2 = bulletPosition[1];
                       t += laserDistance / velocity;
                  }
                   else
                   {
                       break;
                   }

               }
           }
        while (yellowPoint[0] != 0) {}
        yellowPoint = interception_point;
        trajectoryResults.push_back(newRedDot);
        trajectoryResults.push_back(newRedDot2);
        std::this_thread::sleep_for(std::chrono::seconds(4));
        for (std::vector<std::vector<GLfloat>>::const_iterator i = trajectoryResults.begin(); i != trajectoryResults.end(); i++)
        {
            Remove_Value(*i);
        }
        std::this_thread::sleep_for(std::chrono::seconds(3));
        yellowPoint[0] = 0;
    }
    else 
    {
        std::cout << "The shot is not to the solider";
        std::this_thread::sleep_for(std::chrono::seconds(4));
        Remove_Value(newRedDot);
        Remove_Value(newRedDot2);
    }
}

std::vector<double> calculate_next_p(Solider &solider, double p1X, double p1Y, double k, double l)
{
    //נןסחה למציאת נקודה לפי חלוקת קטע ביחס נתון
    double xp = (solider.Get()[0] * l + p1X * k) / (k + l);
    double yp = (solider.Get()[1] * l + p1Y * k) / (k + l);
    vector<double> p = { xp, yp };
    return p;
}
// פונקציה לחישוב הנקודה המרחקת ביותר מהחייל
std::vector<GLfloat> farthestPoint(Solider &soldier, double shootingAngle,double distance2,double timeOfFlight,double totalVelocity) {

    string leaser = "";
    ifstream file("./Src/leaser.txt");
    if (file.peek() == EOF) {
        std::cout << "There are no leaser available " << endl;
        return { 0.0, 0.0,0.0 };
    }
    if (getline(file, leaser)) { // Read one line from the file
        std::cout << "leaser: " << leaser << endl;
    }
    else {
        std::cout << "Failed to read leaser from the file." << endl;
        return { 0.0, 0.0,0.0 };
    }
    file.close();

    std::vector<GLfloat> newYellowDot = { 0.0, 0.0,0.0 };

    double laserDistance = std::stod(leaser);
    if (distance2 < laserDistance) {
        laserDistance = distance2;
        newYellowDot[0] = soldier.Get()[0] + ((laserDistance * cos(shootingAngle))/111000);
        newYellowDot[1] = soldier.Get()[1] + ((laserDistance * sin(shootingAngle))/111000);
        newYellowDot = { static_cast<float>(X_Map(newYellowDot[0],soldier)), static_cast<float>(Y_Map(newYellowDot[1],soldier)), 0 };
        return newYellowDot;
    }

    // חישוב הנקודה המרחקת ביותר באמצעות משוואת פיתגורס
    double when = (distance2 - laserDistance) / totalVelocity;
    std::this_thread::sleep_for(std::chrono::duration<double>(when));
    newYellowDot[0] = soldier.Get()[0] +(( laserDistance * cos(shootingAngle))/111000);
    newYellowDot[1] = soldier.Get()[1] +(( laserDistance * sin(shootingAngle))/111000);
    newYellowDot = { static_cast<float>(X_Map(newYellowDot[0],soldier)), static_cast<float>(Y_Map(newYellowDot[1],soldier)), 0 };
    return newYellowDot;
}

bool if_in_the_firing_line(Solider& solider, double locationX, double locationY, double LocationX2, double LocationY2)
{
    //Distance between the first firing point and the soldier
    double dist12 = sqrt((locationX - solider.Get()[0]) * (locationX - solider.Get()[0]) + (locationY - solider.Get()[1]) * (locationY - solider.Get()[1]));
    //Distance between the first firing point and the second firing point
    double dist13 = sqrt((LocationX2 - solider.Get()[0]) * (LocationX2 - solider.Get()[0]) + (LocationY2 - solider.Get()[1]) * (LocationY2 - solider.Get()[1]));
    //Distance between the soldier and the second shooting point
    double dist23 = sqrt((LocationX2 - locationX) * (LocationX2 - locationX) + (LocationY2 - locationY) * (LocationY2 - locationY));
    //if in the firing line
    return abs(dist12 - (dist13 + dist23)) < 0.000001;
}

std::vector<double> calculate_Bullet_Position(Solider &solider,double x0, double y0, double initVelocityX, double time,double Distance)
{
    vector<double> Position = { 0.0,0.0 };
    double l = time * initVelocityX;
    double k = Distance - l;
    return calculate_next_p(solider,x0,y0,k,l);
}

// Function to calculate the distance between two points in 2 dimensions using Haversine formula in meters
static double haversine_distance(double lat1, double lon1, double lat2, double lon2)
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
static double bearing_with_altitude(double lat1, double lon1, double lat2, double lon2) {
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
void Create_A_Thread_For_Each_Warning(Solider & solider,Sensor & sensor)
{
    string line="";
    while (true)
    {
        line = sensor.Receiving_And_Warning_From_The_Sensor();
        if (line != "") {
            cout << line;
            warning = line; 
            thread WarningName([&solider]() {
                Receiving_An_Alert_And_Calculating_Additional_Data(warning, solider);
                });
            WarningName.detach();
        }
    }
}
