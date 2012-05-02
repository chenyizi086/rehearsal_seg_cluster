#ifndef RSC_UTILS_H
#define RSC_UTILS_H

#include <vector>

using namespace std;

#define ALLOC(t, n) (t *) malloc(sizeof(t) * (n))
#define FREE(p) free(p)

#define ROUND(x) ((int) (0.5 + (x)))

double interpolate(double x1, double y1, double x2, double y2, double x);
const char* const bool_to_string(bool b);
vector<vector<float> > read_matfile(const char* filename);
int nextPowerOf2(int n);
string int2str(int a);
float vector_min(vector<float> vec, int *index);

#endif