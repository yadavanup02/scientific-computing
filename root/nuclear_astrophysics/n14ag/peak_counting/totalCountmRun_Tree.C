#include "TFile.h"
#include "TCanvas.h"
#include "TH1F.h"
#include "TF1.h"
#include "TLine.h"
#include "TSpectrum.h"
#include "TTree.h"
#include "TMarker.h"
#include <iostream>
#include <fstream>
#include <iomanip>
#include <sys/stat.h>
#include <sys/types.h>


void totalCountmRun_Tree(int first_run, int last_run) {
    
    // loop over run numbers 
    int step=1;
    for (int run_number = first_run; run_number <= last_run; run_number += step){
        //int rebin_factor = 2;
        TFile* file = TFile::Open(("/felsdata/n14ag_Felsenkeller_2025/Rootfiles_high/Run" + std::to_string(run_number) + "_high_Tree.root").c_str(), "READ"); 
        if (!file || file->IsZombie()) {
            std::cerr << "Error: Could not open ROOT file!" << std::endl;
            return;
        }
        
        std::map<std::string, std::string> detector_map = {
            {"MeV00", "EB18_A"}, {"MeV01", "EB18_B"}, {"MeV02", "EB18_C"},
            {"MeV03", "EB18_D"}, {"MeV04", "EB18_E"}, {"MeV05", "EB18_F"},
            {"MeV06", "EB18_G"}, 
            {"MeV23", "MB1_1"}, {"MeV24", "MB1_2"}, {"MeV25", "MB1_3"}, 
            {"MeV07", "MB2_1"}, {"MeV08", "MB2_2"}, {"MeV09", "MB2_3"}
            
        };

        std::vector<std::string> detector_names;
        for (const auto& pair : detector_map) {
            detector_names.push_back(pair.first);
        }


        // Create output directory
        std::string output_dir = "/felsdata/n14ag_Felsenkeller_2025/Anup/Efficiency/Run" + std::to_string(run_number);
        struct stat info;
        if (stat(output_dir.c_str(), &info) != 0) {
            mkdir(output_dir.c_str(), 0777);
        }

    // Create output files
        std::string filename = output_dir + "/Total_Counts_Run" + std::to_string(run_number) + ".txt";
        std::ofstream output_file(filename);
        if (!output_file) {
            std::cerr << "Error: Could not create output file!" << std::endl;
            return;
        }
        output_file << "Run Number: " << run_number << std::endl;
        output_file << "Detector Name\tTotal Counts" << std::endl;
        output_file << "----------------------------------------" << std::endl;

        // Process each detector
        for (const auto& detector : detector_names) {
            TTree* tree = (TTree*)file->Get("Tree/felsTree");
            if (!tree) {
                std::cerr << "Warning: Could not find 'Tree/felsTree'!" << std::endl;
                continue;
            }

            if (!tree->GetBranch(detector.c_str())) {
                std::cerr << "Warning: Branch '" << detector << "' does not exist in the tree!" << std::endl;
                continue;
            }
            double value = 0.0;
            if (tree->SetBranchAddress(detector.c_str(), &value) < 0) {
                std::cerr << "Warning: Could not set branch address for '" << detector << "'!" << std::endl;
                continue;
            }
            
            Long64_t nEntries = tree->GetEntries();
            double totalCounts = 0.0;

            for (Long64_t i = 0; i < nEntries; ++i) {
                tree->GetEntry(i);
                totalCounts += value;
            }
            std::string human_readable_name = detector_map[detector];

            output_file << detector_map[detector] << "\t" << totalCounts << std::endl;
            std::cout << "Detector: " << human_readable_name << ", Total Counts: " << totalCounts << std::endl;
           
        }
        output_file.close();
        file->Close();
    }
    
    //     TString output_file1 = TString::Format("/felsdata/n14ag_Felsenkeller_2025/Anup/Efficiency/Run%d/Ntot_Counts_Run%d.txt", run_number, run_number);
    //     std::ofstream output_file2(output_file1.Data());
    //     if (!output_file2) {
    //         std::cerr << "Error: Could not create output file!" << std::endl;
    //         return;
    //     }



    //     TString input_file  = TString::Format("/felsdata/n14ag_Felsenkeller_2025/Anup/Efficiency/Run%d/Total_Counts_Run%d.txt", run_number, run_number);
    //     std::ifstream infile(input_file.Data());
    //     if (!infile.is_open()) {
    //         std::cerr << "Error opening file: " << input_file.Data() << std::endl;
    //         continue;
    //     }
    //     std::string line;
    //     while (std::getline(infile, line)) {
    //         if (line.find("Detector") != std::string::npos || line.find("Run Number") != std::string::npos || line.find("---") != std::string::npos) {
    //             continue;
    //         }
    //         std::stringstream ss(line);
    //         std::string detector;
    //         double totalCounts;
    //         ss >> detector >> totalCounts;
    //         double A = 18.223 * 1000; 

    //         double N_Co, t_co, t_bg, N_bg;
    //         double N_tot = N_Co - ( t_co/t_bg ) * N_bg;
    //         double eta = N_tot / (2 * A * t_co);

    //         output_file2 << detector << "\t" << N_tot << "\t" << eta << std::endl;
    //         std::cout << "Detector: " << detector << ", Total Counts: " << N_tot << ", Efficiency: " << eta << std::endl;


    // }

    // std::cout << "Total Co-60 Counts:" << N_tot << std::endl;
    // std::cout << "Efficiency:" << eta << std::endl;
}
    