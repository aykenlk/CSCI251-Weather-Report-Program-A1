#include "asg1.h"

int main() {
    ConfigData config = {0, 0, 0, 0, "", "", ""};
    WeatherData weather = {nullptr, nullptr, nullptr, nullptr, 0, 0};
    
    int choice;
    do {
        displayMainMenu();
        if (!(cin >> choice)) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Invalid input. Please enter a number.\n";
            continue;
        }
        cin.ignore();
        
        processMenuChoice(choice, config, weather);
        
    } while (choice != 8);
    
    cleanupWeatherData(weather, config);
    return 0;
}

// Helper function to check if a file exists
bool fileExists(const string& filename) {
    ifstream file(filename);
    return file.good();
}

// Helper function to extract filename from config line
string extractFilename(const string& line) {
    // Look for common patterns
    size_t equalPos = line.find('=');
    if (equalPos != string::npos) {
        string filename = line.substr(equalPos + 1);
        // Trim whitespace
        filename.erase(0, filename.find_first_not_of(" \t"));
        filename.erase(filename.find_last_not_of(" \t") + 1);
        return filename;
    }
    
    // Look for .txt extension
    size_t txtPos = line.find(".txt");
    if (txtPos != string::npos) {
        // Find the start of the filename (after last space or =)
        size_t start = line.find_last_of(" =");
        if (start == string::npos) start = 0;
        else start++;
        return line.substr(start, txtPos + 4 - start);
    }
    
    return "";
}

// Helper function to extract value after a key (case insensitive)
string extractValueAfterKey(const string& line, const string& key) {
    // Convert both to lowercase for case-insensitive comparison
    string lineLower = line;
    string keyLower = key;
    transform(lineLower.begin(), lineLower.end(), lineLower.begin(), ::tolower);
    transform(keyLower.begin(), keyLower.end(), keyLower.begin(), ::tolower);
    
    size_t keyPos = lineLower.find(keyLower);
    if (keyPos == string::npos) return "";
    
    // Find the value after the key
    size_t valueStart = keyPos + keyLower.length();
    
    // Skip any delimiters (= : space)
    while (valueStart < line.length() && 
           (line[valueStart] == '=' || line[valueStart] == ':' || 
            line[valueStart] == ' ' || line[valueStart] == '\t')) {
        valueStart++;
    }
    
    // Extract until end of line or comment
    size_t valueEnd = line.find("//", valueStart);
    if (valueEnd == string::npos) valueEnd = line.length();
    
    string value = line.substr(valueStart, valueEnd - valueStart);
    
    // Trim whitespace
    value.erase(0, value.find_first_not_of(" \t"));
    value.erase(value.find_last_not_of(" \t") + 1);
    
    return value;
}

//ensure that the coordinates fall within the grid
bool validateGridCoordinates(int x, int y, const ConfigData& config) {
    return (x >= config.gridX_min && x <= config.gridX_max &&
            y >= config.gridY_min && y <= config.gridY_max);
}

void initializeCityMap(WeatherData& weather, const ConfigData& config) {
    if (config.gridX_min > config.gridX_max || config.gridY_min > config.gridY_max) {
        cerr << "Error: Invalid grid range in initialization\n";
        return;
    }

    int rows = config.gridY_max - config.gridY_min + 1;
    int cols = config.gridX_max - config.gridX_min + 1;
    
    weather.cityMap = new char*[rows];
    for (int y = 0; y < rows; y++) {
        weather.cityMap[y] = new char[cols];
        for (int x = 0; x < cols; x++) {
            weather.cityMap[y][x] = ' ';
        }
    }
    
    weather.cloudCover = new int*[rows];
    for (int y = 0; y < rows; y++) {
        weather.cloudCover[y] = new int[cols];
        for (int x = 0; x < cols; x++) {
            weather.cloudCover[y][x] = 0;
        }
    }
    
    weather.pressure = new int*[rows];
    for (int y = 0; y < rows; y++) {
        weather.pressure[y] = new int[cols];
        for (int x = 0; x < cols; x++) {
            weather.pressure[y][x] = 0;
        }
    }
    
    weather.cityCapacity = 10;
    weather.cityCount = 0;
    weather.cities = new CityData[weather.cityCapacity];
}

