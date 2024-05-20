#include "Solider.h"
Solider::Solider()
{
    string API_key = "";
    ifstream file("./Src/API_key.txt");
    if (file.peek() == EOF) {
        std::cout << "There are no API key available " << endl;
        return ;
    }
    if (getline(file, API_key)) { // Read one line from the file
        std::cout << "API_key: " << API_key << endl;
        this->apiKey = API_key;
    }
    else {
        std::cout << "Failed to read API_key from the file." << endl;
        return ;
    }
    file.close();
}

Solider::Solider(double* location)
{
	this->location[0] = location[0];
	this->location[1] = location[1];
}
 double* Solider::Get()
{
	return location;
}

 void Solider::Update()
{
    std::string apiUrl = "https://www.googleapis.com/geolocation/v1/geolocate?key=" + apiKey;
    std::string response;

    CURL* curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, apiUrl.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &Solider::WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

        // Set the POST data to send to the API
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, "{}"); // Empty JSON object as POST data

        CURLcode res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);

        if (res != CURLE_OK) {
            std::cout<< "Error: Failed to perform cURL request"<<endl;
        }
    }
    else {
        std::cout<<"Error: Failed to initialize cURL"<<endl;
    }
    try {
    json data = json::parse(response);

    // Extract longitude and latitude from the JSON object
    this->location[0] = data["location"]["lat"];
    this->location[1] = data["location"]["lng"];
    std::cout << "x: " << std::fixed << std::setprecision(6) << this->location[0] << std::endl;
    std::cout << "y: " << std::fixed << std::setprecision(6) << this->location[1] << std::endl;
}
catch (json::parse_error& e) {
    std::cerr << "JSON parsing error: " << e.what() << std::endl;
}
 
}
 size_t Solider::WriteCallback(void* contents, size_t size, size_t nmemb, std::string* data) {
    data->append((char*)contents, size * nmemb);
    return size * nmemb;
}

void Solider::Thread_location()
{
    while (true)
    {
        Update();
        std::cout << "location thread is going to sleep for 10 seconds..." << std::endl;

        // Sleep for 10 seconds
        std::this_thread::sleep_for(std::chrono::seconds(10));

        // Display a message after sleeping
        std::cout << "location thread woke up after 10 seconds." << std::endl;
       
    }
}

