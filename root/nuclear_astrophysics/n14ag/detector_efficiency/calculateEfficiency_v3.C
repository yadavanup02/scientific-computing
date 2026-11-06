#include "TFile.h"
#include "TCanvas.h"
#include "TH1F.h"
#include "TF1.h"
#include "TLine.h"
#include "TGraph.h"
#include "TLegend.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <map>
#include <vector>
#include <cmath>
#include <sys/stat.h>

// Structure to hold metadata for each source
struct SourceInfo {
    double activity_kBq;
    double branching_ratio;
    double gamma_energy_keV;
    double t_live; // Live time in seconds
};

// mix angle 
// std::map<int, std::vector<std::string>> run_sources = {
//     {4329, {"137CS"}},
//     {4320, {"60Co"}},
//     {4332, {"88Y_93"}},
//     {4331, {"88Y_99"}},
//     {4335, {"133Ba"}}
// };
// +20 degree angle for the source
// std::map<int, std::vector<std::string>> run_sources = {
//     {4329, {"137CS"}},
//     {4328, {"60Co"}},
//     {4231, {"88Y_93"}},
//     {4331, {"88Y_99"}},
//     {4340, {"133Ba"}}
// };
// -20 degree angle for the source
std::map<int, std::vector<std::string>> run_sources = {
    {4330, {"137CS"}},
    {4320, {"60Co"}},
    {4233, {"88Y_93"}},
    {4333, {"88Y_99"}},
    {4338, {"133Ba"}}
};
std::vector<int> darkColors = {
    kBlue+1,   // Dark blue
    kRed+1,    // Dark red
    kGreen+2,  // Dark green
    kMagenta+2,
    kCyan+2,
    kOrange+7, // Deeper orange
    kViolet+1,
    kTeal+3,
    kPink+9,
    kAzure+3,
    kSpring+3,
    kGray+2,
    kBlack     
};


