#include "adaboost.h"
#include "rsc_utils.h"

Adaboost::Adaboost() {
}

Adaboost::~Adaboost() {
}

void Adaboost::load_classifier(const char *f_ada) {
    parameters = read_matfile(f_ada); 
}

void Adaboost::load_eigenmusic(const char *f_em) {
    eigen_music = read_matfile(f_em);
}

int Adaboost::do_prediction(vector<float> data) {
}
