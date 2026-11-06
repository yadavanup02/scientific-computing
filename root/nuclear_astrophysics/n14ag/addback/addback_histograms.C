/*
#include "TFile.h"
#include "TH1F.h"
#include "TCanvas.h"
#include "TDirectory.h"
#include "TString.h"
#include "TSystem.h"
#include <iostream>
#include <vector>

void addback_histograms() {
    int run_number = 4450;
    TString input_file = TString::Format("/felsdata/n14ag_Felsenkeller_2025/Rootfiles_high/Run%d_high_fromBuffer_250430_1824_Tree.root", run_number);
    TString output_dir = TString::Format("/felsdata/n14ag_Felsenkeller_2025/Anup/Run%d/", run_number);
    TString output_file = output_dir + "Addback_Histogram.root";

    // Create output dir if it doesn't exist
    gSystem->mkdir(output_dir, true);

    // Open input file
    TFile* inFile = new TFile(input_file, "READ");
    if (!inFile || inFile->IsZombie()) {
        std::cerr << "Error: Could not open input file!" << std::endl;
        return;
    }

    // Access the histoRaw directory
    TDirectory* histoDir = (TDirectory*)inFile->Get("histoRaw");
    if (!histoDir) {
        std::cerr << "Error: 'histoRaw' directory not found!" << std::endl;
        inFile->Close();
        return;
    }

    // Switch to the histogram directory
    histoDir->cd();

    // Define index groups for each detector block
    std::vector<int> EB18_indices = {0, 1, 2, 3, 4, 5, 6};
    std::vector<int> MB1_indices = {23, 24, 25};
    std::vector<int> MB2_indices = {7, 8, 9};

    // Function to create an addback histogram
    auto createAddback = [&](const std::vector<int>& indices, const TString& name, const TString& title) {
        TH1F* sumHist = nullptr;
        for (int idx : indices) {
            TString histName = TString::Format("histoRaw[%02d]", idx);
            TH1F* h = (TH1F*)histoDir->Get(histName);
            if (!h) {
                std::cerr << "Warning: Histogram " << histName << " not found!" << std::endl;
                continue;
            }
            if (!sumHist) {
                sumHist = (TH1F*)h->Clone(name);
                sumHist->SetTitle(title);
                sumHist->Reset(); // clear content
            }
            sumHist->Add(h);
        }
        return sumHist;
    };

    // Create addback histograms
    TH1F* EB18Addback = createAddback(EB18_indices, "EB18Addback", "EB18 Addback;Energy (channels);Counts");
    TH1F* MB1Addback = createAddback(MB1_indices, "MB1Addback", "MB1 Addback;Energy (channels);Counts");
    TH1F* MB2Addback = createAddback(MB2_indices, "MB2Addback", "MB2 Addback;Energy (channels);Counts");

    // Plot on canvas
    TCanvas* c = new TCanvas("c_addback", "Addback Histograms", 1200, 800);
    c->Divide(2, 2);

    c->cd(1); if (EB18Addback) EB18Addback->Draw();
    c->cd(2); if (MB1Addback)  MB1Addback->Draw();
    c->cd(3); if (MB2Addback)  MB2Addback->Draw();

    // Save to output file
    TFile* outFile = new TFile(output_file, "RECREATE");
    TDirectory* addbackDir = outFile->mkdir("AddbackHistograms");
    addbackDir->cd();

    if (EB18Addback) EB18Addback->Write();
    if (MB1Addback)  MB1Addback->Write();
    if (MB2Addback)  MB2Addback->Write();
    c->Write();

    outFile->Close();
    inFile->Close();

    std::cout << "\nAddback histogram creation complete." << std::endl;
    std::cout << "Saved in: " << output_file << std::endl;
}
*/

#include "TFile.h"
#include "TH1F.h"
#include "TCanvas.h"
#include "TDirectory.h"
#include "TString.h"
#include "TSystem.h"
#include <iostream>
#include <vector>

void addback_histograms() {
    int run_number = 4451;
    TString input_file = TString::Format("/felsdata/n14ag_Felsenkeller_2025/Rootfiles_high/Run%d_high_fromBuffer_250430_1800_Tree.root", run_number);
    TString output_dir = TString::Format("/felsdata/n14ag_Felsenkeller_2025/Anup/Run%d/", run_number);
    TString output_file = output_dir + "Addback_Histogram.root";

    gSystem->mkdir(output_dir, true);

    TFile* inFile = new TFile(input_file, "READ");
    if (!inFile || inFile->IsZombie()) {
        std::cerr << "Error: Could not open input file!" << std::endl;
        return;
    }

    TDirectory* histoDir = (TDirectory*)inFile->Get("histoRaw");
    if (!histoDir) {
        std::cerr << "Error: 'histoRaw' directory not found!" << std::endl;
        inFile->Close();
        return;
    }

    histoDir->cd();

    // Uniform binning (manual override to avoid mismatch)
    const int bins = 3000;
    const double xmin = 0.0;
    const double xmax = 6.0;

    TH1F* EB18Addback = new TH1F("EB18Addback", "EB18 Addback;Energy (MeV);Counts", bins, xmin, xmax);
    TH1F* MB1Addback  = new TH1F("MB1Addback",  "MB1 Addback;Energy (MeV);Counts",  bins, xmin, xmax);
    TH1F* MB2Addback  = new TH1F("MB2Addback",  "MB2 Addback;Energy (MeV);Counts",  bins, xmin, xmax);

    auto rebin_to_common = [&](const std::vector<int>& indices, TH1F* sumHist) {
        for (int idx : indices) {
            TString hname = TString::Format("histoRaw[%02d]", idx);
            TH1F* hOrig = (TH1F*)histoDir->Get(hname);
            if (!hOrig) {
                std::cerr << "Warning: Histogram " << hname << " not found!" << std::endl;
                continue;
            }

            // Fill into target histogram by looping
            for (int bin = 1; bin <= hOrig->GetNbinsX(); ++bin) {
                double energy = hOrig->GetBinCenter(bin);
                double counts = hOrig->GetBinContent(bin);
                if (energy >= xmin && energy < xmax) {
                    sumHist->Fill(energy, counts);
                }
            }
        }
    };

    // Define which detectors belong to which group
    std::vector<int> EB18_indices = {0, 1, 2, 3, 4, 5, 6};
    std::vector<int> MB1_indices = {23, 24, 25};
    std::vector<int> MB2_indices = {7, 8, 9};

    rebin_to_common(EB18_indices, EB18Addback);
    rebin_to_common(MB1_indices,  MB1Addback);
    rebin_to_common(MB2_indices,  MB2Addback);


    TCanvas* c = new TCanvas("c_addback", "Addback Spectra", 1200, 800);
    c->Divide(2, 2);
    c->cd(1); EB18Addback->Draw();
    c->cd(2); MB1Addback->Draw();
    c->cd(3); MB2Addback->Draw();

    // Save output
    TFile* outFile = new TFile(output_file, "RECREATE");
    TDirectory* addbackDir = outFile->mkdir("AddbackHistograms");
    addbackDir->cd();

    EB18Addback->Write();
    MB1Addback->Write();
    MB2Addback->Write();
    c->Write();
    outFile->Write();
    outFile->Close();
    inFile->Close();

    std::cout << " Addback histograms saved to: " << output_file << std::endl;
}
