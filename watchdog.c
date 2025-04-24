#include <iostream>
#include <string>
#include <curl/curl.h>
#include <chrono>
#include <thread>
#include <cstdlib>
#include <fstream>
#include <nlohmann/json.hpp>

// Callback function to write the response data to a string
size_t write_callback(void* contents, size_t size, size_t nmemb, void* userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

// Function to fetch geolocation data from ipwhois.io
std::string get_geolocation(const std::string& ip) {
    CURL* curl;
    CURLcode res;
    std::string readBuffer;

    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();
    if (curl) {
        // Construct the API request URL
        std::string url = "http://ipwhois.app/json/" + ip;

        // Set the URL and other curl options
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

        // Perform the request
        res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            std::cerr << "cURL error: " << curl_easy_strerror(res) << std::endl;
        }

        // Clean up after the request
        curl_easy_cleanup(curl);
    }
    curl_global_cleanup();

    return readBuffer;
}

// Function to start screen recording using FFmpeg
void start_screen_recording(const std::string& output_file) {
    // FFmpeg command for screen recording (Linux/Unix-like)
    std::string command = "ffmpeg -f x11grab -s 1920x1080 -i :0.0 -vcodec libx264 -preset ultrafast -y " + output_file;
    
    // For Windows, you would use: "ffmpeg -f gdigrab -i desktop -vcodec libx264 -preset ultrafast -y " + output_file
    std::cout << "Starting screen recording..." << std::endl;
    std::system(command.c_str());
}

// Function to stop screen recording by killing the FFmpeg process (if running)
void stop_screen_recording() {
    // Killing FFmpeg process (on Unix-like systems, e.g., Linux, macOS)
    std::cout << "Stopping screen recording..." << std::endl;
    std::system("pkill ffmpeg");
}

// Function to log information to a file
void log_to_file(const std::string& message) {
    std::ofstream log_file("scammer_log.txt", std::ios_base::app);
    if (log_file.is_open()) {
        log_file << message << std::endl;
        log_file.close();
    } else {
        std::cerr << "Error opening log file!" << std::endl;
    }
}

// Function to parse JSON and extract country and city
void log_geolocation_data(const std::string& json_data) {
    try {
        // Parse the JSON response
        auto data = nlohmann::json::parse(json_data);

        // Extract country and city
        std::string country = data["country"].get<std::string>();
        std::string city = data["city"].get<std::string>();

        // Create the log message
        std::string log_message = "Geolocation data: City = " + city + ", Country = " + country;

        // Log the message
        log_to_file(log_message);
        std::cout << log_message << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "Error parsing geolocation data: " << e.what() << std::endl;
    }
}

int main() {
    // Example IPs for testing
    std::string ips[] = {"8.8.8.8", "1.1.1.1", "8.8.4.4"};

    // Log the start of the screen recording session
    log_to_file("Starting screen recording session...");

    // Start recording screen in a file
    std::string output_file = "scammer_screen_recording.mp4";
    start_screen_recording(output_file);

    // Loop through the IPs and query the API
    for (const auto& ip : ips) {
        std::string result = get_geolocation(ip);

        // Log only the country and city
        log_geolocation_data(result);

        // Wait for 1 minute (60 seconds) between requests to prevent over-querying
        std::this_thread::sleep_for(std::chrono::minutes(1));  // 1 request per minute
    }

    // Log the stop of the screen recording session
    log_to_file("Stopping screen recording session...");

    // Stop the screen recording after a few minutes (you can customize this based on your needs)
    std::this_thread::sleep_for(std::chrono::minutes(5));  // Record for 5 minutes as an example
    stop_screen_recording();

    return 0;
}
