#include <SFML/Graphics.hpp>
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

struct Point3D {
    double x,y,z;
};
const double g = 9.81;// acceleration due to gravity (m/s^2)
const double velocity = 975;
void display();
void init();
double solveIntersectionTime(std::vector<double> relativePos, std::vector<double> relativeVel);
void Receiving_An_Alert_And_Calculating_Additional_Data(string alert);
void Calculate_A_Projectile_Trajectory_In_3D(double x0, double y0, double z0, double angleX, double angleY, double timeStep, double totalTime);
Point3D Intercept_Location_Calculation(double locationX, double  locationY, double locationZ, double angleX, double angleY);
void Create_A_Thread_For_Each_Warning();
static double solider1[3] = { 37.7749,-122.4194, 15 };
static Solider solider(solider1);
int main(int argc, char** argv)
{

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutCreateWindow("3D Red Dot");

    glEnable(GL_DEPTH_TEST);

    glutDisplayFunc(display);
    init();

    glutMainLoop();
   


    Sensor sensor;
    Solider mySolider(solider);
    std::thread sensorThread([&sensor, &mySolider]() {
        sensor.Shot_Detection_And_warning(mySolider);
        });
    sensorThread.join();

    // Ensure the thread is joinable so it can run
    //if (warningThrea.joinable()) {
    //    warningThrea.join();
    //}
    /*sf::RenderWindow window(sf::VideoMode(800, 600), "Projectile Trajectory Visualization");*/
    std::thread Receiving_data_from_the_sensor(Create_A_Thread_For_Each_Warning);
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

    glColor3f(1.0, 0.0, 0.0); // Red color
    glPointSize(10.0);

    glBegin(GL_POINTS);
    glVertex3f(0, 0, 2);// Fixed position for the red dot
    glEnd();

    glutSwapBuffers();
}

void init() {
    glClearColor(0.0, 0.0, 0.0, 1.0);
}

void Calculate_A_Projectile_Trajectory_In_3D(double x0, double y0, double z0, double angleX, double angleY, double timeStep, double totalTime)
{
    sf::RenderWindow window(sf::VideoMode(800, 600), "Projectile Trajectory Visualization");
    double vx = velocity * cos(angleY) * cos(angleX);
    double vy = velocity * sin(angleY);
    double vz = velocity * cos(angleY) * sin(angleX);

    for (double t = 0; t <= totalTime; t += timeStep) {
        double x = x0 + vx * t;
        double y = y0 + vy * t - 0.5 * g * t * t;
        double z = z0 + vz * t;
        /*
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }
        }

        window.clear();
        sf::CircleShape shape(5);
        shape.setFillColor(sf::Color::Red);
        shape.setPosition(x, y);
        window.draw(shape);
        window.display();
        */

    }
}
Point3D Intercept_Location_Calculation(double locationX, double  locationY, double locationZ, double angleX, double angleY)
{
    double projectile_vx = velocity * cos(angleY) * cos(angleX);
    double projectile_vy = velocity * sin(angleY);
    double projectile_vz = velocity * cos(angleY) * sin(angleX);

    double angleY_proj = atan2(projectile_vy, sqrt(pow(projectile_vx, 2) + pow(projectile_vz, 2)));
    double angleX_proj = atan2(projectile_vz, projectile_vx);

    double angleY_deg = angleY_proj * 180.0 / M_PI;
    double angleX_deg = angleX_proj * 180.0 / M_PI;

    double speed_of_light = 3e8; // Speed of light in m/s
    double Laser_beam_vx = speed_of_light * cos(angleY_deg * M_PI / 180) * cos(angleX_deg * M_PI / 180);
    double Laser_beam_vy = speed_of_light * sin(angleY_deg * M_PI / 180);
    double Laser_beam_vz = speed_of_light * cos(angleY_deg * M_PI / 180) * sin(angleX_deg * M_PI / 180);
    double velocity_vector[3] = { Laser_beam_vx, Laser_beam_vy, Laser_beam_vz };

    //The relative speed
    double relative_speed[3] = { projectile_vx - Laser_beam_vx ,projectile_vy - Laser_beam_vy ,projectile_vz - Laser_beam_vz };
    //The relative location
    double relative_location[3] = { locationX - solider1[0],locationY - solider1[1],locationZ - solider1[2] };
    double interceptionTime;//The time required for the two objects to intersect
    Point3D adjustedInterceptionPoint = { 0,0,0 };
    std::vector<double> location;
    location.assign(relative_location, relative_location + 3);
    std::vector<double> speed;
    speed.assign(relative_speed, relative_speed + 3);
    if ((interceptionTime = solveIntersectionTime(location, speed)) == -0.1)
        return adjustedInterceptionPoint;

    Point3D interceptionPoint;
    interceptionPoint.x = locationX + projectile_vx * interceptionTime;
    interceptionPoint.y = locationY + projectile_vy * interceptionTime;
    interceptionPoint.z = locationZ + projectile_vz * interceptionTime;
    // Function to adjust the interception point for laser beam speed
        double distanceToProjectile = sqrt(pow(interceptionPoint.x - locationX, 2) +
            pow(interceptionPoint.y - locationY, 2) +
            pow(interceptionPoint.z - locationZ, 2));
        // Calculate time taken for laser beam to travel the distance
        double laserBeamTime = distanceToProjectile / 299792458.0; // Speed of light in m/s
       
        adjustedInterceptionPoint.x= interceptionPoint.x - projectile_vx * laserBeamTime;
        adjustedInterceptionPoint.y= interceptionPoint.y - projectile_vy * laserBeamTime;
        adjustedInterceptionPoint.z = interceptionPoint.z - projectile_vz * laserBeamTime;

        return adjustedInterceptionPoint;

}
double solveIntersectionTime(std::vector<double> relativePos, std::vector<double> relativeVel) {
    // Assuming equations of motion: relativePos[i] = relativeVel[i] * t for each component i
    double time = -1.0; // Initialize time to a negative value

    // Check for division by zero and solve for time
    for (int i = 0; i < 3; ++i) {
        if (relativeVel[i] != 0.0) {
            double t = relativePos[i] / relativeVel[i];
            if (time == -1.0 || t == time) {
                time = t;
            }
            else {
                // Inconsistent intersection times, return a negative value
                return -1.0;
            }
        }
    }

    return time;
}

