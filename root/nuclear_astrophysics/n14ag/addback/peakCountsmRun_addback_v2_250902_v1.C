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

void peakCountsmRun_addback_v2_250902_v1() {
    TCanvas tmpCanvas("tmpCanvas", "Temporary Canvas", 800, 600);

    // Example: more than one run will be summed
    std::vector<int> run_numbers = {4616, 4618, 4620, 4622, 4624};

    // Define detectors and their addback formulas
    std::vector<std::pair<std::string, std::string>> detectors = {
        {"All", "(MeV00+MeV01+MeV02+MeV03+MeV04+MeV05+MeV06+MeV23+MeV24+MeV25+MeV07+MeV08+MeV09)"}
        // You can uncomment/add more detectors here
    };

    // === Create summed histograms for each detector ===
    std::map<std::string, TH1F*> summed_hists;
    for (auto& det_pair : detectors) {
        const std::string& det_name = det_pair.first;
        summed_hists[det_name] = new TH1F(Form("%s", det_name.c_str()), 
                                          Form("Summed %s;Energy (MeV);Counts", det_name.c_str()), 
                                          6000, 0, 6.0);
    }

    // Loop over runs
    for (int run_number : run_numbers) {
        TString input_file = TString::Format("/felsdata/n14ag_Felsenkeller_2025/Rootfiles_high/Run%d_high_Tree.root", run_number);
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

        // Process each detector
        for (auto& det_pair : detectors) {
            const std::string& det_name = det_pair.first;
            const std::string& det_formula = det_pair.second;

            // Temporary histogram for this run
            TH1F* htmp = new TH1F(Form("%s_tmp%d", det_name.c_str(), run_number), 
                                  Form("%s Run%d;Energy (MeV);Counts", det_name.c_str(), run_number), 
                                  6000, 0, 6.0);

            felsTree->Draw(Form("%s>>%s", det_formula.c_str(), htmp->GetName()), 
                           Form("%s>0", det_formula.c_str()), "goff");

            // Add to summed histogram
            summed_hists[det_name]->Add(htmp);

            delete htmp;
        }

        readFile->Close();
    }

    // === Save results ===
    TString output_dir = "/felsdata/n14ag_Felsenkeller_2025/Anup/Run4617/";
    gSystem->mkdir(output_dir, true);
    TString output_rootfile = output_dir + "Addback_tree.root";
    TFile* outFile = new TFile(output_rootfile, "RECREATE");

    for (auto& det_pair : summed_hists) {
        det_pair.second->Write();
        delete det_pair.second;
    }

    outFile->Close();
    std::cout << "Summed histograms saved to: " << output_rootfile << std::endl;
}
