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
#include <gmp.h>
#include <Windows.h>
#include <winhttp.h>
#include <fstream>
#include <nlohmann/json.hpp>
#include <wininet.h>
#include <curl/curl.h>
#include <functional>
struct Point2D {
    double x,y;
};
const double g = 9.81;// acceleration due to gravity (m/s^2)
size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* data);
void ExtractLongitudeAndLatitude(const std::string& jsonResponse, double& longitude, double& latitude);
string GetLocationData(const std::string& apiKey);
void display();
void init();
void drawGreenDot(); 
void drawRedDots();
double solveIntersectionTime(std::vector<double> relativePos, std::vector<double> relativeVel);
static void Receiving_An_Alert_And_Calculating_Additional_Data(string alert, Solider& solider);
void Calculate_A_Projectile_Trajectory_In_2D(double x0, double y0, double angleX, double angleY, double timeStep, double totalTime);
void Intercept_Location_Calculation(double locationX, double locationY, double angleX, double angleY, Solider& solider);
void Create_A_Thread_For_Each_Warning(Solider& solider, Sensor& sensor);
//static double solider1[3] = { 37.7749,-122.4194, 15 };
static double velocity;
double X_Map(double pointX, Solider& solider);
double Y_Map(double pointY, Solider& solider);
std::vector<std::vector<GLfloat>> redDotPositions;
    // Add more positions as needed
std::mutex mtx_GL;
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

    std::thread Receiving_data_from_the_sensor([&sensor, &solider]() {
        Create_A_Thread_For_Each_Warning(solider, sensor);
        });
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(800, 600);
    glutCreateWindow("Simulation");

    glEnable(GL_DEPTH_TEST); // Enable depth testing for 3D rendering


    init();

    glutDisplayFunc(display);

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

    glutSwapBuffers();


    //glMatrixMode(GL_PROJECTION);
    //glLoadIdentity();
    //gluPerspective(45.0, 1.0, 1.0, 100.0); // Adjusted perspective settings
    //glMatrixMode(GL_MODELVIEW);
    //glLoadIdentity();
    //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);



    //glColor3f(1.0, 0.0, 0.0); // Red color
    //glPointSize(10.0);

    //glBegin(GL_POINTS);
    //glVertex3f(0, 0, 0);// Fixed position for the red dot
    //glEnd();

    //glutSwapBuffers();
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
    for (const auto& position : redDotPositions) {
        glBegin(GL_POINTS);
        glVertex3f(position[0], position[1], position[2]);
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
    double X;
    X = pointX - (solider.Get()[0]);
    X = X * 111000;//Convert the distance to meters
    X = X / 50;
    return X;

}
double Y_Map(double pointY, Solider& solider)
{
    double Y;
    Y = pointY - (solider.Get()[1]);
    Y = Y * 111000;//Convert the distance to meters
    Y = Y / 50;
    return Y;
}
//double Z_Hash(double pointZ, Solider& solider)
//{
//    double Y;
//    Y = pointZ - (solider.Get()[2]);
//    Y = Y / 50;
//    return Y;
//}
size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* data) {
    data->append((char*)contents, size * nmemb);
    return size * nmemb;
}

