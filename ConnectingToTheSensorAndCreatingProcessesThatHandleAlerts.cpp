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
typedef void (*FunctionPointer)(Solider& solider, double x0, double y0, double angleX, double angleY, double timeStep, double totalTime);
struct Point2D {
    double x,y;
};
const double g = 9.81;// acceleration due to gravity (m/s^2)
size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* data);
void ExtractLongitudeAndLatitude(const std::string& jsonResponse, double& longitude, double& latitude);
string GetLocationData(const std::string& apiKey);
void idle();
void display();
void init();
void drawGreenDot(); 
void drawRedDots();
void drawYellowDots();
void Remove_Value(std::vector<GLfloat> newRedDot);
void Add_Value(std::vector<GLfloat> newRedDot);
static double bearing_with_altitude(double lat1, double lon1, double lat2, double lon2);
static double haversine_distance(double lat1, double lon1, double lat2, double lon2);
double solveIntersectionTime(std::vector<double> relativePos, std::vector<double> relativeVel);
static void Receiving_An_Alert_And_Calculating_Additional_Data(string alert, Solider& solider);
void Firing_trajectory_calculation(bool display, Solider& solider, double locationX1, double locationY1, double angleX1, double angleY1, double timeStep, double totalTime1);
std::vector<std::vector<GLfloat>>  Calculate_A_Projectile_Trajectory_In_2D(Solider& solider, double x0, double y0, double initVelocityX, double initVelocityY, double timeStep, double totalTime);
void Deleting_a_firing_line(Solider& solider, double x0, double y0, double angleX, double angleY, double timeStep, double totalTime);
std::vector<GLfloat> Intercept_Location_Calculation(double locationX, double locationY, double angleX, double angleY, Solider& solider);
void Create_A_Thread_For_Each_Warning(Solider& solider, Sensor& sensor);
//static double solider1[3] = { 37.7749,-122.4194, 15 };
static double velocity;
double X_Map(double pointX, Solider& solider);
double Y_Map(double pointY, Solider& solider);
std::vector<std::vector<GLfloat>> redDotPositions;
std::vector<std::vector<GLfloat>> yellowDotPositions;
    // Add more positions as needed