//delete all stored data after program stops
void cleanupWeatherData(WeatherData& data, const ConfigData& config) {
    if (data.cityMap != nullptr) {
        int rows = config.gridY_max - config.gridY_min + 1;
        for (int y = 0; y < rows; y++) {
            delete[] data.cityMap[y];
        }
        delete[] data.cityMap;
        data.cityMap = nullptr;
    }
    
    if (data.cloudCover != nullptr) {
        int rows = config.gridY_max - config.gridY_min + 1;
        for (int y = 0; y < rows; y++) {
            delete[] data.cloudCover[y];
        }
        delete[] data.cloudCover;
        data.cloudCover = nullptr;
    }
    
    if (data.pressure != nullptr) {
        int rows = config.gridY_max - config.gridY_min + 1;
        for (int y = 0; y < rows; y++) {
            delete[] data.pressure[y];
        }
        delete[] data.pressure;
        data.pressure = nullptr;
    }
    
    if (data.cities != nullptr) {
        delete[] data.cities;
        data.cities = nullptr;
    }
}

void displayMainMenu() {
    cout << "\nStudent ID: 1035289";
    cout << "\nStudent Name: Ayken Lee";    
    cout << "\n-------------------------------------";
    cout << "\nWeather Information Processing System\n";
    cout << "1) Read configuration file\n";
    cout << "2) Display city map\n";
    cout << "3) Display cloud coverage map (cloudiness index)\n";
    cout << "4) Display cloud coverage map (LMH symbols)\n";
    cout << "5) Display atmospheric pressure map (pressure index)\n";
    cout << "6) Display atmospheric pressure map (LMH symbols)\n";
    cout << "7) Display weather forecast summary report\n";
    cout << "8) Quit\n";
    cout << "Enter your choice: ";
}

