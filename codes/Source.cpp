#include "Header.h"


int main() {
    int T = 24;
    int c_dg_1 = 80;
    int c_dg_2 = 90;
    float soc_initial = 0.2;
    int ess_max = 200;
    float ess_efficiency = 0.95;

    std::vector<int> p_load = { 169,175,179,171,181,172,270,264,273,281,193,158,161,162,250,260,267,271,284,167,128,134,144,150 };
    std::vector<int> c_grid_buy = { 90,90,90,90,90,90,110,110,110,110,110,125,125,125,125,125,125,125,110,110,110,110,110,110 };
    std::vector<int> c_grid_sell = { 70,70,70,70,70,70,90,90,90,90,90,105,105,105,105,105,105,105,90,90,90,90,90,90 };
    std::vector<float> p_rdg_1 = { 0,0,0,0,0,0,0,10,15,20,23,28,33,35,34,31,28,10,0,0,0,0,0,0 };
    std::vector<float> p_rdg_2 = { 0,0,0,0,0,0,0,10,15,20,23,28,33,35,34,31,28,10,0,0,0,0,0,0 };
    
    const string main_path = "D:\\MyCodesRepos\\multi-energy-microgrids-operations\\results\\electric_network";
    const string file_path = main_path + "\\electrical_network_results.csv";

    optimizeMicrogrid(T, c_dg_1, c_dg_2, p_load, c_grid_buy, c_grid_sell, p_rdg_1, p_rdg_2, soc_initial, ess_max, ess_efficiency, file_path);

    return 0;
}
