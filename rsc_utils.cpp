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


double interpolate(double x1, double y1, double x2, double y2, double x)
{
    return y1 + (y2 - y1) * (x - x1) / (x2 - x1);
}

static inline const char* const bool_to_string(bool b)
{
  return b ? "true" : "false";
}

vector<vector<float> > read_matfile(const char* filename) {
    vector<vector<float> > table;    
    // Load table from file
    ifstream file(filename);
    while (file)
    {
        string line;
        getline(file, line);
        istringstream is(line);
        vector<float> row;
        while (is)
        {
            float data;
            is >> data;
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


    