bool readConfigFile(ConfigData& config, WeatherData& weather) {
    string configFileName;
    cout << "\n[Read in and process a config file]\n";
    cout << "Enter config filename: ";
    getline(cin, configFileName);

    ifstream file(configFileName);
    if (!file.is_open()) {
        cerr << "Error: Could not open config file '" << configFileName << "'\n";
        return false;
    }

    cout << "\nReading configuration from: " << configFileName << endl;
    
    string line;
    int lineNum = 0;
    bool hasGridX = false, hasGridY = false, hasCity = false, hasCloud = false, hasPressure = false;
    
    while (getline(file, line)) {
        lineNum++;
        
        // Skip empty lines
        if (line.empty()) continue;
        
        // Trim whitespace from beginning and end
        line.erase(0, line.find_first_not_of(" \t"));
        line.erase(line.find_last_not_of(" \t") + 1);
        
        // Skip comment-only lines (lines starting with //)
        if (line.find("//") == 0) continue;
        
        // Remove inline comments
        size_t commentPos = line.find("//");
        if (commentPos != string::npos) {
            line = line.substr(0, commentPos);
            // Trim again after removing comment
            line.erase(line.find_last_not_of(" \t") + 1);
        }
        
        // Skip if line became empty after removing comments
        if (line.empty()) continue;

        cout << "Processing line " << lineNum << ": '" << line << "'" << endl;
        
        // Try to parse GridX_IdxRange
        if (!hasGridX && line.find("GridX_IdxRange") != string::npos) {
            // Find the equals sign
            size_t equalPos = line.find('=');
            if (equalPos != string::npos) {
                string value = line.substr(equalPos + 1);
                // Trim whitespace
                value.erase(0, value.find_first_not_of(" \t"));
                value.erase(value.find_last_not_of(" \t") + 1);
                
                size_t dashPos = value.find('-');
                if (dashPos != string::npos) {
                    try {
                        config.gridX_min = stoi(value.substr(0, dashPos));
                        config.gridX_max = stoi(value.substr(dashPos + 1));
                        hasGridX = true;
                        cout << "✓ Reading in GridX_IdxRange: " << config.gridX_min << "-"
                             << config.gridX_max << " ... done!\n";
                    } catch (...) {
                        cerr << "✗ Error parsing GridX_IdxRange on line " << lineNum << ": " << line << "\n";
                    }
                }
            }
        }
        // Try to parse GridY_IdxRange
        else if (!hasGridY && line.find("GridY_IdxRange") != string::npos) {
            size_t equalPos = line.find('=');
            if (equalPos != string::npos) {
                string value = line.substr(equalPos + 1);
                value.erase(0, value.find_first_not_of(" \t"));
                value.erase(value.find_last_not_of(" \t") + 1);
                
                size_t dashPos = value.find('-');
                if (dashPos != string::npos) {
                    try {
                        config.gridY_min = stoi(value.substr(0, dashPos));
                        config.gridY_max = stoi(value.substr(dashPos + 1));
                        hasGridY = true;
                        cout << "✓ Reading in GridY_IdxRange: " << config.gridY_min << "-"
                             << config.gridY_max << " ... done!\n";
                    } catch (...) {
                        cerr << "✗ Error parsing GridY_IdxRange on line " << lineNum << ": " << line << "\n";
                    }
                }
            }
        }
        // Try to parse city file (lines with .txt that don't contain cloud/pressure)
        else if (!hasCity && line.find(".txt") != string::npos) {
            // Check if this line contains city-related keywords or is a standalone filename
            if (line.find("city") != string::npos || line.find("City") != string::npos || 
                (!hasCity && !hasCloud && !hasPressure)) {
                // This is likely a city file line
                config.cityFile = line;
                hasCity = true;
                cout << "✓ City location file: " << config.cityFile << " ... found!\n";
            }
        }
        // Try to parse cloud file
        else if (!hasCloud && line.find(".txt") != string::npos) {
            if (line.find("cloud") != string::npos || line.find("Cloud") != string::npos) {
                config.cloudFile = line;
                hasCloud = true;
                cout << "✓ Cloud cover file: " << config.cloudFile << " ... found!\n";
            }
        }
        // Try to parse pressure file
        else if (!hasPressure && line.find(".txt") != string::npos) {
            if (line.find("pressure") != string::npos || line.find("Pressure") != string::npos) {
                config.pressureFile = line;
                hasPressure = true;
                cout << "✓ Pressure file: " << config.pressureFile << " ... found!\n";
            }
        }
        // Special handling for your exact format - standalone filenames
        else if (!hasCity && lineNum > 4 && line.find(".txt") != string::npos && !hasCloud && !hasPressure) {
            // First .txt file after Grid lines is likely city file
            config.cityFile = line;
            hasCity = true;
            cout << "✓ City location file: " << config.cityFile << " ... found!\n";
        }
        else if (!hasCloud && hasCity && line.find(".txt") != string::npos && !hasPressure) {
            // Second .txt file is likely cloud file
            config.cloudFile = line;
            hasCloud = true;
            cout << "✓ Cloud cover file: " << config.cloudFile << " ... found!\n";
        }
        else if (!hasPressure && hasCloud && line.find(".txt") != string::npos) {
            // Third .txt file is likely pressure file
            config.pressureFile = line;
            hasPressure = true;
            cout << "✓ Pressure file: " << config.pressureFile << " ... found!\n";
        }
    }

    file.close();
    
    // If we still haven't found files by keywords, assume they're in order
    if (!hasCity && !hasCloud && !hasPressure) {
        // Re-open file and parse in order
        file.open(configFileName);
        if (file.is_open()) {
            vector<string> txtFiles;
            string line2;
            while (getline(file, line2)) {
                // Remove comments
                size_t commentPos = line2.find("//");
                if (commentPos != string::npos) {
                    line2 = line2.substr(0, commentPos);
                }
                
                // Check if line contains .txt
                if (line2.find(".txt") != string::npos) {
                    // Extract the filename
                    size_t txtPos = line2.find(".txt");
                    size_t start = line2.find_last_of(" \t");
                    if (start == string::npos) start = 0;
                    else start++;
                    string filename = line2.substr(start, txtPos + 4 - start);
                    
                    // Trim whitespace
                    filename.erase(0, filename.find_first_not_of(" \t"));
                    filename.erase(filename.find_last_not_of(" \t") + 1);
                    
                    if (!filename.empty()) {
                        txtFiles.push_back(filename);
                    }
                }
            }
            file.close();
            
            // Assign files in order
            if (txtFiles.size() >= 1) {
                config.cityFile = txtFiles[0];
                hasCity = true;
                cout << "✓ City location file (by order): " << config.cityFile << "\n";
            }
            if (txtFiles.size() >= 2) {
                config.cloudFile = txtFiles[1];
                hasCloud = true;
                cout << "✓ Cloud cover file (by order): " << config.cloudFile << "\n";
            }
            if (txtFiles.size() >= 3) {
                config.pressureFile = txtFiles[2];
                hasPressure = true;
                cout << "✓ Pressure file (by order): " << config.pressureFile << "\n";
            }
        }
    }
    
    // Validate that we have all required information
    cout << "\nValidating configuration...\n";
    
    if (!hasGridX) {
        cerr << "Error: GridX_IdxRange not found in config file\n";
        return false;
    }
    if (!hasGridY) {
        cerr << "Error: GridY_IdxRange not found in config file\n";
        return false;
    }
    if (!hasCity) {
        cerr << "Error: City location file not specified in config\n";
        return false;
    }
    if (!hasCloud) {
        cerr << "Error: Cloud cover file not specified in config\n";
        return false;
    }
    if (!hasPressure) {
        cerr << "Error: Pressure file not specified in config\n";
        return false;
    }
    
    // Validate grid ranges
    if (config.gridX_min > config.gridX_max) {
        cerr << "Error: GridX_IdxRange min > max\n";
        return false;
    }
    if (config.gridY_min > config.gridY_max) {
        cerr << "Error: GridY_IdxRange min > max\n";
        return false;
    }
    
    // Check if files exist
    cout << "\nChecking if data files exist...\n";
    if (!fileExists(config.cityFile)) {
        cerr << "Error: City file '" << config.cityFile << "' not found\n";
        return false;
    }
    cout << "✓ City file '" << config.cityFile << "' found\n";
    
    if (!fileExists(config.cloudFile)) {
        cerr << "Error: Cloud cover file '" << config.cloudFile << "' not found\n";
        return false;
    }
    cout << "✓ Cloud cover file '" << config.cloudFile << "' found\n";
    
    if (!fileExists(config.pressureFile)) {
        cerr << "Error: Pressure file '" << config.pressureFile << "' not found\n";
        return false;
    }
    cout << "✓ Pressure file '" << config.pressureFile << "' found\n";
    
    // Initialize data structures
    cout << "\nInitializing data structures...\n";
    initializeCityMap(weather, config);
    
    // Load data from files
    cout << "\nLoading data from files...\n";
    
    if (!loadCityData(weather, config, config.cityFile)) {
        cerr << "Failed to load city data\n";
        return false;
    }
    cout << "✓ City data loaded from '" << config.cityFile << "'\n";
    
    if (!loadCloudData(weather, config, config.cloudFile)) {
        cerr << "Failed to load cloud data\n";
        return false;
    }
    cout << "✓ Cloud data loaded from '" << config.cloudFile << "'\n";
    
    if (!loadPressureData(weather, config, config.pressureFile)) {
        cerr << "Failed to load pressure data\n";
        return false;
    }
    cout << "✓ Pressure data loaded from '" << config.pressureFile << "'\n";

    cout << "\nConfiguration loaded successfully!\n";
    return true;
}

