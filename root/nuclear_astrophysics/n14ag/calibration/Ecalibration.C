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
#include <vector>
#include <map>
#include <sys/stat.h>
#include <sys/types.h>

/// @brief Reads a ROOT file, fits peaks for multiple detectors, and calibrates energy.
void Ecalibration() {
    int run_number = 4448;  // Replace with your run number
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

    // Create output directory if it does not exist
    std::string output_dir = "/felsdata/n14ag_Felsenkeller_2025/Rootfiles_high/Anup/Run" + std::to_string(run_number) + "_high";
    struct stat info;
    if (stat(output_dir.c_str(), &info) != 0) {
        mkdir(output_dir.c_str(), 0777);
    }

    // List of detector names
    std::vector<std::string> detector_names = {
        "EB18_A", "EB18_B", "EB18_C", "EB18_D", "EB18_E", "EB18_F", "EB18_G",
        "MB1_1", "MB1_2", "MB1_3", "MB2_1", "MB2_2", "MB2_3", "Ron100"
    };

    // Define individual ROIs for each detector
    std::map<std::string, std::pair<std::pair<int, int>, std::pair<int, int>>> detector_rois = {
        {"EB18_A", {{1450, 1510}, {2620, 2700}}},
        {"EB18_B", {{1420, 1480}, {2570, 2640}}},
        {"EB18_C", {{1420, 1480}, {2570, 2630}}},
        {"EB18_D", {{1440, 1500}, {2590, 2680}}},
        {"EB18_E", {{1420, 1500}, {2580, 2660}}},
        {"EB18_F", {{1430, 1510}, {2580, 2660}}},
        {"EB18_G", {{1440, 1520}, {2610, 2690}}},
        {"MB1_1",  {{1420, 1500}, {2570, 2630}}},
        {"MB1_2",  {{1415, 1480}, {2570, 2630}}},
        {"MB1_3",  {{1420, 1500}, {2575, 2625}}},
        {"MB2_1",  {{1430, 1510}, {2590, 2670}}},
        {"MB2_2",  {{1430, 1510}, {2600, 2670}}},
        {"MB2_3",  {{1420, 1500}, {2600, 2680}}},
        {"Ron100", {{1400, 1480}, {2540, 2620}}}
    };

    for (const auto& det : detector_names) {
        std::cout << "Processing detector: " << det << std::endl;

        // Retrieve the histogram
        TH1F* hist = (TH1F*)dir->Get(det.c_str());
        if (!hist) {
            std::cerr << "Warning: Could not find histogram '" << det << "'. Skipping..." << std::endl;
            continue;
        }

        // Create canvas
        TCanvas* fitCanvas = new TCanvas(("fitCanvas_" + det).c_str(), ("Peak Fitting - " + det).c_str(), 800, 600);
        fitCanvas->cd();
        hist->SetLineColor(kBlue);
        hist->SetTitle((det + " Peak Fitting; Channel; Counts").c_str());
        hist->Draw("HIST");

        // Fetch ROIs
        auto rois = detector_rois[det];
        int roi_40K_min = rois.first.first;
        int roi_40K_max = rois.first.second;
        int roi_208Tl_min = rois.second.first;
        int roi_208Tl_max = rois.second.second;

        // Define fitting functions
        TF1* fit_40K = new TF1(("fit_40K_" + det).c_str(), "gaus + pol0(3)", roi_40K_min, roi_40K_max);
        TF1* fit_208Tl = new TF1(("fit_208Tl_" + det).c_str(), "gaus + pol0(3)", roi_208Tl_min, roi_208Tl_max);

        fit_40K->SetLineColor(kRed);
        fit_208Tl->SetLineColor(kGreen);

        // Initial parameters for fits
        fit_40K->SetParameters(800, (roi_40K_min + roi_40K_max) / 2.0, 4);
        fit_208Tl->SetParameters(100, (roi_208Tl_min + roi_208Tl_max) / 2.0, 5);

        // Fit peaks
        hist->Fit(fit_40K, "R");
        hist->Fit(fit_208Tl, "R");

        fit_40K->Draw("SAME");
        fit_208Tl->Draw("SAME");

        hist->GetXaxis()->SetRangeUser(1350, 2700);
        fitCanvas->SetLogy();
        fitCanvas->Update();
        fitCanvas->Draw();

        // Extract fitted peak positions
        double peak_40K = fit_40K->GetParameter(1);
        double peak_208Tl = fit_208Tl->GetParameter(1);

        std::cout << "Detector " << det << ": Fitted Peaks -> 40K: " << peak_40K << ", 208Tl: " << peak_208Tl << std::endl;

        // True energy values
        double energy_40K = 1460.8;
        double energy_208Tl = 2614.5;

        // Calibration graph
        TCanvas* calibCanvas = new TCanvas(("calibCanvas_" + det).c_str(), ("Energy Calibration - " + det).c_str(), 800, 600);
        calibCanvas->cd();
        TGraph* graph = new TGraph(2);
        graph->SetPoint(0, peak_40K, energy_40K);
        graph->SetPoint(1, peak_208Tl, energy_208Tl);

        graph->SetMarkerStyle(20);
        graph->SetTitle(("Energy Calibration; Channel; Energy (keV) - " + det).c_str());
        graph->GetXaxis()->SetRangeUser(0, 3000);
        graph->Draw("AP");

        TF1* linearFit = new TF1(("linearFit_" + det).c_str(), "[0] + [1]*x", 0, 3000);
        // graph->Fit(linearFit, "R");
        //graph->Draw("AP");

        double intercept = linearFit->GetParameter(0);
        double slope = linearFit->GetParameter(1);

        std::cout << "Detector " << det << ": Calibration -> Energy = " << intercept << " + " << slope << " * Channel" << std::endl;

        // Create calibrated histogram
        int nbins = hist->GetNbinsX();
        double eMin = intercept + slope * hist->GetBinCenter(1);
        double eMax = intercept + slope * hist->GetBinCenter(nbins);

        TH1F* energyHist = new TH1F(("energyHist_" + det).c_str(), ("Energy Spectrum - " + det + "; Energy (keV); Counts").c_str(), nbins, eMin, eMax);

        for (int i = 1; i <= nbins; ++i) {
            double energy = intercept + slope * hist->GetBinCenter(i);
            energyHist->SetBinContent(i, hist->GetBinContent(i));
        }

        TCanvas* energyCanvas = new TCanvas(("energyCanvas_" + det).c_str(), ("Energy Spectrum - " + det).c_str(), 800, 600);
        energyCanvas->cd();
        energyHist->SetLineColor(kBlue);
        energyHist->Draw("HIST");
        energyCanvas->Update();
        energyCanvas->Draw();

        // Save the calibrated histogram
        TFile* outFile = new TFile((output_dir + "/Calib_Run" + std::to_string(run_number) + "_" + det + "_high.root").c_str(), "RECREATE");
        energyHist->Write();
        outFile->Close();

        //Clean up
        // delete fit_40K;
        // delete fit_208Tl;
        // delete linearFit;
        // delete graph;
        // delete energyHist;
        // delete outFile;
    }

    //file->Close();
    std::cout << "All detectors processed!" << std::endl;
}
