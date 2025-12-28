#include <iostream>
#include <iomanip>
#include <string>
#include <fstream>
#include <sstream>
#include <cmath>
using namespace std;
 
// ---- DEFINITIONS AND CONSTANTS ----
#define PI acos(-1.0)
#define DISTANCE 75 // meters acceleration distance (75 meters based on D 5.1.1 Formula Student Rules 2026)
#define COLS 13 // number of columns in a racebox csv

// ---- RACEBOX COLUMN HEADERS -----
#define Record 0
#define Time 1
#define Latitude 2
#define Longitude 3
#define Altitude 4
#define Speed 5
#define GForceX 6
#define GForceY 7
#define GForceZ 8
#define Lap 9
#define GyroX 10
#define GyroY 11
#define GyroZ 12

// ----- ROLLOUT SETTINGS -----
// As per Formula Student 2026 handbook rule D 5.2.3 the vehicle is staged 0.3 m behind the start line
// Set to 1 to enable [ROLLOUT_DISTANCE] m rollout before starting 75m measurement
// Set to 0 to start measuring immediately from speed=0
#define USE_ROLLOUT 1
#define ROLLOUT_DISTANCE 0.3 // meters

// ----- IMPORT TEMPLATE -----
// ifstream racebox("src/[your_file_name].csv");

// ----- IMPORTS LIST - KEEP JUST ONE LINE UNCOMMENTED AT A TIME -----
// ifstream racebox("src/RaceBox25-10-2025_18-15.csv");
// ifstream racebox("src/RaceBox25-10-2025_16-00.csv");
// ifstream racebox("src/RaceBox25-10-2025_17-36.csv");
// ifstream racebox("src/RaceBox19-10-2025_17-21.csv"); // 3 runs rolling
// ifstream racebox("src/RaceBox19-10-2025_17-09.csv"); // 3 runs from spreadsheet
// ifstream racebox("src/RaceBox19-10-2025_17-14.csv"); // 3 runs from spreadsheet no 2
// ifstream racebox("src/RaceBox19-10-2025_14-33.csv"); // two weird runs; not in the spreadsheet
// ifstream racebox("src/RaceBox19-10-2025_15-37.csv"); // 6 runs from the spreadsheet
ifstream racebox("src/RaceBox19-10-2025_15-49.csv"); // 2 runs to be filled in the spreadsheet

// ----- FUNCTIONS -----

double degreesToRadians(double degrees) {
	return degrees * PI / 180;
}

// calculate a straight line distance between two sets of geographic coordinated
double distanceGeoM(double lat1, double lon1, double lat2, double lon2)
{
	double earthRadiusKm = 6371.0;

	const double dLat = degreesToRadians(lat2 - lat1);
	const double dLon = degreesToRadians(lon2 - lon1);

	const double rLat1 = degreesToRadians(lat1);
	const double rLat2 = degreesToRadians(lat2);

	const double sinDLat = sin(dLat * 0.5);
	const double sinDLon = sin(dLon * 0.5);

	const double a = sinDLat * sinDLat + sinDLon * sinDLon * cos(rLat1) * cos(rLat2);

	const double c = 2.0 * atan2(sqrt(a), sqrt(1.0 - a));
	return earthRadiusKm * c * 1000;
}

// expected time format: YYYY-MM-DDTHH:MM:SS.000, returns time in miliseconds
long long parseIsoToMillis(const string& s) {
	
	if (s.size() < 19) throw runtime_error("ISO time too short: " + s);

	auto to2 = [&](int pos) -> int {
		return (s[pos] - '0') * 10 + (s[pos + 1] - '0');
	};
	auto to4 = [&](int pos) -> int {
		return (s[pos] - '0') * 1000 + (s[pos + 1] - '0') * 100 + (s[pos + 2] - '0') * 10 + (s[pos + 3] - '0');
	};

	int Y = to4(0);
	int M = to2(5);
	int D = to2(8);
	int h = to2(11);
	int m = to2(14);
	int sec = to2(17);

	int ms = 0;
	if (s.size() >= 23 && s[19] == '.') {
		ms = (s[20] - '0') * 100 + (s[21] - '0') * 10 + (s[22] - '0');
	}

	// Convert date to a monotonically increasing day count (no timezone, no leap seconds drama).
	// Howard Hinnant's civil-from-days style math (compact version).
	auto days_from_civil = [](int y, int mo, int d) -> long long {
		y -= mo <= 2;
		const int era = (y >= 0 ? y : y - 399) / 400;
		const unsigned yoe = (unsigned)(y - era * 400);                  // [0, 399]
		const unsigned doy = (153 * (mo + (mo > 2 ? -3 : 9)) + 2) / 5 + d - 1; // [0, 365]
		const unsigned doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;      // [0, 146096]
		return (long long)era * 146097 + (long long)doe;
	};

	long long days = days_from_civil(Y, M, D);
	long long total_ms = (((days * 24LL + h) * 60LL + m) * 60LL + sec) * 1000LL + ms;
	return total_ms;
}

