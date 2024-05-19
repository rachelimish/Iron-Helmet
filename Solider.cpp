#include "Solider.h"
Solider::Solider()
{
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
    //HANDLE pipe;
    //DWORD bytesWritten, bytesRead;
    //char buffer[4096];

    //// Open the named pipe for writing
    //pipe = CreateFile(TEXT("\\\\.\\pipe\\LocationPipe"), GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);

    //if (pipe == INVALID_HANDLE_VALUE) {
    //    std::cerr << "Failed to open named pipe for writing" << std::endl;
    //    return ;
    //}

    //// Send a request to the parent process to update the position
    //std::string request = "update_position";
    //WriteFile(pipe, request.c_str(), request.size() + 1, &bytesWritten, NULL);

    //// Close the pipe for writing
    //CloseHandle(pipe);

    //// Open the named pipe for reading
    //pipe = CreateFile(TEXT("\\\\.\\pipe\\LocationPipe"), GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);

    //if (pipe == INVALID_HANDLE_VALUE) {
    //    std::cerr << "Failed to open named pipe for reading" << std::endl;
    //    return ;
    //}

    //// Read the updated position from the parent process
    //ReadFile(pipe, buffer, sizeof(buffer), &bytesRead, NULL);
    //std::cout << "Received updated position: " << buffer << std::endl;

    //// Close the pipe for reading
    //CloseHandle(pipe);

}
 size_t Solider::WriteCallback(void* contents, size_t size, size_t nmemb, std::string* data) {
    data->append((char*)contents, size * nmemb);
    return size * nmemb;
}

void Solider::Thread_location()
{
    while (true)
    {
        std::cout << "location thread is going to sleep for 10 seconds..." << std::endl;

        // Sleep for 10 seconds
        std::this_thread::sleep_for(std::chrono::seconds(10));

        // Display a message after sleeping
        std::cout << "location thread woke up after 10 seconds." << std::endl;
        Update();
    }
}

