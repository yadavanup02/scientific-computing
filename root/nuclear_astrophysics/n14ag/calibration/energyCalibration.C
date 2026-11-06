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

void energyCalibration() {
    int run_number = 4448;
    std::string inputFilePath = "/felsdata/n14ag_Felsenkeller_2025/Rootfiles_high/Run" + std::to_string(run_number) + "_high.root";
    std::string outputDir = "/felsdata/n14ag_Felsenkeller_2025/Rootfiles_high/Anup/";
    std::string outputFileName = outputDir + "Calibrated_AllDetectors_Run" + std::to_string(run_number) + ".root";

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

    // Create output ROOT file
    TFile* outFile = new TFile(outputFileName.c_str(), "RECREATE");
    TDirectory* energyDir = outFile->mkdir("Energies");
    energyDir->cd();

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

        fit40K->SetParameters(h->GetMaximum(), (roi40K.first + roi40K.second)/2, 4);
        fit208Tl->SetParameters(h->GetMaximum()/10, (roi208Tl.first + roi208Tl.second)/2, 5);

        h->Fit(fit40K, "RQ");
        h->Fit(fit208Tl, "RQ");

        double peak40K = fit40K->GetParameter(1);
        double peak208Tl = fit208Tl->GetParameter(1);

        // True energies
        double energy40K = 1460.8;
        double energy208Tl = 2614.5;

        // Calibration graph
        TGraph* g = new TGraph(2);
        g->SetPoint(0, peak40K, energy40K);
        g->SetPoint(1, peak208Tl, energy208Tl);

        TF1* calibFit = new TF1("calibFit", "[0] + [1]*x", 0, 3000);
        g->Fit(calibFit, "Q");

        double offset = calibFit->GetParameter(0);
        double slope = calibFit->GetParameter(1);

        // Calibrated histogram
        int nbins = h->GetNbinsX();
        double eMin = offset + slope * h->GetBinCenter(1);
        double eMax = offset + slope * h->GetBinCenter(nbins);
        TH1F* h_energy = new TH1F((det + "_Energy").c_str(), (det + " Calibrated; Energy (keV); Counts").c_str(), nbins, eMin, eMax);

        for (int i = 1; i <= nbins; i++) {
            h_energy->SetBinContent(i, h->GetBinContent(i));
        }

        h_energy->Write();
    }

    outFile->Close();
    std::cout << "All calibrated spectra saved to: " << outputFileName << std::endl;
}