bool loadCityData(WeatherData& data, const ConfigData& config, const string& filename) {
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "Error: Could not open city file '" << filename << "'\n";
        return false;
    }

    string line;
    while (getline(file, line)) {
        if (data.cityCount >= data.cityCapacity) {
            CityData* newCities = new CityData[data.cityCapacity * 2];
            for (int i = 0; i < data.cityCount; i++) {
                newCities[i] = data.cities[i];
            }
            delete[] data.cities;
            data.cities = newCities;
            data.cityCapacity *= 2;
        }

        CityData& city = data.cities[data.cityCount];
        
        size_t bracketEnd = line.find(']');
        if (bracketEnd == string::npos) continue;

        string coordStr = line.substr(1, bracketEnd - 1);
        size_t commaPos = coordStr.find(',');
        if (commaPos == string::npos) continue;

        try {
            city.x = stoi(coordStr.substr(0, commaPos));
            city.y = stoi(coordStr.substr(commaPos + 1));
            
            size_t firstDash = line.find('-', bracketEnd);
            size_t secondDash = line.find('-', firstDash + 1);
            
            city.cityId = stoi(line.substr(firstDash + 1, secondDash - firstDash - 1));
            city.cityName = line.substr(secondDash + 1);
            
            if (!validateGridCoordinates(city.x, city.y, config)) {
                cerr << "Warning: City coordinates (" << city.x << "," << city.y 
                     << ") out of grid range - skipping\n";
                continue;
            }

            int gridX = city.x - config.gridX_min;
            int gridY = city.y - config.gridY_min;
            int rows = config.gridY_max - config.gridY_min + 1;
            int cols = config.gridX_max - config.gridX_min + 1;
            
            if (gridX >= 0 && gridX < cols && gridY >= 0 && gridY < rows) {
                if (city.cityId >= 0 && city.cityId <= 9) {
                    char symbol = '0' + city.cityId;
                    data.cityMap[gridY][gridX] = symbol;
                    data.cityCount++;
                } else {
                    cerr << "Warning: Invalid city ID " << city.cityId << " - must be 0-9\n";
                }
            }
        } catch (...) {
            cerr << "Warning: Error parsing city data - skipping line\n";
            continue;
        }
    }

    file.close();
    if (data.cityCount == 0) {
        cerr << "Error: No valid city data loaded\n";
        return false;
    }
    return true;
}

