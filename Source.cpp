#include "Header.h"


int main() {
    int T = 24;
    int Cdg1 = 80;
    int Cdg2 = 90;
    float socini = 0.2;
    int Pbmax = 200;
    float effin = 0.95;

    std::vector<int> Pload = { 169,175,179,171,181,172,270,264,273,281,193,158,161,162,250,260,267,271,284,167,128,134,144,150 };
    std::vector<int> CGbuy = { 90,90,90,90,90,90,110,110,110,110,110,125,125,125,125,125,125,125,110,110,110,110,110,110 };
    std::vector<int> CGsell = { 70,70,70,70,70,70,90,90,90,90,90,105,105,105,105,105,105,105,90,90,90,90,90,90 };
    std::vector<float> Rdg1 = { 0,0,0,0,0,0,0,10,15,20,23,28,33,35,34,31,28,10,0,0,0,0,0,0 };
    std::vector<float> Rdg2 = { 0,0,0,0,0,0,0,10,15,20,23,28,33,35,34,31,28,10,0,0,0,0,0,0 };

    optimizeMicrogrid(T, Cdg1, Cdg2, Pload, CGbuy, CGsell, Rdg1, Rdg2, socini, Pbmax, effin,"electrical_network_results.csv");

    return 0;
}