string GetLocationData(const std::string& apiKey) {
    //// Function to update the position
    //std::cout << "Updating position..." << std::endl;
    //// Add your position update logic here

    //// Send the updated position to the child process
    //HANDLE pipe;
    //DWORD bytesWritten;

    //// Open the named pipe for writing
    //pipe = CreateFile(TEXT("\\\\.\\pipe\\LocationPipe"), GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);

    //if (pipe == INVALID_HANDLE_VALUE) {
    //    std::cerr << "Failed to open named pipe for writing" << std::endl;
    //    return;
    //}

    // Send the updated position to the child process


    // Close the named pipe in the parent process

    std::string apiUrl = "https://www.googleapis.com/geolocation/v1/geolocate?key=" + apiKey;
    std::string response;

    CURL* curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, apiUrl.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

        // Set the POST data to send to the API
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, "{}"); // Empty JSON object as POST data

        CURLcode res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);

        if (res != CURLE_OK) {
            std::cout << "Error: Failed to perform cURL request" << endl;
        }
    }
    else {
        std::cout << "Error: Failed to initialize cURL" << endl;
    }
    return response;
}
void Calculate_A_Projectile_Trajectory_In_2D(double x0, double y0, double angleX, double angleY, double timeStep, double totalTime)
{
    sf::RenderWindow window(sf::VideoMode(800, 600), "Projectile Trajectory Visualization");
    double vx = velocity * cos(angleY) * cos(angleX);
    double vy = velocity * sin(angleY);

    for (double t = 0; t <= totalTime; t += timeStep) {
        double x = x0 + vx * t;
        double y = y0 + vy * t - 0.5 * g * t * t;

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

//void Calculate_A_Projectile_Trajectory_In_3D(double x0, double y0, double z0, double angleX, double angleY, double timeStep, double totalTime)
//{
//    sf::RenderWindow window(sf::VideoMode(800, 600), "Projectile Trajectory Visualization");
//    double vx = velocity * cos(angleY) * cos(angleX);
//    double vy = velocity * sin(angleY);
//    double vz = velocity * cos(angleY) * sin(angleX);
//
//    for (double t = 0; t <= totalTime; t += timeStep) {
//        double x = x0 + vx * t;
//        double y = y0 + vy * t - 0.5 * g * t * t;
//        double z = z0 + vz * t;
//        /*
//        sf::Event event;
//        while (window.pollEvent(event)) {
//            if (event.type == sf::Event::Closed) {
//                window.close();
//            }
//        }
//        window.clear();
//        sf::CircleShape shape(5);
//        shape.setFillColor(sf::Color::Red);
//        shape.setPosition(x, y);
//        window.draw(shape);
//        window.display();
//        */
//    }
//}
// 
void Intercept_Location_Calculation(double locationX, double locationY, double angleX, double angleY, Solider& solider)
{
    double projectile_vx = velocity * cos(angleY) * cos(angleX);
    double projectile_vy = velocity * sin(angleY);

    double angleY_proj = atan2(projectile_vy, projectile_vx);
    double angleX_proj = 0; // No Z dimension, set to 0

    double angleY_deg = angleY_proj * 180.0 / M_PI;
    double angleX_deg = angleX_proj * 180.0 / M_PI;

    double speed_of_light = 3e8; // Speed of light in m/s
    double Laser_beam_vx = speed_of_light * cos(angleY_deg * M_PI / 180) * cos(angleX_deg * M_PI / 180);
    double Laser_beam_vy = speed_of_light * sin(angleY_deg * M_PI / 180);

    // The relative speed
    double relative_speed[2] = { projectile_vx - Laser_beam_vx, projectile_vy - Laser_beam_vy };
    // The relative location
    double relative_location[2] = { locationX - solider.Get()[0], locationY - solider.Get()[1] };

    double interceptionTime; // The time required for the two objects to intersect
    Point2D adjustedInterceptionPoint = { 0, 0 };
    std::vector<double> location;
    location.assign(relative_location, relative_location + 2);
    std::vector<double> speed;
    speed.assign(relative_speed, relative_speed + 2);

    if ((interceptionTime = solveIntersectionTime(location, speed)) == -0.1)
        return;

    Point2D interceptPoint;
    interceptPoint.x = locationX + projectile_vx * interceptionTime;
    interceptPoint.y = locationY + projectile_vy * interceptionTime;

    // Function to adjust the interception point for laser beam speed
    double distanceToProjectile = sqrt(pow(interceptPoint.x - locationX, 2) + pow(interceptPoint.y - locationY, 2));
    // Calculate time taken for laser beam to travel the distance
    double laserBeamTime = distanceToProjectile / 299792458.0; // Speed of light in m/s

    adjustedInterceptionPoint.x = interceptPoint.x - projectile_vx * laserBeamTime;
    adjustedInterceptionPoint.y = interceptPoint.y - projectile_vy * laserBeamTime;

    std::cout<< adjustedInterceptionPoint.x<< adjustedInterceptionPoint.y<<endl;
}

//Point3D Intercept_Location_Calculation(double locationX, double  locationY, double locationZ, double angleX, double angleY)
//{
//    double projectile_vx = velocity * cos(angleY) * cos(angleX);
//    double projectile_vy = velocity * sin(angleY);
//    double projectile_vz = velocity * cos(angleY) * sin(angleX);
//
//    double angleY_proj = atan2(projectile_vy, sqrt(pow(projectile_vx, 2) + pow(projectile_vz, 2)));
//    double angleX_proj = atan2(projectile_vz, projectile_vx);
//
//    double angleY_deg = angleY_proj * 180.0 / M_PI;
//    double angleX_deg = angleX_proj * 180.0 / M_PI;
//
//    double speed_of_light = 3e8; // Speed of light in m/s
//    double Laser_beam_vx = speed_of_light * cos(angleY_deg * M_PI / 180) * cos(angleX_deg * M_PI / 180);
//    double Laser_beam_vy = speed_of_light * sin(angleY_deg * M_PI / 180);
//    double Laser_beam_vz = speed_of_light * cos(angleY_deg * M_PI / 180) * sin(angleX_deg * M_PI / 180);
//    double velocity_vector[3] = { Laser_beam_vx, Laser_beam_vy, Laser_beam_vz };
//
//    The relative speed
//    double relative_speed[3] = { projectile_vx - Laser_beam_vx ,projectile_vy - Laser_beam_vy ,projectile_vz - Laser_beam_vz };
//    The relative location
//    double relative_location[3] = { locationX - solider1[0],locationY - solider1[1],locationZ - solider1[2] };
//    double interceptionTime;//The time required for the two objects to intersect
//    Point3D adjustedInterceptionPoint = { 0,0,0 };
//    std::vector<double> location;
//    location.assign(relative_location, relative_location + 3);
//    std::vector<double> speed;
//    speed.assign(relative_speed, relative_speed + 3);
//    if ((interceptionTime = solveIntersectionTime(location, speed)) == -0.1)
//        return adjustedInterceptionPoint;
//
//    Point3D interceptionPoint;
//    interceptionPoint.x = locationX + projectile_vx * interceptionTime;
//    interceptionPoint.y = locationY + projectile_vy * interceptionTime;
//    interceptionPoint.z = locationZ + projectile_vz * interceptionTime;
//     Function to adjust the interception point for laser beam speed
//        double distanceToProjectile = sqrt(pow(interceptionPoint.x - locationX, 2) +
//            pow(interceptionPoint.y - locationY, 2) +
//            pow(interceptionPoint.z - locationZ, 2));
//         Calculate time taken for laser beam to travel the distance
//        double laserBeamTime = distanceToProjectile / 299792458.0; // Speed of light in m/s
//       
//        adjustedInterceptionPoint.x= interceptionPoint.x - projectile_vx * laserBeamTime;
//        adjustedInterceptionPoint.y= interceptionPoint.y - projectile_vy * laserBeamTime;
//        adjustedInterceptionPoint.z = interceptionPoint.z - projectile_vz * laserBeamTime;
//
//        return adjustedInterceptionPoint;
//
//}
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

void Receiving_An_Alert_And_Calculating_Additional_Data(string Warning, Solider& solider)
{
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
    mtx_GL.lock();
    redDotPositions.push_back(newRedDot);
    mtx_GL.unlock();
    double Direction = outputStrings[2];
    double shotDirection = Direction + 180.0;
    while (shotDirection >= 360.0) {
        shotDirection -= 360.0;
    }
    double distance = outputStrings[3];
    double directionRad = shotDirection * M_PI / 180.0; // Convert direction angle from degrees to radians

    double initVelocityX = velocity * cos(directionRad); // Calculate initial velocity component in the x-direction
    double initVelocityY = velocity * sin(directionRad); // Calculate initial velocity component in the y-direction

    double timeOfFlight = 2 * distance / initVelocityX; // Calculate the time of flight

    double launchAngleRad = atan((initVelocityY + sqrt(pow(initVelocityY, 2) + 2 * 9.81 * 0)) / initVelocityX); // Calculate launch angle in radians

    double launchAngleDeg = launchAngleRad * 180.0 / M_PI; // Convert launch angle from radians to degrees
    double angleX = std::acos(std::cos(launchAngleDeg * M_PI / 180.0) / std::sqrt(std::cos(launchAngleDeg * M_PI / 180.0) *
        std::cos(launchAngleDeg * M_PI / 180.0) + std::sin(launchAngleDeg * M_PI / 180.0) * std::sin(launchAngleDeg * M_PI / 180.0)));
    double angleY = std::asin(std::sin(launchAngleDeg * M_PI / 180.0) / std::sqrt(std::cos(launchAngleDeg * M_PI / 180.0) *
        std::cos(launchAngleDeg * M_PI / 180.0) + std::sin(launchAngleDeg * M_PI / 180.0) * std::sin(launchAngleDeg * M_PI / 180.0)));
    double totalTime = distance / velocity;
    // double speed = 30.0; // Initial speed of the projectile in m/s
     //double direction = 45.0; // Direction angle in degrees

     //double directionRad = shotDirection * M_PI / 180.0; // Convert direction angle from degrees to radians

     //double initVelocityX = velocity * cos(directionRad); // Calculate initial velocity component in the x-direction
     //double initVelocityY = velocity * sin(directionRad); // Calculate initial velocity component in the y-direction

     //double timeOfFlight = 2 * distance / initVelocityX; // Calculate the time of flight

     //double height = locationZ; // Assume projectile is launched from ground level

     //double launchAngleRad = atan((initVelocityY + sqrt(pow(initVelocityY, 2) + 2 * 9.81 * height)) / initVelocityX); // Calculate launch angle in radians

     //double launchAngleDeg = launchAngleRad * 180.0 / M_PI; // Convert launch angle from radians to degrees
     //double angleX = std::acos(std::cos(launchAngleDeg * M_PI / 180.0) / std::sqrt(std::cos(launchAngleDeg * M_PI / 180.0) * 
     //    std::cos(launchAngleDeg * M_PI / 180.0) + std::sin(launchAngleDeg * M_PI / 180.0) * std::sin(launchAngleDeg * M_PI / 180.0)));
     //double angleY = std::asin(std::sin(launchAngleDeg * M_PI / 180.0) / std::sqrt(std::cos(launchAngleDeg * M_PI / 180.0) * 
     //    std::cos(launchAngleDeg * M_PI / 180.0) + std::sin(launchAngleDeg * M_PI / 180.0) * std::sin(launchAngleDeg * M_PI / 180.0)));
     //double totalTime = distance / velocity;

    if ((solider.Get()[0] - locationX) * cos(angleY) != (solider.Get()[1] - locationY) * sin(angleY)) {
        // Soldier is not on the firing line in 2D, return default interception point
        return; }
        thread firing_range(Calculate_A_Projectile_Trajectory_In_2D, locationX, locationY, angleX, angleY, 0.001, totalTime);
        firing_range.detach();
        Intercept_Location_Calculation(locationX, locationY, angleX, angleY,solider);
   
}
void Create_A_Thread_For_Each_Warning(Solider & solider,Sensor & sensor)
{
    string line ="";
    while (true)
    {
        while ((line = sensor.Receiving_And_Warning_From_The_Sensor()) != "")
        {
            thread WarningName([&]() {
                Receiving_An_Alert_And_Calculating_Additional_Data(line, solider);
                });
            WarningName.detach();
        }
    }
}