#include "TFile.h"
#include "TCanvas.h"
#include "TH1F.h"
#include "TF1.h"
#include "TLine.h"
#include "TGraph.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <map>
#include <cmath>

// Structure to hold metadata for each source
struct SourceInfo {
    double activity_kBq;
    double branching_ratio;
    double gamma_energy_keV;
};

void calculateEfficiency() {
    int run_number = 4320;
    std::string input_txt = "/felsdata/n14ag_Felsenkeller_2025/Anup/Output_" + std::to_string(run_number) + "/PeakCounts_v6_Run" + std::to_string(run_number) + ".txt";

    std::ifstream inFile(input_txt);
    if (!inFile.is_open()) {
        std::cerr << "Error opening file: " << input_txt << std::endl;
        return;
    }

    std::map<std::string, SourceInfo> source_meta;

    // Fill source metadata (Gamma energy is approx. to help label the plot)
    source_meta["137CS"] = {7.09, 0.85, 661.7};
    source_meta["60Co"] = {18.22, 1.00, 1173.2}; // Second Co-60 line at 1332.5 can be added
    source_meta["88Y_93"] = {3.35, 0.93, 898.0};
    source_meta["88Y_99"] = {3.35, 0.99, 1836.0};
    source_meta["133Ba"] = {9.52, 0.6205, 356.0};

    std::map<std::string, double> net_counts_per_source;
    std::string line;
    while (getline(inFile, line)) {
        std::stringstream ss(line);
        std::string detector;
        double net_counts, gross_counts, peak_pos, fwhm;
        ss >> detector >> net_counts >> gross_counts >> peak_pos >> fwhm;

        if (peak_pos > 640 && peak_pos < 680) {
            net_counts_per_source["137CS"] += net_counts;
        } else if (peak_pos > 1160 && peak_pos < 1185) {
            net_counts_per_source["60Co"] += net_counts;
        } else if (peak_pos > 880 && peak_pos < 910) {
            net_counts_per_source["88Y_93"] += net_counts;
        } else if (peak_pos > 1800 && peak_pos < 1860) {
            net_counts_per_source["88Y_99"] += net_counts;
        } else if (peak_pos > 350 && peak_pos < 365) {
            net_counts_per_source["133Ba"] += net_counts;
        }
    }
    inFile.close();

    std::vector<double> energies, efficiencies, eff_errors;

    for (const auto& entry : net_counts_per_source) {
        std::string source = entry.first;
        double counts = entry.second;

        SourceInfo info = source_meta[source];

        double t_live = 427.0; // Replace with actual live time in seconds
        // Calculate efficiency

        double eff = counts / (info.activity_kBq * 1000 * info.branching_ratio * t_live); // Convert activity to Bq
        double err = sqrt(counts) / (info.activity_kBq * 1000 * info.branching_ratio * t_live);

        energies.push_back(info.gamma_energy_keV);
        efficiencies.push_back(eff);
        eff_errors.push_back(err);
        
        // Print results
        std::cout << "Run Number: " << run_number << std::endl;
        std::cout << "Source: " << source
                  << " | E_gamma: " << info.gamma_energy_keV
                  << " keV | Efficiency: " << eff
                  << " | Error: " << err << std::endl;


        std::cout << "Source: " << source
                  << " | E_gamma: " << info.gamma_energy_keV
                  << " keV | Efficiency: " << eff
                  << " | Error: " << err << std::endl;
    }

    // Plot efficiency curve
    TCanvas* c = new TCanvas("c_eff", "Efficiency Curve", 800, 600);
    TGraphErrors* gEff = new TGraphErrors(energies.size(), &energies[0], &efficiencies[0], 0, &eff_errors[0]);
    gEff->SetTitle("Absolute Efficiency Curve;Gamma Energy [keV];Efficiency");
    gEff->SetMarkerStyle(20);
    gEff->Draw("AP");

    c->SaveAs("efficiency_curve_4320.png");
}
