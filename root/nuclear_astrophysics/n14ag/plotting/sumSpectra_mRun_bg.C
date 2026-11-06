#include "TFile.h"
#include "TDirectory.h"
#include "TH1.h"
#include "TSystem.h"
#include "TNamed.h"
#include "TString.h"
#include <iostream>
#include <map>
#include <vector>

void sumSpectra_mRun_bg() {
    // ========== Run numbers ==========
    std::vector<int> run_numbers = {
        4706, 4713, 4725, 4734, 4735, 4736
    };
    // std::vector<int> run_numbers = {
    //     4739
    // };

    // ========== Detector map ==========
    std::map<std::string, std::string> detector_map = {
        {"histoCal[00]", "EB18_A"}, {"histoCal[01]", "EB18_B"}, {"histoCal[02]", "EB18_C"},
        {"histoCal[03]", "EB18_D"}, {"histoCal[04]", "EB18_E"}, {"histoCal[05]", "EB18_F"},
        {"histoCal[06]", "EB18_G"}, {"histoCal[23]", "MB1_1"}, {"histoCal[24]", "MB1_2"},
        {"histoCal[25]", "MB1_3"}, {"histoCal[07]", "MB2_1"}, {"histoCal[08]", "MB2_2"},
        {"histoCal[09]", "MB2_3"}
    };

    // ========== Output file ==========
    TFile* outFile = new TFile("/felsdata/n14ag_Felsenkeller_2025/Anup/573keVresonance/602keV_250826/Run4706-13-25-34to36_bg.root", "RECREATE");

    // Histograms per crystal
    std::map<std::string, TH1*> crystalHists;

    // Histograms per detector group
    std::map<std::string, TH1*> groupHists;

    double total_rt = 0; // total realtime

    // ========== Loop over runs ==========
    for (int run_number : run_numbers) {
        TString input_file = TString::Format("/felsdata/n14ag_Felsenkeller_2025/Rootfiles_high/Run%d_high_Tree.root", run_number);
        TFile* readFile = TFile::Open(input_file, "READ");
        if (!readFile || readFile->IsZombie()) {
            std::cerr << "Error: Could not open ROOT file for run " << run_number << std::endl;
            continue;
        }

        // Get histoCal directory
        TDirectory* dir = (TDirectory*)readFile->Get("histoCal");
        if (!dir) {
            std::cerr << "Error: Could not find histoCal directory in run " << run_number << std::endl;
            readFile->Close();
            continue;
        }

        // Read realtime
        TNamed* named = (TNamed*)readFile->Get("RunInfo/Realtime[s]");
        double rt = 0;
        if (named) {
            sscanf(named->GetTitle(), "%lf", &rt);
            total_rt += rt;
        } else {
            std::cerr << "Warning: Could not read realtime for run " << run_number << std::endl;
        }

        // Loop over all defined detectors
        for (auto& kv : detector_map) {
            std::string histoName = kv.first;
            std::string detectorName = kv.second;

            TH1* htemp = (TH1*)dir->Get(histoName.c_str());
            if (!htemp) {
                std::cerr << "Warning: Histogram " << histoName << " not found in run " << run_number << std::endl;
                continue;
            }

            // Clone histogram for first time, then add
            if (crystalHists.find(detectorName) == crystalHists.end()) {
                crystalHists[detectorName] = (TH1*)htemp->Clone(detectorName.c_str());
                crystalHists[detectorName]->SetDirectory(0);
            } else {
                crystalHists[detectorName]->Add(htemp);
            }

            // Group name = part before "_"
            std::string groupName = detectorName.substr(0, detectorName.find("_"));
            if (groupHists.find(groupName) == groupHists.end()) {
                groupHists[groupName] = (TH1*)htemp->Clone(groupName.c_str());
                groupHists[groupName]->SetDirectory(0);
            } else {
                groupHists[groupName]->Add(htemp);
            }
        }

        readFile->Close();
    }

    // ========== Write output ==========
    outFile->cd();

    outFile->mkdir("Crystals");
    outFile->cd("Crystals");
    for (auto& kv : crystalHists) {
        kv.second->Write();
    }

    outFile->mkdir("Groups");
    outFile->cd("Groups");
    for (auto& kv : groupHists) {
        kv.second->Write();
    }

    outFile->Close();

    std::cout << "Finished. Total realtime = " << total_rt << " s" << std::endl;
    std::cout << "Spectra saved in /felsdata/n14ag_Felsenkeller_2025/Anup/combined_spectra_bg.root" << std::endl;
}
