#pragma once
#include <ilcplex/ilocplex.h>
#include <chrono>
#include <iostream>
#include <fstream>
#include <vector>

ILOSTLBEGIN


void optimizeMicrogrid(
        int T,
        int c_dg_1,
        int c_dg_2,
        const std::vector<int>& p_load,
        const std::vector<int>& c_grid_buy,
        const std::vector<int>& c_grid_sell,
        const std::vector<float>& p_rdg_1,
        const std::vector<float>& p_rdg_2,
        float soc_initial,
        int ess_max,
        float ess_efficiency,
        const std::string& output_filename);


