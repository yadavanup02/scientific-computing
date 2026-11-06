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
#include "TMatrixDSym.h"
#include "Math/Util.h"

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
double calculate_uncertainty(TF1* fit, TFitResultPtr fitResult, double x) {
    if (!fitResult.Get()) {
        std::cerr << "Error: No fit result available!" << std::endl;
        return 0.0;
    }
    
    // Get covariance matrix
    TMatrixDSym covMatrix = fitResult->GetCovarianceMatrix();
    // Calculate derivatives
    TVectorD derivatives(fit->GetNpar());
    for (int i = 0; i < fit->GetNpar(); i++) {
        derivatives[i] = fit->GradientPar(i, &x);
    }
    
    return sqrt(covMatrix.Similarity(derivatives));
}

void plotEfficiency_v1() {
    std::map<std::string, std::vector<double>> detector_energies;
    std::map<std::string, std::vector<double>> detector_efficiencies;
    std::map<std::string, std::vector<double>> detector_errors;

    std::string input_file = "/felsdata/n14ag_Felsenkeller_2025/Anup/Efficiency/efficiency_all_v1.txt";
    std::ifstream inFile(input_file);
    if (!inFile.is_open()) {
        std::cerr << "Error opening file: " << input_file << std::endl;
        return;
    }

    std::string line;
    // Skip header
    std::getline(inFile, line);

    while (std::getline(inFile, line)) {
        if (line.empty()) continue;
        std::stringstream ss(line);
        std::string detector;
        double energy, efficiency, error;
        ss >> detector >> energy >> efficiency >> error;
        detector_energies[detector].push_back(energy);
        detector_efficiencies[detector].push_back(efficiency);
        detector_errors[detector].push_back(error);
    }
    inFile.close();

    TCanvas* c = new TCanvas("c_eff_all", "Efficiency Curves - All Detectors", 1000, 800);
    TLegend* legend = new TLegend(0.79, 0.6, 0.89, 0.89);
    int colorIndex = 0;

    std::string output_dir = "/felsdata/n14ag_Felsenkeller_2025/Anup/Efficiency/";

    std::string output_file1 = output_dir + "minus20degree_2774keV.txt";
    std::ofstream outFile1(output_file1);
    if (!outFile1.is_open()) {
        std::cerr << "Error opening output file: " << output_file1 << std::endl;
        return;
    }
    outFile1 << "Detector\tE_value (keV)\tEfficiency\tError" << std::endl;

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
        g->GetXaxis()->SetTitle("Energy E_{#gamma} [keV]");
        g->GetYaxis()->SetTitle("#varepsilon_{#gamma}");
        g->SetTitle("");
        g->GetXaxis()->SetTitleSize(0.03);
        g->GetYaxis()->SetTitleSize(0.04);
        g->GetXaxis()->SetLabelSize(0.03);
        g->GetYaxis()->SetLabelSize(0.03);

        //g->GetXaxis()->SetLimits(300, 7200);
        g->SetMinimum(1.2e-4);                
        g->SetMaximum(0.02);              
        g->GetXaxis()->SetMoreLogLabels(kTRUE);
        g->GetXaxis()->SetNoExponent(kTRUE);
        g->GetXaxis()->SetNdivisions(20, 10, 0, kTRUE); // 15 major, 10 minor per major, optimize
        g->GetXaxis()->SetLimits(300, 8000); // Explicitly set axis limits
        g->GetHistogram()->GetXaxis()->SetRangeUser(300, 8000); // Ensure range is set for TGraph
        gStyle->SetStatW(0.0);
        gStyle->SetStatH(0.0);
        c->SetGrid();

        // Fit as before
        TF1* fit = new TF1(("fit_" + detector).c_str(), "exp([0] + [1]*log(x) + [2]*pow(log(x),2) + [3]*pow(log(x),3))", 300, 7000);
        fit->SetLineColor(color);
        TFitResultPtr fitResult = g->Fit(fit, "ERQS");
        fit->Draw("SAME");
        legend->AddEntry(g, detector.c_str(), "p");
        // efficiency and uncertainty at specific energy points
        std::vector<double> test_energies = {2774.0};
        for (double energy : test_energies) {
            double eff = fit->Eval(energy);
            double unc = calculate_uncertainty(fit, fitResult, energy);
            std::cout << detector << "|" << energy << " keV | Efficiency = " << eff 
                    << " ± " << unc << " (" << (unc/eff)*100 << "%)\n";
            
            outFile1 << detector << "\t" << energy << "\t" << eff << "\t" << unc << std::endl;
        }
    }
    legend->Draw();
    c->SetLogx();
    c->SetLogy();
    c->SaveAs((output_dir +"xxxxxx.pdf").c_str());
}