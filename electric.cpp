#include "Header.h"

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
    const std::string& output_filename
) {
    auto start = std::chrono::high_resolution_clock::now();
    IloEnv env;
    try {
        IloModel model(env);

        // Decision variables
        IloNumVarArray PGbuy(env, T, 0, IloInfinity);
        IloNumVarArray PGsell(env, T, 0, IloInfinity);
        IloNumVarArray statoc(env, T, 0, 1);
        IloNumVarArray Bchg(env, T, 0, 100);
        IloNumVarArray Bdischg(env, T, 0, 100);
        IloNumVarArray Pdg1(env, T, 0, 80);
        IloNumVarArray Pdg2(env, T, 0, 100);

        // Objective function
        IloExpr objective(env);
        for (int t = 0; t < T; t++) {
            objective += Cdg1 * Pdg1[t] + Cdg2 * Pdg2[t] + CGbuy[t] * PGbuy[t] - CGsell[t] * PGsell[t];
        }
        model.add(IloMinimize(env, objective));

        // Constraints
        for (int t = 0; t < T; t++) {
            model.add(0 <= statoc[t]);
            model.add(statoc[t] <= 1);

            if (t == 0) {
                model.add(statoc[t] == socini + ((effin * Bchg[t] - (Bdischg[t] / effin)) / Pbmax));
                model.add(Bchg[t] <= (Pbmax * (1 - socini) / effin));
                model.add(Bdischg[t] <= (Pbmax * socini * effin));
            }
            else {
                model.add(statoc[t] == statoc[t - 1] + ((effin * Bchg[t] - (Bdischg[t] / effin)) / Pbmax));
                model.add(Bchg[t] <= (Pbmax * (1 - statoc[t - 1])) / effin);
                model.add(Bdischg[t] <= Pbmax * statoc[t - 1] * effin);
            }

            model.add(Pdg1[t] + Pdg2[t] + Rdg1[t] + Rdg2[t] + Bdischg[t] - Bchg[t] + PGbuy[t] - PGsell[t] == Pload[t]);
        }

        // Solve
        IloCplex cplex(model);
        cplex.setOut(env.getNullStream());
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

        // Save results
        std::ofstream outputFile(output_filename);
        if (outputFile.is_open()) {
            outputFile << "Time,Pload,CGbuy,CGsell,Rdg1,Rdg2,PGbuy,PGsell,statoc,Bchg,Bdischg,Pdg1,Pdg2\n";
            for (int t = 0; t < T; t++) {
                outputFile << t + 1 << "," << Pload[t] << "," << CGbuy[t] << "," << CGsell[t] << ","
                    << Rdg1[t] << "," << Rdg2[t] << "," << cplex.getValue(PGbuy[t]) << ","
                    << -cplex.getValue(PGsell[t]) << "," << cplex.getValue(statoc[t]) << ","
                    << -cplex.getValue(Bchg[t]) << "," << cplex.getValue(Bdischg[t]) << ","
                    << cplex.getValue(Pdg1[t]) << "," << cplex.getValue(Pdg2[t]) << "\n";
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