std::shared_mutex mtx_GL;
static string warning;
//static double locationX1, locationY1, angleX1, angleY1, totalTime1;
std::mutex mtx_GL2;
constexpr double M16A1_MASS = 4.0; // in grams
constexpr double M16A1_DRAG_COEFFICIENT = 0.25; // drag coefficient
int main(int argc, char** argv)
{
    //sf::RenderWindow window(sf::VideoMode(800, 600), "SFML Window");

    //// Load the gunshot sound
    //sf::SoundBuffer gunshotBuffer;
    //if (!gunshotBuffer.loadFromFile("C:/Users/racheli/Desktop/פרויקטטט/ConnectingToTheSensorAndCreatingProcessesThatHandleAlerts/Src/shot.wav")) {
    //    std::cerr << "Failed to load sound file" << std::endl;
    //    return 1;
    //}
    //sf::Sound gunshotSound;
    //gunshotSound.setBuffer(gunshotBuffer);

    //// Define the position for the red dot
    //sf::Vector2f dotPosition(400, 300); // Set the position to the center of the window

    //// Main loop for the program
    //while (window.isOpen())
    //{
    //    // Check for window events
    //    sf::Event event;
    //    while (window.pollEvent(event))
    //    {
    //        if (event.type == sf::Event::Closed)
    //            window.close();
    //    }

    //    // Play the gunshot sound
    //    gunshotSound.play();

    //    // Display a red dot at the specified position
    //    sf::CircleShape dot(5);
    //    dot.setFillColor(sf::Color::Red);
    //    dot.setPosition(dotPosition);

    //    window.clear();
    //    window.draw(dot);
    //    window.display();
    //}
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
       /* mtx_GL.lock();*/
        glBegin(GL_POINTS);
        glVertex3f(position[0], position[1], position[2]);
        glEnd();
      /*  mtx_GL.unlock();*/
    }

}
void drawYellowDots() {
    glColor3f(1.0, 1.0, 0.0); // Yellow color (R, G, B values)
    glPointSize(5.0);
    
    for (const auto& position : yellowDotPositions) {
        mtx_GL2.lock();
        glBegin(GL_POINTS);
        glVertex3f(position[0], position[1], position[2]);
        glEnd();
        mtx_GL2.unlock();
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
std::vector<std::vector<GLfloat>>  Calculate_A_Projectile_Trajectory_In_2D(Solider& solider, double x0, double y0, double initVelocityX, double initVelocityY, double timeStep, double totalTime)
{
    std::vector<std::vector<GLfloat>> trajectory;
   /* double vx = (velocity * cos(angleY) * cos(angleX));
    double vy = (velocity * sin(angleY));*/
    std::vector<GLfloat> newRedDot;
    double t = timeStep;
     for(int i=0;i<=6;i++){
         // Calculate x and y positions at time t
         double xPos = x0 + initVelocityX * t;
         double yPos = y0 + initVelocityY * t - 0.5 * g * t * t;

         // Store the position (x, y) in the trajectory points
         newRedDot = { static_cast<float>(X_Map(xPos,solider)), static_cast<float>(Y_Map(yPos,solider)), 0 };

         Add_Value(newRedDot);
         trajectory.push_back(newRedDot);
         // Increment time
         t += timeStep;
  
    //double Vx = angleX * cos(angleY); // initial velocity in x-direction
    //double Vy = angleX * sin(angleY); // initial velocity in y-direction

    //for (double t = timeStep; t <= totalTime; t += timeStep) {
    //    double currentX = x0 + Vx * t;
    //    double currentY = y0 + Vy * t - 0.5 * g * t * t;

    //for (double t = 0; t <= 6; t ++) {
    //    double v = sqrt((vx * vx) + (vy * vy));
    //    double drag = 0.5 * M16A1_DRAG_COEFFICIENT * v * v / M16A1_MASS;
    //    double ax = -((drag * vx) / v);
    //    double ay = -(g + (drag * vy) / v);

    //    vx += ax * timeStep;
    //    vy += ay * timeStep;

    //    double x = x0 + (vx * t);
    //    double y = y0 + (vy * t) - 0.5 * g * t * t;
    /*double vx = velocity * cos(angleY) * cos(angleX);
    double vy = velocity * sin(angleY);
    std::vector<GLfloat> newRedDot;
    for (double t = timeStep; t <= totalTime; t += timeStep) {
        double v = sqrt(vx * vx + vy * vy);
        double drag = 0.5 * M16A1_DRAG_COEFFICIENT * v * v / M16A1_MASS;
        double ax = -((drag * vx) / v);
        double ay = -(g + (drag * vy) / v);

        vx += ax * timeStep;
        vy += ay * timeStep;

        double x = x0 + vx * t;
        double y = y0 + vy * t - 0.5 * g * t * t;*/
    /*std::vector<std::vector<GLfloat>> trajectory;
    double vx = velocity * cos(angleY) * cos(angleX);
    double vy = velocity * sin(angleY);
    std::vector<GLfloat> newRedDot;
    double ax = 0.0;
    double ay = 0.0;
    for (double t = 0; t <= totalTime; t += timeStep) {
        double v = sqrt(vx * vx + vy * vy);
        double drag = 0.5 * M16A1_DRAG_COEFFICIENT * v * v / M16A1_MASS;
        double ax = -((drag * vx) / v);
        double ay = -(g + (drag * vy) / v);

        vx += ax * timeStep;
        vy += ay * timeStep;

        double x = x0 + vx * t;
        double y = y0 + vy * t - 0.5 * g * t * t;*/

        // Use the corrected x and y values for further calculations or visualization


    }
    return trajectory;
    //std::vector<std::vector<GLfloat>> Route;
    //double vx = velocity * cos(angleY) * cos(angleX);
    //double vy = velocity * sin(angleY);

    //double ax, ay;

    //for (double t = timeStep; t <= totalTime; t += timeStep) {
    //    double v = sqrt(vx * vx + vy * vy);
    //    double drag = 0.5 * M16A1_DRAG_COEFFICIENT * v * v / M16A1_MASS;
    //    ax = -drag * vx / v;
    //    ay = -g - drag * vy / v;

    //    vx = vx + ax * timeStep;
    //    vy = vy + ay * timeStep;

    //    double x = x0 + vx * t;
    //    double y = y0 + vy * t - 0.5 * g * t * t;
    //    std::vector<GLfloat> newRedDot = { static_cast<float>(X_Map(x,solider)), static_cast<float>(Y_Map(y,solider)), 0 };
    //    mtx_GL.lock();
    //    redDotPositions.push_back(newRedDot);
    //    mtx_GL.unlock();
    //    Route.push_back(newRedDot);
    //}
    //return Route;
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
std::vector<GLfloat> Intercept_Location_Calculation(double locationX, double locationY, double angleX, double angleY, Solider& solider)
{
    std::vector<GLfloat> newYellowDot = { 0.0, 0.0,0.0 };
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
        return newYellowDot;

    Point2D interceptPoint;
    interceptPoint.x = locationX + projectile_vx * interceptionTime;
    interceptPoint.y = locationY + projectile_vy * interceptionTime;

    // Function to adjust the interception point for laser beam speed
    double distanceToProjectile = sqrt(pow(interceptPoint.x - locationX, 2) + pow(interceptPoint.y - locationY, 2));
    // Calculate time taken for laser beam to travel the distance
    double laserBeamTime = distanceToProjectile / 299792458.0; // Speed of light in m/s

    adjustedInterceptionPoint.x = interceptPoint.x - projectile_vx * laserBeamTime;
    adjustedInterceptionPoint.y = interceptPoint.y - projectile_vy * laserBeamTime;

    newYellowDot = { static_cast<float>(X_Map(adjustedInterceptionPoint.x,solider)), static_cast<float>(Y_Map(adjustedInterceptionPoint.y,solider)), 0 };
    mtx_GL2.lock();
    yellowDotPositions.push_back(newYellowDot);
    mtx_GL2.unlock();
    std::cout<< adjustedInterceptionPoint.x<<","<< adjustedInterceptionPoint.y << endl;
    return newYellowDot;
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
    /*mtx_GL.lock();
    redDotPositions.push_back(newRedDot);
    mtx_GL.unlock();*/
    double Direction = outputStrings[2];
    double distance = outputStrings[3];

    string radius = "";
    ifstream file("./Src/radius.txt");
    if (file.peek() == EOF) {
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
    /* mtx_GL.lock();
    redDotPositions.push_back(newRedDot2);
    mtx_GL.unlock();*/
    double dist12 = sqrt((locationX - solider.Get()[0]) * (locationX - solider.Get()[0]) + (locationY - solider.Get()[1]) * (locationY - solider.Get()[1]));
    double dist13 = sqrt((LocationX2 - solider.Get()[0]) * (LocationX2 - solider.Get()[0]) + (LocationY2 - solider.Get()[1]) * (LocationY2 - solider.Get()[1]));
    double dist23 = sqrt((LocationX2 - locationX) * (LocationX2 - locationX) + (LocationY2 - locationY) * (LocationY2 - locationY));
    //if in the firing line
    bool if_in_the_firing_line = abs(dist12 - (dist13 + dist23)) < 0.000001;
   
    //double totalVelocity = sqrt(initVelocityX * initVelocityX + initVelocityY * initVelocityY);

    //double timeOfFlight = distance2 / totalVelocity;

    //double horizontal_distance = sqrt(pow(LocationY2 - solider.Get()[1], 2) + pow(LocationX2 - solider.Get()[0], 2));
    //double firingAngle = atan((pow(totalVelocity, 2) - sqrt(pow(totalVelocity, 4) - 2 * g * (g * pow(horizontal_distance, 2)))) / (g * horizontal_distance));

    //double launchAngleDeg = firingAngle * 180.0 / M_PI; // Convert launch angle from radians to degrees
    //double angleX = atan(initVelocityY / initVelocityX); // Calculate launch angle in the x-direction in radians
    //double angleY = atan(sqrt(initVelocityX * initVelocityX + initVelocityY * initVelocityY) / g); // Calculate launch angle in the y-direction in radians

    //double Direction2 = bearing_with_altitude(solider.Get()[0], solider.Get()[1], LocationX2, LocationY2);
    //double distance2 = haversine_distance(solider.Get()[0], solider.Get()[1], LocationX2, LocationY2);

    //double shotDirection = Direction2 + 180.0;
    //while (shotDirection >= 360.0) {
    //    shotDirection -= 360.0;
    //}
    //double directionRad = shotDirection * M_PI / 180.0; // Convert direction angle from degrees to radians

    //double initVelocityX = velocity * cos(directionRad); // Calculate initial velocity component in the x-direction
    //double initVelocityY = velocity * sin(directionRad); // Calculate initial velocity component in the y-direction

    ////double timeOfFlight = 2 * distance2 / initVelocityX; // Calculate the time of flight
    //double totalVelocity = sqrt(pow(initVelocityX, 2) + pow(initVelocityY, 2));

    //// Calculate the time of flight based on the total velocity magnitude
    //double timeOfFlight = distance / totalVelocity;
    //double horizontal_distance = sqrt(pow(solider.Get()[1] - solider.Get()[0], 2) + pow(LocationY2 - LocationX2, 2));
    //// double launchAngleRad = atan((initVelocityY + sqrt(pow(initVelocityY, 2) + 2 * 9.81 * 0)) / initVelocityX); // Calculate launch angle in radians
    //double firingAngle = atan((pow(totalVelocity, 2) - sqrt(pow(totalVelocity, 4) - g * (g * pow(horizontal_distance, 2)))) / (g * horizontal_distance));

    //double launchAngleDeg = firingAngle * 180.0 / M_PI; // Convert launch angle from radians to degrees
    //double angleX = std::acos(std::cos(launchAngleDeg * M_PI / 180.0) / std::sqrt(std::cos(launchAngleDeg * M_PI / 180.0) *
    //    std::cos(launchAngleDeg * M_PI / 180.0) + std::sin(launchAngleDeg * M_PI / 180.0) * std::sin(launchAngleDeg * M_PI / 180.0)));
    //double angleY = std::asin(std::sin(launchAngleDeg * M_PI / 180.0) / std::sqrt(std::cos(launchAngleDeg * M_PI / 180.0) *
    //    std::cos(launchAngleDeg * M_PI / 180.0) + std::sin(launchAngleDeg * M_PI / 180.0) * std::sin(launchAngleDeg * M_PI / 180.0)));
    /*double totalTime = distance2 / velocity;*/
    if (if_in_the_firing_line)
    {


        double Direction2 = bearing_with_altitude(solider.Get()[0], solider.Get()[1], LocationX2, LocationY2);
        double distance2 = haversine_distance(solider.Get()[0], solider.Get()[1], LocationX2, LocationY2);

        double shotDirection = Direction2 + 180.0;
        while (shotDirection >= 360.0) {
            shotDirection -= 360.0;
        }
        double directionRad = shotDirection * M_PI / 180.0; // Convert direction angle from degrees to radians

        double initVelocityX = velocity * cos(directionRad); // Calculate initial velocity component in the x-direction
        double initVelocityY = velocity * sin(directionRad); // Calculate initial velocity component in the y-direction

        double totalVelocity = sqrt(initVelocityX * initVelocityX + initVelocityY * initVelocityY);

        double timeOfFlight = distance2 / totalVelocity;

        double horizontal_distance = sqrt(pow(LocationY2 - solider.Get()[1], 2) + pow(LocationX2 - solider.Get()[0], 2));
        double firingAngle = atan((pow(totalVelocity, 2) - sqrt(pow(totalVelocity, 4) - 2 * g * (g * pow(horizontal_distance, 2)))) / (g * horizontal_distance));

        double launchAngleDeg = firingAngle * 180.0 / M_PI; // Convert launch angle from radians to degrees
        double angleX = atan(initVelocityY / initVelocityX); // Calculate launch angle in the x-direction in radians
        double angleY = atan((initVelocityY + sqrt(initVelocityY * initVelocityY + 2 * g * LocationY2)) / initVelocityX); // Calculate launch angle in the y-direction in radians
     
        double tt = 0.01;
        double t = 0.000001;
        while(tt<= timeOfFlight){
            // Calculate x and y positions at time t
            double xPos = LocationX2 + initVelocityX * t;
            double yPos = LocationY2 + initVelocityY * t - 0.5 * g * t * t;

            // Store the position (x, y) in the trajectory points
            newRedDot = { static_cast<float>(X_Map(xPos,solider)), static_cast<float>(Y_Map(yPos,solider)), 0 };

            Add_Value(newRedDot);
            // Increment time
            t += 0.000001;
            tt += 0.01;
        }
         std::vector<std::vector<GLfloat>> trajectoryResults;
        thread firing_range([&solider, LocationX2, LocationY2, initVelocityX, initVelocityY, timeOfFlight, &trajectoryResults]() {
            trajectoryResults = Calculate_A_Projectile_Trajectory_In_2D(solider, LocationX2, LocationY2, initVelocityX, initVelocityY, 0.001, timeOfFlight);
            });
       /* std::vector<GLfloat> interception_point;
        interception_point=Intercept_Location_Calculation(locationX, locationY, angleX, angleY, solider);*/
        firing_range.join();
        /*trajectoryResults.push_back(newRedDot);
        for (const auto& trajectory : trajectoryResults) {
            mtx_GL.lock();
            redDotPositions.erase(std::remove(redDotPositions.begin(), redDotPositions.end(), trajectory), redDotPositions.end());
            mtx_GL.unlock();
        }
        if (interception_point[0] != 0.0 && interception_point[1] != 0.0) 
        {
            mtx_GL2.lock();
            yellowDotPositions.erase(std::remove(redDotPositions.begin(), redDotPositions.end(), interception_point), redDotPositions.end());
            mtx_GL2.unlock();
        }*/
        
    }
    else 
    {
        std::cout << "The shot is not to the solider";
        std::this_thread::sleep_for(std::chrono::seconds(4));
        Remove_Value(newRedDot);
        Remove_Value(newRedDot2);
        /* mtx_GL.lock();
        redDotPositions.erase(std::remove(redDotPositions.begin(), redDotPositions.end(), newRedDot), redDotPositions.end());
        redDotPositions.erase(std::remove(redDotPositions.begin(), redDotPositions.end(), newRedDot2), redDotPositions.end());
        mtx_GL.unlock();*/
    }
}
//void Firing_trajectory_calculation(bool display,Solider &solider, double locationX1, double locationY1, double angleX1, double angleY1, double timeStep, double totalTime1)
//{
//    std::map<bool, FunctionPointer> functionMap = {
//        {true, Calculate_A_Projectile_Trajectory_In_2D},
//        {false, Deleting_a_firing_line}
//   };
//    functionMap[display](solider, locationX1, locationY1, angleX1, angleY1, timeStep, totalTime1);
//}
void Deleting_a_firing_line(Solider& solider, double x0, double y0, double angleX, double angleY, double timeStep, double totalTime)
{
    double vx = velocity * cos(angleY) * cos(angleX);
    double vy = velocity * sin(angleY);

    for (double t = 0; t <= totalTime; t += timeStep) {
        double x = x0 + vx * t;
        double y = y0 + vy * t - 0.5 * g * t * t;
        std::vector<GLfloat> newRedDot = { static_cast<float>(X_Map(x,solider)), static_cast<float>(Y_Map(y,solider)), 0 };
        mtx_GL.lock();
        redDotPositions.erase(std::remove(redDotPositions.begin(), redDotPositions.end(), newRedDot), redDotPositions.end());
        mtx_GL.unlock();
    }
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