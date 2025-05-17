#include "Header.h"


// DGs data
DgData readDgDataFromCSV(const std::string& filename) {
    DgData dgData;
    std::ifstream file(filename);
    std::set<int> unique_ids;  // Track unique integer IDs

    if (!file.is_open()) {
        std::cerr << "Failed to open DG CSV file: " << filename << std::endl;
        return dgData;
    }

    std::string line;
    bool isHeader = true;

    while (std::getline(file, line)) {
        std::stringstream ss(line);
        std::string value;

        if (isHeader) {
            isHeader = false;
            continue; // skip header
        }

        std::vector<std::string> tokens;
        while (std::getline(ss, value, ',')) {
            tokens.push_back(value);
        }

        if (tokens.size() != 4) {
            std::cerr << "Invalid line in CSV (expected 4 columns): " << line << std::endl;
            continue;
        }

        try {
            int id_int = static_cast<int>(std::stod(tokens[0]));
            if (unique_ids.find(id_int) != unique_ids.end()) {
                std::cerr << "Duplicate DG ID found: " << id_int << " (skipping this entry)" << std::endl;
                continue;
            }

            // Valid unique ID, insert and store
            unique_ids.insert(id_int);
            dgData.id.push_back(id_int);
            dgData.cost_per_kwh.push_back(std::stod(tokens[1]));
            dgData.max_p_kw.push_back(std::stod(tokens[2]));
            dgData.min_p_kw.push_back(std::stod(tokens[3]));
        }
        catch (const std::invalid_argument& e) {
            std::cerr << "Conversion error in line: " << line << std::endl;
        }
    }

    file.close();
    return dgData;
}

// Prices data
ElectricityPrice readElectricityPriceFromCSV(const std::string& filename) {
    ElectricityPrice priceData;
    std::ifstream file(filename);

    if (!file.is_open()) {
        std::cerr << "Failed to open electricity price CSV file: " << filename << std::endl;
        return priceData;
    }

    std::string line;
    bool isHeader = true;

    while (std::getline(file, line)) {
        std::stringstream ss(line);
        std::string value;

        if (isHeader) {
            isHeader = false;
            continue; // skip header
        }

        std::vector<std::string> tokens;
        while (std::getline(ss, value, ',')) {
            tokens.push_back(value);
        }

        if (tokens.size() != 2) {
            std::cerr << "Invalid line in CSV (expected 2 columns): " << line << std::endl;
            continue;
        }

        try {
            priceData.buy_price.push_back(std::stod(tokens[0]));
            priceData.sell_price.push_back(std::stod(tokens[1]));
        }
        catch (const std::invalid_argument& e) {
            std::cerr << "Conversion error in line: " << line << std::endl;
        }
    }

    file.close();
    return priceData;
}

void printElectricityPriceTable(const ElectricityPrice& prices) {
    std::cout << "\nElectricity Price Schedule (24 Hours)\n";
    std::cout << "---------------------------------------------\n";
    std::cout << "| Hour | Ebuy_price (¢/kWh) | Esell_price (¢/kWh) |\n";
    std::cout << "---------------------------------------------\n";

    for (size_t i = 0; i < prices.buy_price.size(); ++i) {
        std::cout << "| "
            << std::setw(4) << i + 1 << " | "
            << std::setw(16) << std::fixed << std::setprecision(2) << prices.buy_price[i] << " | "
            << std::setw(18) << prices.sell_price[i] << " |\n";
    }

    std::cout << "---------------------------------------------\n";
    std::cout << "Total Hours: " << prices.buy_price.size() << "\n";
}

