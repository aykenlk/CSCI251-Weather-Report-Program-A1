# Weather Information Processing System
A C++ console application that processes weather data (cloud cover and atmospheric pressure) from text files, displays interactive maps, and generates forecast summaries with rain probability predictions.

# Weather Information Processing System

A C++ console application that processes weather data (cloud cover and atmospheric pressure) from text files, displays interactive maps, and generates forecast summaries with rain probability predictions.

## Features

- **Configuration File Reader** – Reads grid dimensions and input file names from a config file.
- **City Map Display** – Renders a grid-based city map with city IDs as numbered markers.
- **Cloud Coverage Maps** – Shows cloudiness index (0–9) or LMH symbols (Low/Medium/High).
- **Atmospheric Pressure Maps** – Displays pressure index or LMH symbols.
- **Weather Forecast Summary** – Computes average cloud cover and pressure per city area, calculates rain probability, and shows ASCII rain graphics.

## How It Works

1. The user provides a configuration file containing:
   - `GridX_IdxRange` and `GridY_IdxRange` (e.g., `0-9`)
   - Paths to three data files: city locations, cloud cover, and pressure.
2. The program reads city coordinates (`[x,y]-id-name`), cloud cover percentages, and pressure percentages.
3. Data is stored in dynamic 2D arrays sized to the grid range.
4. The user can view maps (raw indices or LMH symbols) and generate a forecast report for each city.

## Compilation & Execution

### Requirements
- C++11 or later compiler (g++, clang++, MSVC)

### Compile
```bash
g++ -std=c++11 csci251_a1.cpp -o weather_app
```

### Run
```bash
./weather_app
```

## File Formats

### Configuration File (example: `config.txt`)
```
GridX_IdxRange = 0-9
GridY_IdxRange = 0-9
citylocation.txt
cloudcover.txt
pressure.txt
```

### City File (`citylocation.txt`)
```
[0,0]-1-Central
[1,2]-2-Northgate
```

### Cloud / Pressure Files (`cloudcover.txt`, `pressure.txt`)
```
[0,0]-75
[1,2]-42
```

## Menu Options

1. Read configuration file  
2. Display city map  
3. Display cloud coverage map (cloudiness index)  
4. Display cloud coverage map (LMH symbols)  
5. Display atmospheric pressure map (pressure index)  
6. Display atmospheric pressure map (LMH symbols)  
7. Display weather forecast summary report  
8. Quit  

## Sample Output (Forecast Summary)

```
Weather Forecast Summary Report
-------------------------------

City Name: Central
City ID: 1
Ave. Cloud Cover (ACC): 68.50 (H)
Ave. Pressure (AP): 35.20 (M)
Probability of Rain (%): 60.00
~~~~
~~~~~
   \\
```

## Future Improvements

- Support for dynamic file naming (no hardcoded keywords)
- Whitespace tolerance in input files
- Finer index assignment (more than 10 levels)
- Real-time API data integration
- Additional weather metrics (temperature, wind speed)

## Author

Ayken Lee Kang  
CSCI251 Assignment 1