bool loadCloudData(WeatherData& data, const ConfigData& config, const string& filename) {
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "Error: Could not open cloud cover file '" << filename << "'\n";
        return false;
    }

    string line;
    while (getline(file, line)) {
        line = line.substr(0, line.find("//"));
        line.erase(remove_if(line.begin(), line.end(), ::isspace), line.end());
        if (line.empty()) continue;

        size_t bracketEnd = line.find(']');
        if (bracketEnd == string::npos) continue;

        string coordStr = line.substr(1, bracketEnd - 1);
        size_t commaPos = coordStr.find(',');
        if (commaPos == string::npos) continue;

        try {
            int x = stoi(coordStr.substr(0, commaPos));
            int y = stoi(coordStr.substr(commaPos + 1));
            
            if (!validateGridCoordinates(x, y, config)) {
                cerr << "Warning: Cloud coordinates (" << x << "," << y 
                     << ") out of grid range - skipping\n";
                continue;
            }

            size_t dashPos = line.find('-', bracketEnd);
            int value = stoi(line.substr(dashPos + 1));
            
            if (value < 0 || value > 100) {
                cerr << "Warning: Invalid cloud cover value " << value << " - must be 0-100\n";
                continue;
            }

            int gridX = x - config.gridX_min;
            int gridY = y - config.gridY_min;
            int rows = config.gridY_max - config.gridY_min + 1;
            int cols = config.gridX_max - config.gridX_min + 1;
            
            if (gridX >= 0 && gridX < cols && gridY >= 0 && gridY < rows) {
                data.cloudCover[gridY][gridX] = value;
            }
        } catch (...) {
            cerr << "Warning: Error parsing cloud data - skipping line\n";
            continue;
        }
    }

    file.close();
    return true;
}

bool loadPressureData(WeatherData& data, const ConfigData& config, const string& filename) {
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "Error: Could not open pressure file '" << filename << "'\n";
        return false;
    }

    string line;
    while (getline(file, line)) {
        line = line.substr(0, line.find("//"));
        line.erase(remove_if(line.begin(), line.end(), ::isspace), line.end());
        if (line.empty()) continue;

        size_t bracketEnd = line.find(']');
        if (bracketEnd == string::npos) continue;

        string coordStr = line.substr(1, bracketEnd - 1);
        size_t commaPos = coordStr.find(',');
        if (commaPos == string::npos) continue;

        try {
            int x = stoi(coordStr.substr(0, commaPos));
            int y = stoi(coordStr.substr(commaPos + 1));
            
            if (!validateGridCoordinates(x, y, config)) {
                cerr << "Warning: Pressure coordinates (" << x << "," << y 
                     << ") out of grid range - skipping\n";
                continue;
            }

            size_t dashPos = line.find('-', bracketEnd);
            int value = stoi(line.substr(dashPos + 1));
            
            if (value < 0 || value > 100) {
                cerr << "Warning: Invalid pressure value " << value << " - must be 0-100\n";
                continue;
            }

            int gridX = x - config.gridX_min;
            int gridY = y - config.gridY_min;
            int rows = config.gridY_max - config.gridY_min + 1;
            int cols = config.gridX_max - config.gridX_min + 1;
            
            if (gridX >= 0 && gridX < cols && gridY >= 0 && gridY < rows) {
                data.pressure[gridY][gridX] = value;
            }
        } catch (...) {
            cerr << "Warning: Error parsing pressure data - skipping line\n";
            continue;
        }
    }

    file.close();
    return true;
}

void displayCityMap(const WeatherData& weather, const ConfigData& config) {
    int rows = config.gridY_max - config.gridY_min + 1;
    int cols = config.gridX_max - config.gridX_min + 1;

    int maxYLabelWidth = to_string(config.gridY_max).length();
    
    // Print top border
    cout << string(maxYLabelWidth + 1, ' ') << "#";
    for (int x = 0; x <= cols; x++) {
        cout << "   #";
    }
    cout << endl;

    // Print grid rows
    for (int y = rows - 1; y >= 0; y--) {
        cout << setw(maxYLabelWidth) << (config.gridY_min + y) << " #";
        for (int x = 0; x < cols; x++) {
            cout << "   " << weather.cityMap[y][x];
        }
        cout << "   #" << endl;
    }

    // Print bottom border
    cout << string(maxYLabelWidth + 1, ' ') << "#";
    for (int x = 0; x <= cols; x++) {
        cout << "   #";
    }
    cout << endl;

    // Print X-axis labels - perfectly aligned
    cout << string(maxYLabelWidth + 2, ' ');
    for (int x = config.gridX_min; x <= config.gridX_max; x++) {
        cout << setw(4) << x;
    }
    cout << endl;
}

