#include "TFile.h"
#include "TDirectory.h"
#include "TH1F.h"
#include "TKey.h"
#include <iostream>
#include <string>

void readRootFile() {
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

    // Loop over all objects in the directory
    TIter next(dir->GetListOfKeys());
    TKey* key;
    while ((key = (TKey*)next())) {
        // Check if the key is a TH1F histogram
        if (std::string(key->GetClassName()) == "TH1F") {
            TH1F* hist = (TH1F*)key->ReadObj();
            std::cout << "Histogram: " << hist->GetName() << " has " 
                      << hist->GetNbinsX() << " bins." << std::endl;

            // Example: Print first 5 bin contents
            std::cout << "  First 5 bin contents: ";
            for (int i = 1; i <= std::min(5, hist->GetNbinsX()); i++) {
                std::cout << hist->GetBinContent(i) << " ";
            }
            std::cout << std::endl;
        }
    }

    // Close the file
    file->Close();
    std::cout << "ROOT file processing complete." << std::endl;
}

// RUN IN ROOT:
// .L readRootFile.C
// readRootFile();
