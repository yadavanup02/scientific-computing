#include "TFile.h"
#include "TTree.h"
#include "TH1F.h"
#include "TCanvas.h"
#include "TString.h"
#include <vector>
#include <map>
#include <iostream>

void addback_analysis() {
    int run_number = 4557;
    TString input_file = TString::Format("/felsdata/n14ag_Felsenkeller_2025/Rootfiles_high/Run%d_high_250502_0942_Tree.root", run_number);
    TString output_dir = TString::Format("/felsdata/n14ag_Felsenkeller_2025/Anup/Run%d/", run_number);
    
    // Open input file
    TFile* file = TFile::Open(input_file, "READ");
    if (!file || file->IsZombie()) {
        std::cerr << "Error: Could not open ROOT file!" << std::endl;
        return;
    }

    // Get the tree
    TTree* tree = (TTree*)file->Get("Tree/felsTree");
    if (!tree) {
        std::cerr << "Error: Could not find tree!" << std::endl;
        file->Close();
        return;
    }

    // // Define detector branches
    // std::vector<TString> detector_branches = {
    //     "EB18_A", "EB18_B", "EB18_C", "EB18_D", "EB18_E", "EB18_F", "EB18_G",
    //     "MB1_1", "MB1_2", "MB1_3", "MB2_1", "MB2_2", "MB2_3"
    // };

    // Define detector branches 
    std::vector<TString> detector_branches = {
        "MeV00", "MeV01", "MeV02", "MeV03", "MeV04", "MeV05", "MeV06"
    };
    // std::vector<TString> detector_branches = {
    //     "MeV00", "MeV01", "MeV02", "MeV03", "MeV04", "MeV05", "MeV06",
    //     "MeV23", "MeV24", "MeV25", "MeV07", "MeV08", "MeV09"
    // };
    // Energy range for addback (in MeV)
    double min_energy = 3.7;
    double max_energy = 3.9;

    // Create histograms
    TH1F* h_individual = new TH1F("h_individual", "Individual Detectors;Energy (MeV);Counts", 
                                 1000, min_energy-0.1, max_energy+0.1);
    TH1F* h_addback = new TH1F("h_addback", "Addback Spectrum;Energy (MeV);Counts", 
                               1000, min_energy-0.1, max_energy+0.1);

    // Set up branch addresses
    std::map<TString, float> detector_energies;
    for (const auto& det : detector_branches) {
        tree->SetBranchAddress(det, &detector_energies[det]);
    }

    // Process events
    Long64_t nEntries = tree->GetEntries();
    for (Long64_t i = 0; i < nEntries; i++) {
        tree->GetEntry(i);
        
        float sum_energy = 0;
        bool in_range = false;
        
        // Check each detector
        for (const auto& det : detector_branches) {
            float energy = detector_energies[det];
            
            // Fill individual detectors
            if (energy > min_energy && energy < max_energy) {
                h_individual->Fill(energy);
            }
            
            // Sum energies for addback (only if in our ROI)
            if (energy > 0.1) { // Threshold to avoid noise
                sum_energy += energy;
                in_range = true;
            }
        }
        
        // Fill addback histogram if any detector had energy in ROI
        if (in_range && sum_energy > min_energy && sum_energy < max_energy) {
            h_addback->Fill(sum_energy);
        }
    }

    // Create output file
    TFile* outfile = new TFile(output_dir + "addback_results.root", "RECREATE");
    
    // Save histograms
    h_individual->Write();
    h_addback->Write();
    
    // Create and save canvas
    TCanvas* c1 = new TCanvas("c1", "Addback Comparison", 1200, 800);
    c1->Divide(2,1);
    c1->cd(1);
    h_individual->Draw();
    c1->cd(2);
    h_addback->Draw();
    c1->Write();
    
    // Clean up
    outfile->Close();
    file->Close();
    
    std::cout << "Addback analysis complete. Results saved to: " 
              << output_dir << "addback_results.root" << std::endl;
}