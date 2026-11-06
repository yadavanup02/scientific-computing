#include "TFile.h"
#include "TTree.h"
#include "TH1F.h"
#include "TCanvas.h"
#include "TString.h"
#include "TLine.h"
#include "TSpectrum.h"
#include "TMarker.h"
#include <iostream>
#include <fstream>
#include <iomanip>
#include <sys/stat.h>
#include <vector>
#include <map>

void peakCountsmRun_addback_v2_250902() {
    TCanvas tmpCanvas("tmpCanvas", "Temporary Canvas", 800, 600);

    std::vector<int> run_numbers = {4588};
    // Define detectors and their addback formulas
    std::vector<std::pair<std::string, std::string>> detectors = {
        // {"EB18", "(MeV00+MeV01+MeV02+MeV03+MeV04+MeV05+MeV06)"},
        // {"MB1", "(MeV23+MeV24+MeV25)"},
        // // {"MB2", "(MeV07+MeV08+MeV09)"},
        // {"AllnDE", "(MeV00+MeV01+MeV02+MeV05+MeV06+MeV23+MeV24+MeV25+MeV07+MeV08+MeV09)"},
        {"All", "(MeV00+MeV01+MeV02+MeV03+MeV04+MeV05+MeV06+MeV23+MeV24+MeV25+MeV07+MeV08+MeV09)"}
    };
    

    for (int run_number : run_numbers) {
        // File handling
        TString input_file = TString::Format("/felsdata/n14ag_Felsenkeller_2025/Rootfiles_high/Run%d_high_Tree.root", run_number);
        TString output_dir = TString::Format("/felsdata/n14ag_Felsenkeller_2025/Anup/Run%d/", run_number);
        gSystem->mkdir(output_dir, true);
        TString output_rootfile = output_dir + "Addback_tree.root";
        
        TFile* readFile = TFile::Open(input_file, "READ");
        if (!readFile || readFile->IsZombie()) {
            std::cerr << "Error: Could not open ROOT file for run " << run_number << std::endl;
            continue;
        }

        TTree* felsTree = (TTree*)readFile->Get("Tree/felsTree");
        if (!felsTree) {
            std::cerr << "Error: Could not find tree in run " << run_number << std::endl;
            readFile->Close();
            continue;
        }

        // Create output files
        TFile* outFile = new TFile(output_rootfile, "RECREATE");

        // Process each detector
        for (size_t det_idx = 0; det_idx < detectors.size(); det_idx++) {
            const auto& detector_pair = detectors[det_idx];
            const std::string& det_name = detector_pair.first;
            const std::string& det_formula = detector_pair.second;

            
            // Create and fill histogram
            TH1F* h = new TH1F(det_name.c_str(), Form("%s;Energy (MeV);Counts", det_name.c_str()), 3000, 0, 6.0);
            felsTree->Draw(Form("%s>>%s", det_formula.c_str(), det_name.c_str()), 
                          Form("%s>0", det_formula.c_str()), "goff");
            
            outFile->cd();
            h->Write();
            delete h;

        }
        outFile->Close();
        readFile->Close();
    }
    
    
    std::cout << "All runs processed!" << std::endl;
}