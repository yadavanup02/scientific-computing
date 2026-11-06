// Addback Tree analysis
#include "TFile.h"
#include "TTree.h"
#include "TH1F.h"
#include "TCanvas.h"
#include "TString.h"

#include "TFile.h"
#include "TTree.h"
#include "TH1F.h"
#include "TString.h"
#include "TSystem.h"
#include <iostream>
#include <vector>

void antiCompton_EB18G_mAdd() {
    // List of runs
    // std::vector<int> runs = {
    //     4616, 4618, 4620, 4622, 4624, 4696, 4698, 4699, 4702, 4703,
    //     4705, 4708, 4710, 4712, 4715, 4718, 4720, 4722
    // }; 
    std::vector<int> runs = {
        4562
    };  

    // Output directory and file for combined spectrum
    TString output_dir = "/felsdata/n14ag_Felsenkeller_2025/Anup/Run4562/";
    gSystem->mkdir(output_dir, true);
    TString output_rootfile = output_dir + "EB18G_AntiCompton_4562.root";

    // Histogram to hold the sum
    TH1F* hCombined = new TH1F("EB18_G_AntiCompton",
                               "EB18_G Anti-Compton Combined;Energy (MeV);Counts",
                               3000, 0, 6.0);
    // hCombined->Sumw2(); // proper error handling

    // Anti-Compton cut string
    TString cut = "(MeV06 > 0 && MeV01 == 0 && MeV02 == 0 && MeV03 == 0 && MeV04 == 0 && MeV05 == 0 && MeV00 == 0)";

    // Loop over runs
    for (auto run_number : runs) {
        TString input_file = TString::Format("/felsdata/n14ag_Felsenkeller_2025/Rootfiles_high/Run%d_high_Tree.root", run_number);
        TString run_output_dir = TString::Format("/felsdata/n14ag_Felsenkeller_2025/Anup/Run%d/", run_number);
        gSystem->mkdir(run_output_dir, true);
        TString run_output_file = run_output_dir + "EB18G_AntiCompton.root";

        TFile* readFile = TFile::Open(input_file, "READ");
        if (!readFile || readFile->IsZombie()) {
            std::cerr << "Error: Could not open file " << input_file << std::endl;
            continue;
        }

        TTree* felsTree = (TTree*)readFile->Get("Tree/felsTree");
        if (!felsTree) {
            std::cerr << "Error: Could not find tree in " << input_file << std::endl;
            readFile->Close();
            continue;
        }

        // Per-run histogram
        TString hname = TString::Format("EB18_G_AntiCompton_Run%d", run_number);
        TH1F* hRun = new TH1F(hname, Form("EB18_G Anti-Compton Run %d;Energy (MeV);Counts", run_number), 3000, 0, 6.0);
        // hRun->Sumw2();

        // Fill histogram with cut
        felsTree->Draw(Form("MeV06 >> %s", hname.Data()), cut, "goff");

        // Save individual histogram
        TFile* outRun = new TFile(run_output_file, "RECREATE");
        hRun->Write();
        outRun->Close();

        // Add to combined histogram
        hCombined->Add(hRun);

        std::cout << "Processed Run " << run_number << std::endl;

        readFile->Close();
    }

    // Save combined histogram
    TFile* outFile = new TFile(output_rootfile, "RECREATE");
    hCombined->Write();

    outFile->Close();

    std::cout << "Combined Anti-Compton spectrum saved to " << output_rootfile << std::endl;
}