void printDgDataTable(const DgData& dg) {
    std::cout << "\nNumber of DGs: " << dg.id.size() << "\n";
    std::cout << std::fixed << std::setprecision(2);
    std::cout << "----------------------------------------------------------\n";
    std::cout << "| DG ID | Cost per kWh | Max Power (kW) | Min Power (kW) |\n";
    std::cout << "----------------------------------------------------------\n";

    for (size_t i = 0; i < dg.id.size(); ++i) {
        std::cout << "| "
            << std::setw(6) << dg.id[i] << " | "
            << std::setw(13) << dg.cost_per_kwh[i] << " | "
            << std::setw(14) << dg.max_p_kw[i] << " | "
            << std::setw(14) << dg.min_p_kw[i] << " |\n";
    }

    std::cout << "----------------------------------------------------------\n";
}


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
    const std::string& output_filename
) {
    auto start = std::chrono::high_resolution_clock::now();
    IloEnv env;

    try {
        IloModel model(env);

        // === Decision variables ===
        IloNumVarArray p_grid_buy(env, T, 0, IloInfinity);
        IloNumVarArray p_grid_sell(env, T, 0, IloInfinity);
        IloNumVarArray soc(env, T, 0, 1);
        IloNumVarArray p_ess_charge(env, T, 0, 100);
        IloNumVarArray p_ess_discharge(env, T, 0, 100);
        IloNumVarArray p_dg_1(env, T, 0, 80);
        IloNumVarArray p_dg_2(env, T, 0, 100);

        // === Objective Function ===
        IloExpr objective(env);
        for (int t = 0; t < T; t++) {
            objective += c_dg_1 * p_dg_1[t] +
                c_dg_2 * p_dg_2[t] +
                c_grid_buy[t] * p_grid_buy[t] -
                c_grid_sell[t] * p_grid_sell[t];
        }
        model.add(IloMinimize(env, objective));

        // === Constraints ===
        for (int t = 0; t < T; t++) {
            // State of charge bounds
            model.add(soc[t] >= 0);
            model.add(soc[t] <= 1);

            // SoC update equations & ESS power bounds
            if (t == 0) {
                model.add(soc[t] == soc_initial +
                    ((ess_efficiency * p_ess_charge[t] - p_ess_discharge[t] / ess_efficiency) / ess_max));
                model.add(p_ess_charge[t] <= (ess_max * (1 - soc_initial) / ess_efficiency));
                model.add(p_ess_discharge[t] <= (ess_max * soc_initial * ess_efficiency));
            }
            else {
                model.add(soc[t] == soc[t - 1] +
                    ((ess_efficiency * p_ess_charge[t] - p_ess_discharge[t] / ess_efficiency) / ess_max));
                model.add(p_ess_charge[t] <= (ess_max * (1 - soc[t - 1])) / ess_efficiency);
                model.add(p_ess_discharge[t] <= ess_max * soc[t - 1] * ess_efficiency);
            }

            // Power balance constraint
            model.add(p_dg_1[t] + p_dg_2[t] +
                p_rdg_1[t] + p_rdg_2[t] +
                p_ess_discharge[t] - p_ess_charge[t] +
                p_grid_buy[t] - p_grid_sell[t] == p_load[t]);
        }

        // === Solve the optimization model ===
        IloCplex cplex(model);
        cplex.setOut(env.getNullStream());  // Suppress solver output

        if (!cplex.solve()) {
            std::cerr << "Optimization failed." << std::endl;
            env.end();
            return;
        }

        double obj = cplex.getObjValue();
        auto end = std::chrono::high_resolution_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

        std::cout << "\n\tElapsed time: " << elapsed.count() << " ms" << std::endl;
        std::cout << "Solution status: " << cplex.getStatus() << std::endl;
        std::cout << "Minimized Objective Function: " << obj << std::endl;

        // === Save results to file ===
        std::ofstream outputFile(output_filename);
        if (outputFile.is_open()) {
            // LaTeX-ready CSV header
            outputFile << "t,${P^{load}}$,${c^{buy}}$,${c^{sell}}$,${P^{rdg1}}$,${P^{rdg2}}$,${P^{buy}_t}$,${P^{sell}_t}$,"
                "${SoC_t}$,${P^{chg}_t}$,${P^{dis}_t}$,${P^{dg1}_t}$,${P^{dg2}_t}$\n";

            for (int t = 0; t < T; t++) {
                outputFile << t + 1 << ","                            // time
                    << p_load[t] << ","                        // load
                    << c_grid_buy[t] << ","                    // buy price
                    << c_grid_sell[t] << ","                   // sell price
                    << p_rdg_1[t] << ","                       // RDG1
                    << p_rdg_2[t] << ","                       // RDG2
                    << cplex.getValue(p_grid_buy[t]) << ","   // grid buy
                    << cplex.getValue(p_grid_sell[t]) << ","  // grid sell
                    << cplex.getValue(soc[t]) << ","          // SoC
                    << cplex.getValue(p_ess_charge[t]) << "," // battery charge
                    << cplex.getValue(p_ess_discharge[t]) << "," // battery discharge
                    << cplex.getValue(p_dg_1[t]) << ","       // DG1
                    << cplex.getValue(p_dg_2[t]) << "\n";     // DG2
            }

            outputFile.close();
            std::cout << "Output saved to " << output_filename << std::endl;
        }
        else {
            std::cerr << "Error writing to file: " << output_filename << std::endl;
        }
    }
    catch (IloException& e) {
        std::cerr << "Cplex exception: " << e.getMessage() << std::endl;
    }
    catch (...) {
        std::cerr << "Unknown exception occurred." << std::endl;
    }

    env.end();
}
