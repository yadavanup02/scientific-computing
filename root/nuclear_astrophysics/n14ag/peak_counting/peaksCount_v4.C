#include "TFile.h"
#include "TCanvas.h"
#include "TH1F.h"
#include "TF1.h"
#include "TLine.h"
#include <iostream>
#include <fstream>
#include <sys/stat.h>
#include <sys/types.h>
//  this function reads the calibrated energy histogram from a ROOT file and calculates the peak counts
// ROI are set for the 1618 keV resonance for three differnt peaks 1081 (secondary), 2539 (primary), 4592 (primary) keV

void peaksCount_v4() {
    // Define run number for output file
    int run_number = 4320;

    // Open the ROOT file and retrieve the calibrated energy histogram
    TFile* file = TFile::Open(("/felsdata/n14ag_Felsenkeller_2025/Rootfiles_high/Run" + std::to_string(run_number) + "_high.root").c_str(), "READ"); 
    if (!file || file->IsZombie()) {
        std::cerr << "Error: Could not open ROOT file!" << std::endl;
        return;
    }

    // Retrieve the calibrated energy histogram
    std::vector<std::string> detector_names = {
        "EB18_A", "EB18_B", "EB18_C", "EB18_D", "EB18_E", "EB18_F", "EB18_G",
        "MB1_1", "MB1_2", "MB1_3", "MB2_1", "MB2_2", "MB2_3"
    };

    std::vector<TH1F*> energyHists;

    for (const auto& detector : detector_names) {
        TH1F* hist = (TH1F*)file->Get(("Energies/" + detector).c_str());
        if (!hist) {
            std::cerr << "Error: Could not find '" << detector << "' histogram!" << std::endl;
            continue;
        }
        energyHists.push_back(hist);
    }

    if (energyHists.empty()) {
        std::cerr << "Error: No valid histograms found!" << std::endl;
        file->Close();
        return;
    }

    // Define ROIs for three peaks
    struct peak {
        int peak_energy;
        int bin_min, bin_max;
        int bkg_left_min, bkg_left_max;
        int bkg_right_min, bkg_right_max;
        int x_min, x_max;  // X-axis range for plotting
    };

    // for (const auto& peak : peaks) {
    //     {1332.5, 
    //     1300, 1360,
    //     1270, 1290,
    //     1370, 1390,
    //     1260, 1400}
    // };
    
    std::vector<peak> peaks = {
        {1335, 1310, 1365,
            1270, 1290,
            1370, 1390,
            1260, 1400}
    };

    // Create output directory if it does not exist
    std::string output_dir = "/felsdata/n14ag_Felsenkeller_2025/Anup/Output_" + std::to_string(run_number);
    struct stat info;
    if (stat(output_dir.c_str(), &info) != 0) {
        mkdir(output_dir.c_str(), 0777);
    }

    // Open text file to store peak counts
    std::ofstream outFile(output_dir + "/PeakCounts_v4_Run" + std::to_string(run_number) + ".txt");
    outFile << "Run Number:\t" << run_number << "\n";
    outFile << "Detector\tGross Counts\tNet CountsPeak\tFitted Position (keV)\tFWHM (keV)\n";
    outFile << "-------------------------------------------------------------------------------\n";

    std::vector<TCanvas*> canvases; // Store canvases
    int canvas_index = 1; // To differentiate canvas names

    // Loop over peaks and process them
    // Loop through all detectors for the single peak
    for (size_t i = 0; i < energyHists.size(); ++i) {
        TH1F* energyHist = energyHists[i];
        if (!energyHist) continue;

        for (const auto& peak : peaks) {
    
            double peak_counts = energyHist->Integral(energyHist->FindBin(peak.bin_min), energyHist->FindBin(peak.bin_max));
            double bkg_left_counts = energyHist->Integral(energyHist->FindBin(peak.bkg_left_min), energyHist->FindBin(peak.bkg_left_max));
            double bkg_right_counts = energyHist->Integral(energyHist->FindBin(peak.bkg_right_min), energyHist->FindBin(peak.bkg_right_max));
            double bkg_left_avg = bkg_left_counts / (energyHist->FindBin(peak.bkg_left_max) - energyHist->FindBin(peak.bkg_left_min) + 1);
            double bkg_right_avg = bkg_right_counts / (energyHist->FindBin(peak.bkg_right_max) - energyHist->FindBin(peak.bkg_right_min) + 1);
            double avg_bkg_per_bin = (bkg_left_avg + bkg_right_avg) / 2.0;
            double net_peak_counts = peak_counts - avg_bkg_per_bin * (energyHist->FindBin(peak.bin_max) - energyHist->FindBin(peak.bin_min) + 1);

            // Create a new canvas for each peak
            TCanvas* c = new TCanvas(("c" + std::to_string(canvas_index)).c_str(), ("Peak" + std::to_string(canvas_index)).c_str(), 800, 600);
            canvases.push_back(c);
            c->cd();

            // Clone histogram so that SetRangeUser() does not affect the others
            TH1F* hist_clone = (TH1F*)energyHist->Clone(("hist_clone_" + std::to_string(canvas_index)).c_str());
            hist_clone->GetXaxis()->SetRangeUser(peak.x_min, peak.x_max);
            hist_clone->SetLineColor(kBlue);
            hist_clone->SetTitle(("Co-60 peak in " + detector_names[i] + " keV; Energy [keV]; Counts").c_str());
            hist_clone->Draw("");

            // Fit function (Gaussian + Linear Background)
            TF1* fitFunc = new TF1(("fitFunc" + std::to_string(canvas_index)).c_str(),
                                "gaus + pol3(3)", peak.bin_min, peak.bin_max);
            
            // Initial parameters for the fit
            fitFunc->SetParameter(1, peak.peak_energy);  // Mean
            fitFunc->SetParameter(2, 5);  // Initial sigma guess

            hist_clone->Fit(fitFunc, "RQ");

            // Extract peak position and FWHM
            double peak_position = fitFunc->GetParameter(1);
            double sigma = fitFunc->GetParameter(2);
            double FWHM = 2.355 * sigma;

            // Write results to text file as tab-separated columns
            outFile << std::fixed << std::setprecision(1); // Set precision to 2 decimal points
            outFile << detector_names[i] << "\t"
                    << peak_counts << "\t"
                    << net_peak_counts << "\t" 
                    << peak_position << "\t"
                    << FWHM << "\n";


            std::cout << "Peak at " << peak.peak_energy << " keV:\n";
            std::cout << "  Peak position: " << peak_position << "\n";
            std::cout << "  FWHM: " << FWHM << "\n";
            std::cout << "  Gross counts: " << peak_counts << "\n";
            std::cout << "  Background counts: " << avg_bkg_per_bin * (energyHist->FindBin(peak.bin_max) - energyHist->FindBin(peak.bin_min) + 1) << "\n";
            std::cout << "  Net counts: " << net_peak_counts << "\n\n";

            // Draw lines for peak ROI
            TLine* line1 = new TLine(peak.bin_min, 0, peak.bin_min, hist_clone->GetMaximum() * 0.3);
            TLine* line2 = new TLine(peak.bin_max, 0, peak.bin_max, hist_clone->GetMaximum() * 0.3);
            line1->SetLineColor(kRed);
            line2->SetLineColor(kRed);
            line1->Draw("same");
            line2->Draw("same");

            // Draw lines for background regions
            TLine* bkg1 = new TLine(peak.bkg_left_min, 0, peak.bkg_left_min, hist_clone->GetMaximum() * 0.3);
            TLine* bkg2 = new TLine(peak.bkg_left_max, 0, peak.bkg_left_max, hist_clone->GetMaximum() * 0.3);
            TLine* bkg3 = new TLine(peak.bkg_right_min, 0, peak.bkg_right_min, hist_clone->GetMaximum() * 0.3);
            TLine* bkg4 = new TLine(peak.bkg_right_max, 0, peak.bkg_right_max, hist_clone->GetMaximum() * 0.3);

            bkg1->SetLineColor(kGreen);
            bkg2->SetLineColor(kGreen);
            bkg3->SetLineColor(kGreen);
            bkg4->SetLineColor(kGreen);

            bkg1->Draw("same");
            bkg2->Draw("same");
            bkg3->Draw("same");
            bkg4->Draw("same");

            c->SetLogy();
            c->Update();
            c->SaveAs((output_dir + "/Peak_v4" + std::to_string(canvas_index) + "_Run" + std::to_string(run_number) + ".png").c_str());

            canvas_index++;
        }
    }

    // Close output file
    outFile.close();

    // Close the ROOT file
    //file->Close();
}
