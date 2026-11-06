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

void energyCalibration_v2() {
    int run_number = 4557;
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
        inFile->Close();
        return;
    }

    // Detector names
    std::vector<std::string> detector_names = {
        "EB18_A", "EB18_B", "EB18_C", "EB18_D", "EB18_E", "EB18_F", "EB18_G",
        "MB1_1", "MB1_2", "MB1_3", "MB2_1", "MB2_2", "MB2_3", "Ron100"
    };

    // Define ROIs
    std::map<std::string, std::pair<std::pair<int, int>, std::pair<int, int>>> detector_rois = {
        {"EB18_A", {{1450, 1510}, {2640, 2665}}},
        {"EB18_B", {{1435, 1475}, {2570, 2640}}},
        {"EB18_C", {{1440, 1465}, {2590, 2615}}},
        {"EB18_D", {{1440, 1500}, {2600, 2640}}},
        {"EB18_E", {{1420, 1500}, {2600, 2640}}},
        {"EB18_F", {{1430, 1490}, {2580, 2660}}},
        {"EB18_G", {{1460, 1500}, {2630, 2670}}},
        {"MB1_1",  {{1440, 1475}, {2570, 2630}}},
        {"MB1_2",  {{1415, 1480}, {2570, 2630}}},
        {"MB1_3",  {{1450, 1480}, {2580, 2620}}},
        {"MB2_1",  {{1445, 1490}, {2590, 2670}}},
        {"MB2_2",  {{1430, 1510}, {2620, 2660}}},
        {"MB2_3",  {{1430, 1490}, {2610, 2630}}},
        {"Ron100", {{1400, 1480}, {2540, 2620}}}
    };

    // Create output ROOT file
    TFile* outFile = new TFile(outputFileName.c_str(), "RECREATE");
    if (!outFile || outFile->IsZombie()) {
        std::cerr << "Error creating output file!" << std::endl;
        inFile->Close();
        return;
    }

    TDirectory* energyDir = outFile->mkdir("Energies");
    TDirectory* calibDir = outFile->mkdir("Calibration");

    // Create canvases
    TCanvas* masterCanvas40K = new TCanvas("masterCanvas40K", "40K Peaks - All Detectors", 1200, 800);
    masterCanvas40K->Divide(4, 4);
    TCanvas* masterCanvas208Tl = new TCanvas("masterCanvas208Tl", "208Tl Peaks - All Detectors", 1200, 800);
    masterCanvas208Tl->Divide(4, 4);

    // Style settings
    gStyle->SetOptStat(0);
    gStyle->SetOptFit(1);

    for (size_t i = 0; i < detector_names.size(); i++) {
        const auto& det = detector_names[i];
        TH1F* h = (TH1F*)dir->Get(det.c_str());
        if (!h) {
            std::cerr << "Warning: Histogram " << det << " not found." << std::endl;
            continue;
        }

        auto roi40K = detector_rois[det].first;
        auto roi208Tl = detector_rois[det].second;

        // Store fit parameters
        double peak40K = 0, peak208Tl = 0;

        // Process 40K peak
        {
            masterCanvas40K->cd(i+1);
            
            TH1F* h40K = (TH1F*)h->Clone((det + "_40K").c_str());
            h40K->GetXaxis()->SetRangeUser(roi40K.first, roi40K.second);
            h40K->SetTitle((det + " - 40K Peak").c_str());
            h40K->SetLineColor(kBlue);
            h40K->Draw("HIST");
            
            TF1* fit40K = new TF1((det + "_fit40K").c_str(), "gaus + pol0(3)", roi40K.first, roi40K.second);
            fit40K->SetParameters(h40K->GetMaximum()/2, (roi40K.first + roi40K.second)/2, 5);
            h40K->Fit(fit40K, "RQ");
            fit40K->SetLineColor(kRed);
            fit40K->Draw("SAME");
            
            peak40K = fit40K->GetParameter(1);
        }

        // Process 208Tl peak
        {
            masterCanvas208Tl->cd(i+1);
            
            TH1F* h208Tl = (TH1F*)h->Clone((det + "_208Tl").c_str());
            h208Tl->GetXaxis()->SetRangeUser(roi208Tl.first, roi208Tl.second);
            h208Tl->SetTitle((det + " - 208Tl Peak").c_str());
            h208Tl->SetLineColor(kBlue);
            h208Tl->Draw("HIST");
            
            TF1* fit208Tl = new TF1((det + "_fit208Tl").c_str(), "gaus + pol0(3)", roi208Tl.first, roi208Tl.second);
            fit208Tl->SetParameters(h208Tl->GetMaximum()/10, (roi208Tl.first + roi208Tl.second)/2, 5);
            h208Tl->Fit(fit208Tl, "RQ");
            fit208Tl->SetLineColor(kRed);
            fit208Tl->Draw("SAME");
            
            peak208Tl = fit208Tl->GetParameter(1);
        }

        // Energy calibration
        double energy40K = 1460.8;
        double energy208Tl = 2614.5;

        TGraph* g = new TGraph(2);
        g->SetPoint(0, peak40K, energy40K);
        g->SetPoint(1, peak208Tl, energy208Tl);

        TF1* calibFit = new TF1((det + "_calibFit").c_str(), "[0] + [1]*x", 0, 3000);
        g->Fit(calibFit, "Q");

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
    }

    // Save and display canvases
    calibDir->cd();
    
    masterCanvas40K->Draw();
    masterCanvas40K->Update();
    masterCanvas40K->Write();
    masterCanvas40K->SaveAs((outputDir + "40K_peaks.pdf").c_str());
    
    masterCanvas208Tl->Draw();
    masterCanvas208Tl->Update();
    masterCanvas208Tl->Write();
    masterCanvas208Tl->SaveAs((outputDir + "208Tl_peaks.pdf").c_str());

    // Clone canvases before closing files
    // TCanvas* persistentCanvas40K = (TCanvas*)masterCanvas40K->DrawClone();
    // TCanvas* persistentCanvas208Tl = (TCanvas*)masterCanvas208Tl->DrawClone();

    outFile->Close();
    inFile->Close();
    // Keep canvases open
    // persistentCanvas40K->Draw();
    // persistentCanvas208Tl->Draw();
    
    std::cout << "\n\nProcessing complete!" << std::endl;
    std::cout << "Results saved to: " << outputFileName << std::endl;
    std::cout << "Press ENTER in the terminal to exit..." << std::endl;
    
    // Wait for user input before exiting
    std::cin.ignore();
    
    // Cleanup
    // delete persistentCanvas40K;
    // delete persistentCanvas208Tl;
}