int getCloudinessIndex(int value) {
    if (value >= 90) return 9;
    if (value >= 80) return 8;
    if (value >= 70) return 7;
    if (value >= 60) return 6;
    if (value >= 50) return 5;
    if (value >= 40) return 4;
    if (value >= 30) return 3;
    if (value >= 20) return 2;
    if (value >= 10) return 1;
    return 0;
}

int getPressureIndex(int value) {
    if (value >= 90) return 9;
    if (value >= 80) return 8;
    if (value >= 70) return 7;
    if (value >= 60) return 6;
    if (value >= 50) return 5;
    if (value >= 40) return 4;
    if (value >= 30) return 3;
    if (value >= 20) return 2;
    if (value >= 10) return 1;
    return 0;
}

void displayCloudCoverage(const WeatherData& weather, const ConfigData& config, bool showLMH) {
    int rows = config.gridY_max - config.gridY_min + 1;
    int cols = config.gridX_max - config.gridX_min + 1;

    int maxYLabelWidth = to_string(config.gridY_max).length();
    
    // Print top border
    cout << string(maxYLabelWidth + 1, ' ') << "#";
    for (int x = 0; x <= cols; x++) {
        cout << "   #";
    }
    cout << endl;

    // Print grid rows
    for (int y = rows - 1; y >= 0; y--) {
        cout << setw(maxYLabelWidth) << (config.gridY_min + y) << " #";
        for (int x = 0; x < cols; x++) {
            if (showLMH) {
                cout << "   " << getLMHSymbol(weather.cloudCover[y][x]);
            } else {
                cout << "   " << getCloudinessIndex(weather.cloudCover[y][x]);
            }
        }
        cout << "   #" << endl;
    }

    // Print bottom border
    cout << string(maxYLabelWidth + 1, ' ') << "#";
    for (int x = 0; x <= cols; x++) {
        cout << "   #";
    }
    cout << endl;

    // Print X-axis labels
    cout << string(maxYLabelWidth + 2, ' ');
    for (int x = config.gridX_min; x <= config.gridX_max; x++) {
        cout << setw(4) << x;
    }
    cout << endl;

    if (showLMH) {
        cout << "\nLMH Legend:\n";
        cout << " L = Low (0-34)\n";
        cout << " M = Medium (35-64)\n";
        cout << " H = High (65-100)\n";
    }
}

void displayPressureMap(const WeatherData& weather, const ConfigData& config, bool showLMH) {
    int rows = config.gridY_max - config.gridY_min + 1;
    int cols = config.gridX_max - config.gridX_min + 1;

    int maxYLabelWidth = to_string(config.gridY_max).length();
    
    // Print top border
    cout << string(maxYLabelWidth + 1, ' ') << "#";
    for (int x = 0; x <= cols; x++) {
        cout << "   #";
    }
    cout << endl;

    // Print grid rows
    for (int y = rows - 1; y >= 0; y--) {
        cout << setw(maxYLabelWidth) << (config.gridY_min + y) << " #";
        for (int x = 0; x < cols; x++) {
            if (showLMH) {
                cout << "   " << getPressureLMHSymbol(weather.pressure[y][x]);
            } else {
                cout << "   " << getPressureIndex(weather.pressure[y][x]);
            }
        }
        cout << "   #" << endl;
    }

    // Print bottom border
    cout << string(maxYLabelWidth + 1, ' ') << "#";
    for (int x = 0; x <= cols; x++) {
        cout << "   #";
    }
    cout << endl;

    // Print X-axis labels
    cout << string(maxYLabelWidth + 2, ' ');
    for (int x = config.gridX_min; x <= config.gridX_max; x++) {
        cout << setw(4) << x;
    }
    cout << endl;

    if (showLMH) {
        cout << "\nPressure LMH Legend:\n";
        cout << " L = Low (0-34)\n";
        cout << " M = Medium (35-64)\n";
        cout << " H = High (65-100)\n";
    }
}

