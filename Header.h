#pragma once
#include <ilcplex/ilocplex.h>
#include <chrono>
#include <iostream>
#include <fstream>
#include <vector>

ILOSTLBEGIN


void optimizeMicrogrid(
    int T,
    int Cdg1,
    int Cdg2,
    const std::vector<int>& Pload,
    const std::vector<int>& CGbuy,
    const std::vector<int>& CGsell,
    const std::vector<float>& Rdg1,
    const std::vector<float>& Rdg2,
    float socini,
    int Pbmax,
    float effin,
    const std::string& output_filename);