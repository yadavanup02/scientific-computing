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
    double t_live; // Live time in seconds
};

// Map of run number to source label
std::map<int, std::vector<std::string>> run_sources = {
    {4329, {"137CS"}},
    {4320, {"60Co"}},
    {4332, {"88Y_93"}},
    {4331, {"88Y_99"}},
    {4335, {"133Ba"}}
};

void calculateEfficiency_v1() {
    std::map<std::string, SourceInfo> source_meta = {
        {"137CS", {7.09, 0.85, 661.7, 4167.0}},
        {"60Co",  {18.22, 1.00, 1332.49, 57652.0}},  // You can add 1332.5 separately if needed
        {"88Y_93", {3.35, 0.93, 898.0, 4177.0}},
        {"88Y_99", {3.35, 0.99, 1836.0, 4966.0}},
        {"133Ba", {9.52, 0.6205, 356.0, 2473.0}}
    };

    std::vector<double> energies, efficiencies, eff_errors;

    for (const auto& run_entry : run_sources) {
        int run_number = run_entry.first;
        std::vector<std::string> sources = run_entry.second;

        std::string input_txt = "/felsdata/n14ag_Felsenkeller_2025/Anup/Output_" + std::to_string(run_number) + "/PeakCounts_v6_Run" + std::to_string(run_number) + ".txt";
        std::ifstream inFile(input_txt);
        if (!inFile.is_open()) {
            std::cerr << "Error opening file: " << input_txt << std::endl;
            continue;
        }

        std::string line;
        std::map<std::string, double> counts;

        while (getline(inFile, line)) {
            std::stringstream ss(line);
            std::string detector;
            double gross, net, peakE, fwhm;
            ss >> detector >> gross >> net >> peakE >> fwhm;

            if (peakE > 650 && peakE < 675) counts["137CS"] += net;
            if (peakE > 1300 && peakE < 1395) counts["60Co"] += net;
            if (peakE > 880 && peakE < 910) counts["88Y_93"] += net;
            if (peakE > 1800 && peakE < 1870) counts["88Y_99"] += net;
            if (peakE > 350 && peakE < 365) counts["133Ba"] += net;
        }
        inFile.close();

        for (const auto& s : sources) {
            double net = counts[s];
            const auto& info = source_meta[s];
            double t_live = info.t_live; 

            double eff = net / (info.activity_kBq * 1000. * info.branching_ratio * t_live);
            double err = sqrt(net) / (info.activity_kBq * 1000. * info.branching_ratio * t_live);

            energies.push_back(info.gamma_energy_keV);
            efficiencies.push_back(eff);
            eff_errors.push_back(err);

            std::cout << "Run: " << run_number << " | Source: " << s
                      << " | E_gamma: " << info.gamma_energy_keV
                      << " keV | Efficiency: " << eff
                      << " | Error: " << err << std::endl;
        }
    }

    // Plotting
    TCanvas* c = new TCanvas("c_eff", "Efficiency Curve", 800, 600);
    TGraphErrors* gEff = new TGraphErrors(energies.size(), &energies[0], &efficiencies[0], 0, &eff_errors[0]);
    gEff->SetTitle("Absolute Efficiency Curve;Gamma Energy [keV];Efficiency");
    gEff->SetMarkerStyle(20);
    gEff->SetMarkerColor(kBlue);
    gEff->Draw("AP");

    c->SetGrid();
    c->SetLogx();
    c->SaveAs("efficiency_curve_v1.png");
}