void Receiving_An_Alert_And_Calculating_Additional_Data(string Warning)
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
    double locationX = outputStrings[0];
    double locationY = outputStrings[1];
    double locationZ = outputStrings[2];
    double Direction= outputStrings[3];
    double shotDirection = Direction + 180.0;
    while (shotDirection >= 360.0) {
        shotDirection -= 360.0;
    }
    double distance = outputStrings[4];
   // double speed = 30.0; // Initial speed of the projectile in m/s
    //double direction = 45.0; // Direction angle in degrees

    double directionRad = shotDirection * M_PI / 180.0; // Convert direction angle from degrees to radians

    double initVelocityX = velocity * cos(directionRad); // Calculate initial velocity component in the x-direction
    double initVelocityY = velocity * sin(directionRad); // Calculate initial velocity component in the y-direction

    double timeOfFlight = 2 * distance / initVelocityX; // Calculate the time of flight

    double height = locationZ; // Assume projectile is launched from ground level

    double launchAngleRad = atan((initVelocityY + sqrt(pow(initVelocityY, 2) + 2 * 9.81 * height)) / initVelocityX); // Calculate launch angle in radians

    double launchAngleDeg = launchAngleRad * 180.0 / M_PI; // Convert launch angle from radians to degrees
    double angleX = std::acos(std::cos(launchAngleDeg * M_PI / 180.0) / std::sqrt(std::cos(launchAngleDeg * M_PI / 180.0) * std::cos(launchAngleDeg * M_PI / 180.0) + std::sin(launchAngleDeg * M_PI / 180.0) * std::sin(launchAngleDeg * M_PI / 180.0)));
    double angleY = std::asin(std::sin(launchAngleDeg * M_PI / 180.0) / std::sqrt(std::cos(launchAngleDeg * M_PI / 180.0) * std::cos(launchAngleDeg * M_PI / 180.0) + std::sin(launchAngleDeg * M_PI / 180.0) * std::sin(launchAngleDeg * M_PI / 180.0)));
    double totalTime = distance / velocity;


    Calculate_A_Projectile_Trajectory_In_3D(locationX, locationY, locationZ, angleX, angleY,0.01, totalTime);

}
void Create_A_Thread_For_Each_Warning()
{
    Sensor sensor;
    string line = "";
    //string WarningName;
    while (true)
    {
        while ((line = sensor.Receiving_And_Warning_From_The_Sensor()) != "")
        {
            // WarningName = "Warning" +to_string(mone);
            thread WarningName(Receiving_An_Alert_And_Calculating_Additional_Data, line);
            WarningName.join();
            //mone++;
        }
    }
}