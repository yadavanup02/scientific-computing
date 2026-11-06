// Addback Tree analysis
#include "TFile.h"
#include "TTree.h"
#include "TH1F.h"
#include "TCanvas.h"
#include "TString.h"

void antiCompton_addback(int run_number) {
    // Input and output files
    TString input_file = TString::Format("/felsdata/n14ag_Felsenkeller_2025/Rootfiles_high/Run%d_high_Tree.root", run_number);
    TString output_dir = TString::Format("/felsdata/n14ag_Felsenkeller_2025/Anup/Run%d/", run_number);
    gSystem->mkdir(output_dir, true);
    TString output_rootfile = output_dir + "EB18G_AntiCompton.root";

    // Open file
    TFile* readFile = TFile::Open(input_file, "READ");
    if (!readFile || readFile->IsZombie()) {
        std::cerr << "Error: Could not open ROOT file for run " << run_number << std::endl;
        return;
    }

    TTree* felsTree = (TTree*)readFile->Get("Tree/felsTree");
    if (!felsTree) {
        std::cerr << "Error: Could not find tree in run " << run_number << std::endl;
        readFile->Close();
        return;
    }

    // Create histogram
    TH1F* hEB18G = new TH1F("EB18_G_AntiCompton",
                            "EB18_G Anti-Compton;Energy (MeV);Counts",
                            6000, 0, 6.0);

    // Draw with anti-Compton condition
    TString cut = "EB18_G > 0 && EB18_A == 0 && EB18_B == 0 && EB18_C == 0 && EB18_D == 0 && EB18_E == 0 && EB18_F == 0";
    felsTree->Draw("EB18_G >> EB18_G_AntiCompton", cut, "goff");

    // Save result
    TFile* outFile = new TFile(output_rootfile, "RECREATE");
    hEB18G->Write();
    outFile->Close();

    readFile->Close();

    std::cout << "Anti-Compton histogram for EB18_G saved to " << output_rootfile << std::endl;
}