void displayWeatherSummary(const WeatherData& weather, const ConfigData& config) {
    if (weather.cityCount == 0) {
        cout << "No city data available. Please load configuration first.\n";
        return;
    }

    if (weather.cloudCover == nullptr || weather.pressure == nullptr) {
        cout << "Weather data not available. Please load configuration first.\n";
        return;
    }

    cout << "\nWeather Forecast Summary Report\n";
    cout << "-------------------------------\n";

    // Create visited array 
    bool** visited = new bool*[config.gridY_max - config.gridY_min + 1];
    CityData*** cityInfoMap = new CityData**[config.gridY_max - config.gridY_min + 1];
    for (int y = 0; y <= config.gridY_max - config.gridY_min; y++) {
        visited[y] = new bool[config.gridX_max - config.gridX_min + 1]();
        cityInfoMap[y] = new CityData*[config.gridX_max - config.gridX_min + 1]();
    }

    // Initialize city info map
    for (int i = 0; i < weather.cityCount; i++) {
        int x = weather.cities[i].x - config.gridX_min;
        int y = weather.cities[i].y - config.gridY_min;
        cityInfoMap[y][x] = &weather.cities[i];
    }

    // Process each city
    for (int i = 0; i < weather.cityCount; i++) {
        int startX = weather.cities[i].x - config.gridX_min;
        int startY = weather.cities[i].y - config.gridY_min;

        if (visited[startY][startX]) continue;

        string cityName = weather.cities[i].cityName;
        int cityId = weather.cities[i].cityId;
        int minX = weather.cities[i].x;
        int maxX = weather.cities[i].x;
        int minY = weather.cities[i].y;
        int maxY = weather.cities[i].y;
        int citySize = 0;

        // Use a queue for BFS to find all connected city coordinates
        int queueSize = weather.cityCount * 4; // Sufficiently large
        int* queueX = new int[queueSize];
        int* queueY = new int[queueSize];
        int front = 0, rear = 0;
        queueX[rear] = weather.cities[i].x;
        queueY[rear] = weather.cities[i].y;
        rear++;
        visited[startY][startX] = true;
        citySize++;

        // Program moves Up left right, down and up
        const int dx[] = {0, 0, -1, 1};
        const int dy[] = {-1, 1, 0, 0};

        while (front < rear) {
            int currentX = queueX[front];
            int currentY = queueY[front];
            front++;

            // Update city boundaries
            minX = min(minX, currentX);
            maxX = max(maxX, currentX);
            minY = min(minY, currentY);
            maxY = max(maxY, currentY);

            for (int dir = 0; dir < 4; dir++) {
                int nx = currentX + dx[dir];
                int ny = currentY + dy[dir];

                // Check if its within grid bounds
                if (nx < config.gridX_min || nx > config.gridX_max ||
                    ny < config.gridY_min || ny > config.gridY_max) {
                    continue;
                }

                int gridX = nx - config.gridX_min;
                int gridY = ny - config.gridY_min;

                // Check if this is a city coordinate and not visited
                if (cityInfoMap[gridY][gridX] != nullptr && !visited[gridY][gridX]) {
                    if (cityInfoMap[gridY][gridX]->cityName == cityName) {
                        visited[gridY][gridX] = true;
                        queueX[rear] = nx;
                        queueY[rear] = ny;
                        rear++;
                        citySize++;
                    }
                }
            }
        }

        delete[] queueX;
        delete[] queueY;

        // Calculate the surrounding area (1 grid around the city boundaries)
        int areaMinX = max(minX - 1, config.gridX_min);
        int areaMaxX = min(maxX + 1, config.gridX_max);
        int areaMinY = max(minY - 1, config.gridY_min);
        int areaMaxY = min(maxY + 1, config.gridY_max);

        // Calculate average cloud cover and pressure
        double cloudSum = 0;
        double pressureSum = 0;
        int areaCount = 0;

        for (int y = areaMinY; y <= areaMaxY; y++) {
            for (int x = areaMinX; x <= areaMaxX; x++) {
                int gridX = x - config.gridX_min;
                int gridY = y - config.gridY_min;
                cloudSum += weather.cloudCover[gridY][gridX];
                pressureSum += weather.pressure[gridY][gridX];
                areaCount++;
            }
        }

        double avgCloud = cloudSum / areaCount;
        double avgPressure = pressureSum / areaCount;

        // Get LMH classifications
        string cloudLMH = getLMHSymbol(static_cast<int>(avgCloud));
        string pressureLMH = getPressureLMHSymbol(static_cast<int>(avgPressure));

        // Calculate rain probability
        double rainProbability = calculateRainProbability(cloudLMH, pressureLMH);

        // Display the report
        cout << "\nCity Name: " << cityName << "\n";
        cout << "City ID: " << cityId << "\n";
        cout << "Ave. Cloud Cover (ACC): " << fixed << setprecision(2) << avgCloud 
             << " (" << cloudLMH << ")\n";
        cout << "Ave. Pressure (AP): " << fixed << setprecision(2) << avgPressure 
             << " (" << pressureLMH << ")\n";
        cout << "Probability of Rain (%): " << fixed << setprecision(2) << rainProbability << "\n";
        
        // Display ASCII graphics based on rain probability
        if (rainProbability >= 90) {          
            cout << "~~~~\n";
            cout << "~~~~~\n";
            cout << "\\\\\\\\\\ \n";
        }
        else if (rainProbability >= 80) {
            cout << "~~~~\n";
            cout << "~~~~~\n";
            cout << " \\\\\\\\ \n";
        }
        else if (rainProbability >= 70) {
            cout << "~~~~\n";
            cout << "~~~~~\n";
            cout << "  \\\\\\ \n";
        }
        else if (rainProbability >= 60) {
            cout << "~~~~\n";
            cout << "~~~~~\n";
            cout << "   \\\\ \n";
        }
        else if (rainProbability >= 50) {
            cout << "~~~~\n";
            cout << "~~~~~\n";
            cout << "    \\ \n";
        }
        else if (rainProbability >= 40) {
            cout << "~~~~\n";
            cout << "~~~~~\n";
            cout << "\n";
        }
        else if (rainProbability >= 30) {
            cout << "~~~\n";
            cout << "~~~~\n";
            cout << "\n";
        }
        else if (rainProbability >= 20) {
            cout << "~~\n";
            cout << "~~~\n";
            cout << "\n";
        }
        else if (rainProbability >= 10) {
            cout << "~\n";
            cout << "~~\n";
            cout << "\n";
        }
    }

    for (int y = 0; y <= config.gridY_max - config.gridY_min; y++) {
        delete[] visited[y];
        delete[] cityInfoMap[y];
    }
    delete[] visited;
    delete[] cityInfoMap;
}

