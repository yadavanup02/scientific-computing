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

void energyCalibration_v1() {
    int run_number = 4422;
    std::string inputFilePath = "/felsdata/n14ag_Felsenkeller_2025/Rootfiles_high/Run" + std::to_string(run_number) + "_high.root";
    std::string outputDir = "/felsdata/n14ag_Felsenkeller_2025/Rootfiles_high/Anup/Run" + std::to_string(run_number) + "/";
    std::string outputFileName = outputDir + "Calibrated_Run" + std::to_string(run_number) + ".root";

    // Create output directory if it doesn't exist
    struct stat info;
    if (stat(outputDir.c_str(), &info) != 0) mkdir(outputDir.c_str(), 0777);

    // Open input file
    TFile* inFile = TFile::Open(inputFilePath.c_str(), "READ");
    if (!inFile || inFile->IsZombie()) {
        std::cerr << "Error opening input file!" << std::endl;
        return;
    }

    TDirectory* dir = (TDirectory*)inFile->Get("Energies");
    if (!dir) {
        std::cerr << "Could not find 'Energies' directory." << std::endl;
        return;
    }

    // Detector names
    std::vector<std::string> detector_names = {
        "EB18_A", "EB18_B", "EB18_C", "EB18_D", "EB18_E", "EB18_F", "EB18_G",
        "MB1_1", "MB1_2", "MB1_3", "MB2_1", "MB2_2", "MB2_3", "Ron100"
    };

    // Define ROIs
    std::map<std::string, std::pair<std::pair<int, int>, std::pair<int, int>>> detector_rois = {
        {"EB18_A", {{1450, 1510}, {2630, 2670}}},
        {"EB18_B", {{1420, 1480}, {2570, 2640}}},
        {"EB18_C", {{1420, 1480}, {2570, 2630}}},
        {"EB18_D", {{1440, 1500}, {2610, 2680}}},
        {"EB18_E", {{1420, 1500}, {2590, 2650}}},
        {"EB18_F", {{1430, 1510}, {2580, 2660}}},
        {"EB18_G", {{1440, 1520}, {2610, 2690}}},
        {"MB1_1",  {{1420, 1500}, {2570, 2630}}},
        {"MB1_2",  {{1415, 1480}, {2570, 2630}}},
        {"MB1_3",  {{1420, 1500}, {2580, 2620}}},
        {"MB2_1",  {{1430, 1510}, {2590, 2670}}},
        {"MB2_2",  {{1430, 1510}, {2600, 2670}}},
        {"MB2_3",  {{1420, 1500}, {2610, 2640}}},
        {"Ron100", {{1400, 1480}, {2540, 2620}}}
    };

    // Create output ROOT file
    TFile* outFile = new TFile(outputFileName.c_str(), "RECREATE");
    TDirectory* energyDir = outFile->mkdir("Energies");
    TDirectory* calibDir  = outFile->mkdir("Calibration");

    // Create canvases for peak fitting
    TCanvas* canvas40K = new TCanvas("canvas40K", "40K Peak Fits", 1200, 900);
    canvas40K->Divide(4, 4);
    TCanvas* canvas208Tl = new TCanvas("canvas208Tl", "208Tl Peak Fits", 1200, 900);
    canvas208Tl->Divide(4, 4);
    
    int canvasPadIndex = 1;

    for (const auto& det : detector_names) {
        TH1F* h = (TH1F*)dir->Get(det.c_str());
        if (!h) {
            std::cerr << "Warning: Histogram " << det << " not found." << std::endl;
            continue;
        }

        auto roi40K = detector_rois[det].first;
        auto roi208Tl = detector_rois[det].second;

        TF1* fit40K = new TF1("fit40K", "gaus + pol0(3)", roi40K.first, roi40K.second);
        TF1* fit208Tl = new TF1("fit208Tl", "gaus + pol0(3)", roi208Tl.first, roi208Tl.second);

        fit40K->SetParameters(h->GetMaximum() /2, (roi40K.first + roi40K.second) / 2, 5);
        fit208Tl->SetParameters(h->GetMaximum() / 10, (roi208Tl.first + roi208Tl.second) / 2, 5);

        h->Fit(fit40K, "RQ");
        h->Fit(fit208Tl, "RQ");

        double peak40K = fit40K->GetParameter(1);
        double peak208Tl = fit208Tl->GetParameter(1);

        double energy40K = 1460.8;
        double energy208Tl = 2614.5;

        TGraph* g40K = new TGraph(1);
        g40K->SetPoint(0, peak40K, energy40K);

        TGraph* g208Tl = new TGraph(1);
        g208Tl->SetPoint(0, peak208Tl, energy208Tl);

        TF1* calibFit = new TF1("calibFit", "[0] + [1]*x", 0, 3000);
        //g40K->Fit(calibFit, "Q");
        //g208Tl->Fit(calibFit, "Q");

        double offset = calibFit->GetParameter(0);
        double slope = calibFit->GetParameter(1);

        int nbins = h->GetNbinsX();
        double eMin = offset + slope * h->GetBinCenter(1);
        double eMax = offset + slope * h->GetBinCenter(nbins);
        TH1F* h_energy = new TH1F((det).c_str(), (det + " Calibrated; Energy (keV); Counts").c_str(), nbins, eMin, eMax);
        for (int i = 1; i <= nbins; i++) {
            h_energy->SetBinContent(i, h->GetBinContent(i));
        }

        energyDir->cd();
        h_energy->Write();

        // Plot 40K
        canvas40K->cd(canvasPadIndex);
        h->GetXaxis()->SetRangeUser(roi40K.first, roi40K.second);
        h->SetLineColor(kBlue);
        h->SetTitle((det + " - 40K Peak").c_str());
        h->Draw("HIST");
        fit40K->SetLineColor(kRed);
        //fit40K->Draw("SAME");

        // Plot 208Tl
        canvas208Tl->cd(canvasPadIndex);
        h->GetXaxis()->SetRangeUser(roi208Tl.first, roi208Tl.second);
        h->SetLineColor(kBlue);
        h->SetTitle((det + " - 208Tl Peak").c_str());
        h->Draw("HIST");
        fit208Tl->SetLineColor(kRed);
        //fit208Tl->Draw("SAME");

        canvasPadIndex++; 
    }


    // Save canvases
    calibDir->cd();
    canvas40K->Write();
    canvas208Tl->Write();

    outFile->Close();
    std::cout << "Saved calibrated histograms and canvases to: " << outputFileName << std::endl;
}
