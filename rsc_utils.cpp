/*
 *  sautils.cpp
 *  scorealign
 *
 *  Created by Roger Dannenberg on 10/20/07.
 *  Copyright 2007 __MyCompanyName__. All rights reserved.
 *
 */
#include <fstream>;
#include <iostream>;
#include <sstream>;
#include "rsc_utils.h"
#include <string>


double interpolate(double x1, double y1, double x2, double y2, double x)
{
    return y1 + (y2 - y1) * (x - x1) / (x2 - x1);
}

vector<vector<float> > read_matfile(const char* filename) {
    vector<vector<float> > table; 
    string line;
    // Load table from file
    ifstream file(filename);
    while (getline(file, line))
    {
        float data;
        istringstream is(line);
        vector<float> row;
        while (is >> data)
        {
            row.push_back(data);
        }
        table.push_back(row);
    }
    return table;
}

int nextPowerOf2(int n)
{
    int result = 1;
    while (result < n) 
		result = (result << 1);
    return result;
}

string int2str(int a) {
    stringstream ss;
    ss << a;
    return ss.str();
}

float vector_min(vector<float> vec, int *index) {
    float v = vec[0];
    *index = 0;
    for (int i = 1; i < vec.size(); i++) {
        if (vec[i] < v) {
            v = vec[i];
            *index = i;
        }
    }
    return v;
}




    