double calculateRainProbability(const string& cloudLMH, const string& pressureLMH) {
    if (cloudLMH == "H" && pressureLMH == "L") return 90.0;
    if (cloudLMH == "M" && pressureLMH == "L") return 80.0;
    if (cloudLMH == "L" && pressureLMH == "L") return 70.0;
    if (cloudLMH == "H" && pressureLMH == "M") return 60.0;
    if (cloudLMH == "M" && pressureLMH == "M") return 50.0;
    if (cloudLMH == "L" && pressureLMH == "M") return 40.0;
    if (cloudLMH == "H" && pressureLMH == "H") return 30.0;
    if (cloudLMH == "M" && pressureLMH == "H") return 20.0;
    if (cloudLMH == "L" && pressureLMH == "H") return 10.0;
    return 0.0;
}

string getLMHSymbol(int value) {
    if (value >= 65) return "H";
    if (value >= 35) return "M";
    return "L";
}

string getPressureLMHSymbol(int value) {
    if (value >= 65) return "H";
    if (value >= 35) return "M";
    return "L";
}

void processMenuChoice(int choice, ConfigData& config, WeatherData& weather) {
    switch(choice) {
        case 1:
            if (readConfigFile(config, weather)) {
                cout << "\nAll records successfully stored!\n";
            } else {
                cout << "Failed to load configuration.\n";
            }
            break;
        case 2:
            if (weather.cityCount == 0) {
                cout << "No city data available. Please load configuration first.\n";
            } else {
                displayCityMap(weather, config);
            }
            break;
        case 3:
            if (weather.cloudCover == nullptr) {
                cout << "No cloud data available. Please load configuration first.\n";
            } else {
                displayCloudCoverage(weather, config, false);
            }
            break;
        case 4:
            if (weather.cloudCover == nullptr) {
                cout << "No cloud data available. Please load configuration first.\n";
            } else {
                displayCloudCoverage(weather, config, true);
            }
            break;
        case 5:
            if (weather.pressure == nullptr) {
                cout << "No pressure data available. Please load configuration first.\n";
            } else {
                displayPressureMap(weather, config, false);
            }
            break;
        case 6:
            if (weather.pressure == nullptr) {
                cout << "No pressure data available. Please load configuration first.\n";
            } else {
                displayPressureMap(weather, config, true);
            }
            break;
        case 7:
            displayWeatherSummary(weather, config);
            break;
        case 8:
            cout << "Exiting program...\n";
            break;
        default:
            cout << "Invalid choice. Please try again.\n";
    }

    if (choice != 8) {
        cout << "\nPress Enter to continue...";
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }
}
