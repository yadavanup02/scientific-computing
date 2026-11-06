#include "TFile.h"
#include "TDirectory.h"
#include "TH1F.h"
#include "TKey.h"
#include "TCanvas.h"
#include "TStyle.h"
#include <iostream>
#include <string>

void readRootFile_v1() {
    int run_number = 4186;  // Replace with your run number
    std::string rootFilename = "/felsdata/n14ag_Felsenkeller_2025/Rootfiles_high/Run" + std::to_string(run_number) + "_high.root";

    // Open the ROOT file
    TFile* file = TFile::Open(rootFilename.c_str(), "READ");
    if (!file || file->IsZombie()) {
        std::cerr << "Error: Could not open file " << rootFilename << std::endl;
        return;
    }
    std::cout << "Opened ROOT file successfully: " << rootFilename << std::endl;

    // Access the "Energies" directory
    TDirectory* dir = (TDirectory*)file->Get("Energies");
    if (!dir) {
        std::cerr << "Error: Could not find 'Energies' directory in the file." << std::endl;
        file->Close();
        return;
    }
    std::cout << "Found 'Energies' directory. Listing histograms...\n";

    // Retrieve the "Ron100" histogram
    TH1F* ron100Hist = (TH1F*)dir->Get("Ron100");
    if (!ron100Hist) {
        std::cerr << "Error: Could not find histogram 'Ron100'." << std::endl;
        file->Close();
        return;
    }

    std::cout << "Found histogram 'Ron100' with " << ron100Hist->GetNbinsX() 
              << " bins. First 5 bin contents: ";
    
    // Print first 5 bins (to check for nonzero values)
    for (int i = 1; i <= 5; i++) {
        std::cout << ron100Hist->GetBinContent(i) << " ";
    }
    std::cout << std::endl;

    // Create a canvas
    TCanvas* canvas = new TCanvas("canvas", "Ron100 Histogram", 800, 600);

    // Set ROOT style
    gStyle->SetOptStat(0);  // Hide statistics box
    gStyle->SetTitleFontSize(0.04);

    // Customize the histogram appearance
    ron100Hist->SetLineColor(kBlue);
    ron100Hist->SetLineWidth(2);
    ron100Hist->SetFillColorAlpha(kBlue, 0.3);  // Semi-transparent fill
    ron100Hist->SetTitle("Ron100 Energy Spectrum; Energy (keV); Counts");

    // Set log scale if necessary (uncomment if needed)
    // canvas->SetLogy();

    // Draw the histogram
    ron100Hist->Draw("HIST");

    // Update canvas and keep it open
    canvas->Update();
}
