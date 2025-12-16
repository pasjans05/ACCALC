// ACCALC.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <iomanip>
#include <string>
#include <fstream>
#include <sstream>
using namespace std;

#define PI 3.14
// racebox csv column headers:
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
#define COLS 13 // number of columns in a racebox csv
 
ifstream racebox("src/RaceBox25-10-2025_18-15.csv");

float readTime(string entry, int* i)
{
	double digit, time = 0;
	int decimal = 0;
	for (; entry[*i] != ','; (*i)++)
	{
		if (entry[*i] == '.')
		{
			decimal = 1;
		}
		else
		{
			digit = entry[*i] - '0';
			if (!decimal)
			{
				time *= 10;
				time += digit;
			}
			else
			{
				for (int j = 0; j < decimal; j++)
					digit /= 10;
				time += digit;
				decimal++;
			}
		}
	}
	return time;
}

double degreesToRadians(double degrees) {
	return degrees * PI / 180;
}

double distance(double lat1, double lon1, double lat2, double lon2)
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

int main()
{
	string entry; // one line from racebox file
	double time = 0, latitude = 0, longitude = 0, time0 = 0, lat0 = 0, long0 = 0;
	int speed = 0;
	bool isRead[COLS]; // variable to track whether we already read data from that column - columns numbered as in racebox csv header
	// 0 Record; 1 Time; 2 Latitude; 3 Longitude; 4 Altitude; 5 Speed; 6 GForceX; 7 GForceY; 8 GForceZ; 9 Lap; 10 GyroX; 11 GyroY; 12 GyroZ;
	getline(racebox, entry);
	/*while (getline(racebox, entry))
	{
		// cout << entry << endl;
		for (int i = 0; i < COLS; i++)
			isRead[i] = false;
		// reading data from entry:
		for (int i = 1; i < entry.length(); i++)
		{
			if (entry[i - 1] == ',' && !isRead[Time])
			{
				time = readTime(entry, &i);
				isRead[Time] = true;
			}
			if (entry[i - 1] == ',' && !isRead[Latitude])
			{
				latitude = readTime(entry, &i);
				isRead[Latitude] = true;
			}
			cout << time << '\t' << latitude << endl;
		}
	}*/


	while (getline(racebox, entry))
	{
		string field;
		stringstream ss(entry);

		int col = 0;
		while (getline(ss, field, ','))
		{
			if (col == Time) time = stod(field);
			if (col == Latitude) latitude = stod(field);
			if (col == Longitude) longitude = stod(field);
			if (col == Speed) speed = stoi(field);
			col++;
		}
		if (speed == 0)
		{
			time0 = time;
			lat0 = latitude;
			long0 = longitude;
		}
		// cout << fixed << setprecision(10) << time << '\t' << latitude << '\n';
	}

}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
