#include "TLine.h"
#include "TMarker.h"
#include "TGraph.h"
#include "TGraphErrors.h"
#include "TRandom.h"
#include "TStyle.h"
#include "TCanvas.h"
#include "TF1.h"
#include "TArrow.h"
#include "TLatex.h"
#include "TString.h"
#include "stdio.h"
#include "iostream"
#include "fstream"
#include <vector>
#include <sstream>
#include <map>

struct EfficiencyData {
    double eff;
    double effErr;
};

void resonanceStrength_v2(int start_run, int end_run) { 
    // ================== Load efficiency file ==================
    const char* eff_path = "/felsdata/n14ag_Felsenkeller_2025/Anup/simEfficiency_2775.txt";
    std::ifstream efficiency_file(eff_path);
    std::map<std::string, EfficiencyData> effMap;

    if (!efficiency_file.is_open()) {
        std::cerr << "Error opening efficiency file: " << eff_path << std::endl;
        return;
    }

    std::string line;
    while (std::getline(efficiency_file, line)) {
        if (line.find("Detector") != std::string::npos) continue; // skip header
        std::stringstream ss(line);
        std::string det;
        double peakE, eff, effErr;
        if (ss >> det >> peakE >> eff >> effErr) {
            effMap[det] = {eff, effErr};
        }
    }
    efficiency_file.close();


    // ================== Loop over run numbers ==================
    for (int run_number = start_run; run_number <= end_run; run_number++) {
        TString input_file = TString::Format("/felsdata/n14ag_Felsenkeller_2025/Anup/Run%d/Peak1_LcLu_Run%d.txt", run_number, run_number);
        std::ifstream infile(input_file.Data());

        if (!infile.is_open()) {
            std::cerr << "Error opening file: " << input_file.Data() << std::endl;
            continue;
        }

        std::vector<double> resonance_strength, error;
        std::vector<std::string> used_detectors;

        while (std::getline(infile, line)) {
            if (line.find("Detector") != std::string::npos || 
                line.find("Run Number") != std::string::npos ||  
                line.find("---") != std::string::npos) {
                continue;
            }

            std::stringstream ss(line);
            std::string detector;
            double peakE, gross, net, netErr, L_u, L_c;
            if (ss >> detector >> peakE >> gross >> net >> netErr) {
                if (effMap.find(detector) == effMap.end()) {
                    std::cerr << "Warning: No efficiency found for detector " << detector << std::endl;
                    continue;
                }

                double N_max = net;
                double N_err = netErr;
                double neta = effMap[detector].eff;
                double neta_err = effMap[detector].effErr;

                // ==== constants ====
                double Branching_ratio = 0.783;
                double W = 1.0;                          
                double epsilon = 43.569063e-15;   // eV cm^2/atoms
                double M_14N = 14.003074;
                double M_4He = 4.002603;
                double COM_factor = M_14N / (M_14N + M_4He);
                double E_r = 1136.0 * 1e3;       // eV
                double I = 24.874e-6;            // beam current in A
                double live_time = 50465.31;     // seconds    

                // effective stopping power
                double epsilon_effective = COM_factor * epsilon * (1 + 0.3663 / 99.6337);

                // de Broglie wavelength
                double lambda_sqr_by2 = std::pow((1.0 / COM_factor), 2) * 4.125e-18 / (M_4He * E_r);

                // total alphas
                double N_alpha = I * live_time / (1.0 * 1.6e-19);

                // resonance strength
                double wg = (epsilon_effective / (lambda_sqr_by2)) * (N_max / (N_alpha * Branching_ratio * neta * W));
                double wg_err = wg * std::sqrt( std::pow((N_err / N_max), 2) 
                                              + std::pow((neta_err / neta), 2) 
                                              + std::pow(0.01, 2)); // 1% error current

                resonance_strength.push_back(wg);
                error.push_back(wg_err);
                used_detectors.push_back(detector);
            }
        }
        infile.close();

        // ========== Print and Save Results ==========
        for (size_t i = 0; i < resonance_strength.size(); ++i) {
            std::cout << "Run " << run_number 
                      << " | Detector: " << used_detectors[i] 
                      << " | Resonance Strength: " << resonance_strength[i] 
                      << " ± " << error[i] << std::endl;
        }

        // Ensure output directory exists
        TString outDir = TString::Format("/felsdata/n14ag_Felsenkeller_2025/Anup/Resonance_1136keV/Run%d", run_number);
        std::string mkdir_cmd = "mkdir -p " + std::string(outDir.Data());
        system(mkdir_cmd.c_str());

        std::ofstream output_file(TString::Format("%s/Resonance_Strength_v2.txt", outDir.Data()).Data(), std::ios::trunc);
        if (output_file.is_open()) {
            output_file << "Detector\tResonance_Strength(meV)\tError (meV)\n";
            for (size_t i = 0; i < resonance_strength.size(); ++i) {
                output_file << std::fixed << std::setprecision(3);
                output_file << used_detectors[i] << "\t" 
                            << resonance_strength[i] * 1000 << "\t" 
                            << error[i] * 1000 << std::endl;
            }
            output_file.close();
        } else {
            std::cerr << "Error opening output file!" << std::endl;
        }
    }

    std::cout << "Done. Press ENTER to exit..." << std::endl;
    std::cin.ignore();
}