void calculateEfficiency_v3() {
    // mix angele for the source
    // std::map<std::string, SourceInfo> source_meta = {
    //     {"137CS", {7.09, 0.85, 661.7, 4167.0}},
    //     {"60Co",  {18.22, 1.00, 1332.49, 57652.0}},
    //     {"88Y_93", {3.35, 0.93, 898.0, 4177.0}},
    //     {"88Y_99", {3.35, 0.99, 1836.0, 4966.0}},
    //     {"133Ba", {9.52, 0.6205, 356.0, 2473.0}}
    // };
    // -20 degree angle for the source 
    std::map<std::string, SourceInfo> source_meta = {
        {"137CS", {7.09, 0.85, 661.7, 4966.0}},
        {"60Co",  {18.22, 1.00, 1332.49, 57652.0}},
        {"88Y_93", {3.35, 0.93, 898.0, 4310.0}},
        {"88Y_99", {3.35, 0.99, 1836.0, 4310.0}},
        {"133Ba", {9.52, 0.6205, 356.0, 1399.0}}
    };
    // +20 degree angle for the source 
    // std::map<std::string, SourceInfo> source_meta = {
    //     {"137CS", {7.09, 0.85, 661.7, 4167.0}},
    //     {"60Co",  {18.22, 1.00, 1332.49, 952.0}},
    //     {"88Y_93", {3.35, 0.93, 898.0, 4177.0}},
    //     {"88Y_99", {3.35, 0.99, 1836.0, 4177.0}},
    //     {"133Ba", {9.52, 0.6205, 356.0, 6971.0}}
    // };

    std::map<std::string, std::vector<double>> detector_energies;
    std::map<std::string, std::vector<double>> detector_efficiencies;
    std::map<std::string, std::vector<double>> detector_errors;

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
        while (getline(inFile, line)) {
            if (line.find("Detector") != std::string::npos || line.find("Run Number") != std::string::npos || line.find("---") != std::string::npos) continue;

            std::stringstream ss(line);
            std::string detector;
            double gross, net, peakE, fwhm;
            ss >> detector >> gross >> net >> peakE >> fwhm;

            for (const auto& s : sources) {
                const auto& info = source_meta[s];

                bool in_range = false;
                if (s == "137CS" && peakE > 600 && peakE < 700) in_range = true;
                if (s == "60Co" && peakE > 1300 && peakE < 1395) in_range = true;
                if (s == "88Y_93" && peakE > 830 && peakE < 950) in_range = true;
                if (s == "88Y_99" && peakE > 1800 && peakE < 1900) in_range = true;
                if (s == "133Ba" && peakE > 310 && peakE < 390) in_range = true;

                if (!in_range) continue;

                double eff = net / (info.activity_kBq * 1000. * info.branching_ratio * info.t_live);
                double err = sqrt(net) / (info.activity_kBq * 1000. * info.branching_ratio * info.t_live);

                detector_energies[detector].push_back(info.gamma_energy_keV);
                detector_efficiencies[detector].push_back(eff);
                detector_errors[detector].push_back(err);

                std::cout << "Run: " << run_number << " | Detector: " << detector << " | Source: " << s
                          << " | E_gamma: " << info.gamma_energy_keV
                          << " keV | Efficiency: " << eff
                          << " | Error: " << err << std::endl;
            }
        }
        inFile.close();
    }

    TCanvas* c = new TCanvas("c_eff_all", "Efficiency Curves - All Detectors", 1000, 800);

    TLegend* legend = new TLegend(0.79, 0.6, 0.89, 0.89);
    int colorIndex = 0;

    for (const auto& entry : detector_energies) {
        const std::string& detector = entry.first;
        const std::vector<double>& energies = entry.second;
        const std::vector<double>& efficiencies = detector_efficiencies[detector];
        const std::vector<double>& errors = detector_errors[detector];

        int color = darkColors[colorIndex % darkColors.size()];
        colorIndex++;

        TGraphErrors* g = new TGraphErrors(energies.size(), &energies[0], &efficiencies[0], 0, &errors[0]);
        g->SetMarkerStyle(20);
        g->SetMarkerColor(color);
        g->SetLineWidth(3);
        g->SetLineColor(color);
        g->Draw(colorIndex == 1 ? "AP" : "P SAME");
       // g->SetTitle("Absolute Efficiency Curve" );
        g->GetXaxis()->SetTitle("Energy E_{#gamma} [keV]");
        g->GetYaxis()->SetTitle("#varepsilon_{#gamma}");
        g->SetTitle("");
        g->GetXaxis()->SetTitleSize(0.03);
        g->GetYaxis()->SetTitleSize(0.04);
        g->GetXaxis()->SetLabelSize(0.03);
        g->GetYaxis()->SetLabelSize(0.03);

        // Set x and y range
        g->GetXaxis()->SetLimits(300, 2200);
        g->SetMinimum(1e-3);                
        g->SetMaximum(0.02);               
        g->GetXaxis()->SetMoreLogLabels(kTRUE); 
        g->GetXaxis()->SetNoExponent(kTRUE);   
        // g->GetYaxis()->SetMoreLogLabels(kTRUE); 
        // g->GetYaxis()->SetNoExponent(kTRUE);   

        // Enable grid and set x-ticks on every grid line
        c->SetGrid();
        // g->GetXaxis()->SetNdivisions(810);  
        // // Replace default x-axis with a custom linear axis (overlay)
        // TGaxis* customX = new TGaxis(300, c->GetUymin(), 2200, c->GetUymin(),
        //                              300, 2200, 10, "-L");
        // // customX->SetLabelSize(0.03);
        // // customX->SetTitle("Energy E_{#gamma} [keV]");
        // // customX->SetTitleSize(0.04);
        // customX->Draw();

        // // Set vertical grid labels
        // TGaxis* yAxis = new TGaxis(300, 1e-3, 300, 0.02, 1e-3, 0.02, 510, "G");
        // yAxis->SetLabelSize(0.04);
        // yAxis->SetTitle("#varepsilon_{#gamma}");
        // yAxis->SetTitleSize(0.05);
        // yAxis->Draw();

        // Log-polynomial fit: log(eff) = a0 + a1*log(E) + a2*log(E)^2 + a3*log(E)^3
        // TF1* fit = new TF1(("fit_" + detector).c_str(), "exp([0] + [1]*log(x) + [2]*pow(log(x),2) + [3]*pow(log(x),3))", 330, 2000);
        TF1* fit = new TF1(("fit_" + detector).c_str(), "exp([0] + [1]*log(x) + [2]*pow(log(x),2))", 330, 2000);

        fit->SetLineColor(color);
        g->Fit(fit, "QN");
        // Calculate efficiency and uncertainty at specific energy points
        // TF1* fit = new TF1(("fit_" + detector).c_str(), "exp([0] + [1]*log(x/a0) + [2]*pow(log(x/a0),2))", 330, 2000);
        // fit->SetLineColor(color);
        // g->Fit(fit, "QN");

        // Calculate efficiency and uncertainty at specific energy points
        std::vector<double> energy_points = {356.0, 661.7, 898.0, 1173.2, 1332.5, 1836.0};

        for (double E : energy_points) {
            // Get efficiency from fit
            double eff = fit->Eval(E);
            
            // Get fit parameters and covariance matrix
            double p0 = fit->GetParameter(0);
            double p1 = fit->GetParameter(1);
            double p2 = fit->GetParameter(2);

            // Get all information from the fit
            double cov00 = fit->GetCovarianceMatrixElement(0,0);
            double cov01 = fit->GetCovarianceMatrixElement(0,1);
            double cov02 = fit->GetCovarianceMatrixElement(0,2);
            double cov11 = fit->GetCovarianceMatrixElement(1,1);
            double cov12 = fit->GetCovarianceMatrixElement(1,2);
            double cov22 = fit->GetCovarianceMatrixElement(2,2);
            
            // Calculate partial derivatives
            double logE = log(E);
            double dEff_dp0 = eff;  // ∂eff/∂p0 = eff
            double dEff_dp1 = eff * logE;  // ∂eff/∂p1 = eff*log(E)
            double dEff_dp2 = eff * pow(logE,2);  // ∂eff/∂p2 = eff*log²(E)
            
            // Calculate uncertainty using error propagation
            double variance = 
                pow(dEff_dp0,2)*cov00 + 
                pow(dEff_dp1,2)*cov11 + 
                pow(dEff_dp2,2)*cov22 +
                2*dEff_dp0*dEff_dp1*cov01 +
                2*dEff_dp0*dEff_dp2*cov02 +
                2*dEff_dp1*dEff_dp2*cov12;
            
            double eff_error = sqrt(variance);
            
            std::cout << "Detector: " << detector 
                    << " | Energy: " << E << " keV"
                    << " | Efficiency: " << eff 
                    << " ± " << eff_error 
                    << " (" << (eff_error/eff)*100 << "%)" << std::endl;
        }

        // Save results to a file
        std::ofstream effFile(output_dir + "/efficiency_results_" + detector + ".txt");
        effFile << "Energy [keV]\tEfficiency\tUncertainty\n";
        for (double E : energy_points) {
            double eff = fit->Eval(E);
            // [Same error calculation as above...]
            effFile << E << "\t" << eff << "\t" << eff_error << "\n";
        }
        effFile.close();
        fit->Draw("SAME");
        legend->AddEntry(g, detector.c_str(), "p");

        color++;
        if (color == 10) color++;
    }
    std::string output_dir = "/felsdata/n14ag_Felsenkeller_2025/Anup/Efficiency";
    struct stat info;
    if (stat(output_dir.c_str(), &info) != 0 || !(info.st_mode & S_IFDIR)) {
        if (mkdir(output_dir.c_str(), 0777) != 0) {
            std::cerr << "Error creating directory: " << output_dir << std::endl;
            return;
        }
    }
    legend->Draw();
    //c->SetGrid();
    c->SetLogx();
    c->SetLogy();
    c->SaveAs((output_dir + "/eff_at_1173keV.pdf").c_str());
}
