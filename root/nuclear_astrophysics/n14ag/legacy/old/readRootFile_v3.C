#include "TFile.h"
#include "TDirectory.h"
#include "TH1F.h"
#include "TCanvas.h"
#include "TStyle.h"
#include "TF1.h"
#include "TGraph.h"
#include "TLegend.h"
#include <iostream>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
/// @brief // This function reads a ROOT file containing a histogram and performs peak fitting and energy calibration.
void readRootFile_v3() {
    int run_number = 4447;  // Replace with your run number
    std::string rootFilename = "/felsdata/n14ag_Felsenkeller_2025/Rootfiles_high/Run" + std::to_string(run_number) + "_high.root";

    // Open the ROOT file
    TFile* file = TFile::Open(rootFilename.c_str(), "READ");
    if (!file || file->IsZombie()) {
        std::cerr << "Error: Could not open file " << rootFilename << std::endl;
        return;
    }
    std::cout << "Opened ROOT file successfully: " << rootFilename << std::endl;

    // Access the "Energies" directory
    TDirectory* dir = (TDirectory*)file->Get("Energies");
    if (!dir) {
        std::cerr << "Error: Could not find 'Energies' directory in the file." << std::endl;
        file->Close();
        return;
    }

    //Get the histogram  
    TH1F* ron100Hist = (TH1F*)dir->Get("EB18_G");
    if (!ron100Hist) {
        std::cerr << "Error: Could not find histogram 'EB18_G'." << std::endl;
        file->Close();
        return;
    }
    // Create output directory if it does not exist
    std::string output_dir = "/felsdata/n14ag_Felsenkeller_2025/Rootfiles_high/Anup/Run" + std::to_string(run_number) + "_high";
    struct stat info;
    if (stat(output_dir.c_str(), &info) != 0) {
        mkdir(output_dir.c_str(), 0777);
    }
    // Create a canvas for fitting peaks
    TCanvas* fitCanvas = new TCanvas("fitCanvas", "Peak Fitting", 800, 600);
    fitCanvas->cd();
    ron100Hist->SetLineColor(kBlue);
    ron100Hist->SetTitle("EB18G Peak Fitting; Channel; Counts");
    ron100Hist->Draw("HIST");

    // Define ROIs
    int roi_40K_min = 1460, roi_40K_max = 1500;
    int roi_208Tl_min = 2640, roi_208Tl_max = 2680;

    // Gaussian fits
    //TF1* fit_40K = new TF1("fit_40K", "gaus", roi_40K_min, roi_40K_max);
    //TF1* fit_208Tl = new TF1("fit_208Tl", "gaus", roi_208Tl_min, roi_208Tl_max);
    // // Gaussian fits
    TF1* fit_40K = new TF1("fit_40K", "gaus + pol0(3)", roi_40K_min, roi_40K_max);
    TF1* fit_208Tl = new TF1("fit_208Tl", "gaus + pol0(3)", roi_208Tl_min, roi_208Tl_max);


    fit_40K->SetLineColor(kRed);
    fit_208Tl->SetLineColor(kGreen);

    // Fit histograms
    fit_40K->SetParameters(800, 1480, 4);
    fit_208Tl->SetParameters(100, 2653, 5);
    ron100Hist->Fit(fit_40K, "R");
    ron100Hist->Fit(fit_208Tl, "R");

    // Draw fits explicitly
    fit_40K->Draw("SAME");
    fit_208Tl->Draw("SAME");

    // Update the canvas
    ron100Hist->GetXaxis()->SetRangeUser(1300, 2800);
    // ron100Hist->SetLogy();
    fitCanvas->SetLogy();
    //fitCanvas->SetLogy();
    fitCanvas->Update();
    fitCanvas->Draw();

    // Extract peak positions
    double peak_40K = fit_40K->GetParameter(1);  // Mean (centroid)
    double peak_208Tl = fit_208Tl->GetParameter(1);

    std::cout << "Fitted Peak Positions: 40K = " << peak_40K << ", 208Tl = " << peak_208Tl << std::endl;

    // True energy values
    double energy_40K = 1460.8;
    double energy_208Tl = 2614.5;

    // Create a graph for linear calibration
    TCanvas* calibCanvas = new TCanvas("calibCanvas", "Energy Calibration", 800, 600);
    calibCanvas->cd();
    TGraph* graph = new TGraph(2);
    graph->SetPoint(0, peak_40K, energy_40K);
    graph->SetPoint(1, peak_208Tl, energy_208Tl);

    graph->SetMarkerStyle(20);
    graph->SetTitle("Energy Calibration; Channel; Energy (keV)");
    graph->GetXaxis()->SetRangeUser(0, 3000);
    graph->Draw("AP");

    // Fit with linear function
    TF1* linearFit = new TF1("linearFit", "[0] + [1]*x", 0, 3000);
    graph->Fit(linearFit, "R");
    graph->Draw("AP");

    // Extract calibration parameters
    double intercept = linearFit->GetParameter(0);
    double slope = linearFit->GetParameter(1);

    std::cout << "Calibration Equation: Energy = " << intercept << " + " << slope << " * Channel" << std::endl;
    // Convert the histogram into energy scale
    int nbins = ron100Hist->GetNbinsX();
    double eMin = intercept + slope * ron100Hist->GetBinCenter(1);
    double eMax = intercept + slope * ron100Hist->GetBinCenter(nbins);
    
    // Convert the histogram into energy scale
    TH1F* energyHist = new TH1F("energyHist", "Energy Spectrum; Energy (keV); Counts",
                                nbins, eMin, eMax);

    for (int i = 1; i <= nbins; i++) {
        double energy = intercept + slope * ron100Hist->GetBinCenter(i);
        energyHist->SetBinContent(i, ron100Hist->GetBinContent(i));
    }

    // Plot the calibrated spectrum
    TCanvas* energyCanvas = new TCanvas("energyCanvas", "Energy Spectrum", 800, 600);
    energyCanvas->cd();
    energyHist->SetLineColor(kBlue);
    energyHist->SetTitle("Energy Spectrum; Energy (keV); Counts");
    energyHist->Draw("HIST");
    //energyCanvas->SetLogy();
    energyCanvas->Update();
    energyCanvas->Draw();

    // Save the calibrated spectrum to a new ROOT file
    TFile* outFile = new TFile((output_dir + "/Calib_Run" + std::to_string(run_number) + "_EB18_G.root").c_str(), "RECREATE");
    energyHist->Write();
    outFile->Close();
}
