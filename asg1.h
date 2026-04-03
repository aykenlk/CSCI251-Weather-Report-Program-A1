// asg1.h file
#ifndef ASG1_H
#define ASG1_H

#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <cctype>
#include <iomanip>
#include <algorithm>
#include <limits>
#include <queue>

using namespace std;

struct ConfigData {
    int gridX_min, gridX_max; 
    int gridY_min, gridY_max;
    string cityFile;
    string cloudFile;
    string pressureFile;
};

struct CityData {
    int x, y;
    int cityId;
    string cityName;
};

struct WeatherData {
    char** cityMap;
    int** cloudCover;
    int** pressure;
    CityData* cities;
    int cityCount;
    int cityCapacity;
};

// Function prototypes
bool loadCloudData(WeatherData& data, const ConfigData& config, const string& filename);
bool loadPressureData(WeatherData& data, const ConfigData& config, const string& filename);
void displayCloudCoverage(const WeatherData& weather, const ConfigData& config, bool showLMH = false);
void displayPressureMap(const WeatherData& weather, const ConfigData& config, bool showLMH = false);
void displayWeatherSummary(const WeatherData& weather, const ConfigData& config);
string getLMHSymbol(int value);
string getPressureLMHSymbol(int value);
int getCloudinessIndex(int value);
int getPressureIndex(int value);
void displayMainMenu();
bool readConfigFile(ConfigData& config, WeatherData& weather);
void displayCityMap(const WeatherData& weather, const ConfigData& config);
void processMenuChoice(int choice, ConfigData& config, WeatherData& weather);
void initializeCityMap(WeatherData& weather, const ConfigData& config);
bool loadCityData(WeatherData& data, const ConfigData& config, const string& filename);
void cleanupWeatherData(WeatherData& data, const ConfigData& config);
bool validateGridCoordinates(int x, int y, const ConfigData& config);
double calculateRainProbability(const string& cloudLMH, const string& pressureLMH);

#endif // ASG1_H