bool looksNumericSeconds(const string& s) {
	// Accept digits, one dot, optional leading spaces and sign
	bool seenDigit = false, seenDot = false;
	size_t i = 0;
	while (i < s.size() && isspace((unsigned char)s[i])) i++;
	if (i < s.size() && (s[i] == '+' || s[i] == '-')) i++;

	for (; i < s.size(); i++) {
		char c = s[i];
		if (isdigit((unsigned char)c)) { seenDigit = true; continue; }
		if (c == '.' && !seenDot) { seenDot = true; continue; }
		if (isspace((unsigned char)c)) continue;
		return false;
	}
	return seenDigit;
}

// convert time as it occurs in racebox csv to seconds since session start format
double parseTimeSecondsSinceStart(const string& timeField) {
	static bool haveT0 = false;
	static long long t0_ms = 0;

	if (looksNumericSeconds(timeField)) {
		// Numeric mode: already seconds since start
		return stod(timeField);
	}

	// ISO mode
	long long ms = parseIsoToMillis(timeField);
	if (!haveT0) { haveT0 = true; t0_ms = ms; }
	return (ms - t0_ms) / 1000.0;
}

// ----- MAIN -----

int main()
{
	string entry; // one line from racebox file
	double time = 0, latitude = 0, longitude = 0, time0 = 0, lat0 = 0, long0 = 0, lat01 = 0, long01 = 0, prevlat = 0, prevlong = 0, distcalc = 0;
	int lap, prevlap = 0;
	int speed = 0;
	bool run;
	
#if USE_ROLLOUT
	// Rollout mode: track when we've traveled ROLLOUT_DISTANCE from speed=0
	bool waitingForRollout = false;
	double rolloutStartLat = 0, rolloutStartLong = 0;
#endif
	
	getline(racebox, entry);
	run = false;
	while (getline(racebox, entry))
	{
		string field;
		stringstream ss(entry);

		int col = 0;
		while (getline(ss, field, ','))
		{
			if (col == Time) time = parseTimeSecondsSinceStart(field);
			if (col == Latitude) latitude = stod(field);
			if (col == Longitude) longitude = stod(field);
			if (col == Speed) speed = stoi(field);
			if (col == Lap) lap = stoi(field);
			col++;
		}
		if (lap != prevlap)
		{
			if (lap != 0)
			{
				lat01 = prevlat;
				long01 = prevlong;
			}
			if (lap == 0 || (prevlap != 0 && lap > prevlap))
			{
				// if session has the colldown bits between each run and runs are numbered in their column, check the distance between your set gates with the line below
				// cout << "racebox gate distance: " << distanceGeoM(latitude, longitude, lat01, long01) << endl;
			}
		}
		else
		{
			prevlat = latitude;
			prevlong = longitude;
		}
		prevlap = lap;
		
#if USE_ROLLOUT
		// With rollout: detect speed=0, wait for rollout distance, then start timing
		if (speed == 0)
		{
			// Reset to new starting point
			rolloutStartLat = latitude;
			rolloutStartLong = longitude;
			waitingForRollout = true;
			run = false;
		}
		else if (waitingForRollout)
		{
			double rolloutDist = distanceGeoM(rolloutStartLat, rolloutStartLong, latitude, longitude);
			if (rolloutDist >= ROLLOUT_DISTANCE)
			{
				// Rollout complete, start timing from here
				time0 = time;
				lat0 = latitude;
				long0 = longitude;
				run = true;
				waitingForRollout = false;
			}
		}
		
		if (run && distanceGeoM(lat0, long0, latitude, longitude) >= DISTANCE)
		{
			cout << "0-75m time (with " << ROLLOUT_DISTANCE << "m rollout): " << fixed << setprecision(3) << time - time0 << " s" << endl;
			run = false;
		}
#else
		// Without rollout: start timing immediately from speed=0
		if (speed == 0)
		{
			time0 = time;
			lat0 = latitude;
			long0 = longitude;
			run = true;
		}
		if (run && distanceGeoM(lat0, long0, latitude, longitude) >= DISTANCE)
		{
			cout << "0-75m time: " << fixed << setprecision(3) << time - time0 << " s" << endl;
			run = false;
		}
#endif
		
		// cout << fixed << setprecision(10) << time << '\t' << latitude << '\n';
	}
}
