#include "TFile.h"
#include "TCanvas.h"
#include "TH1F.h"
#include "TF1.h"
#include "TLine.h"
#include <iostream>
#include <fstream>

void peakCount_v1() {
    // Define run number for output file
    int run_number = 4189;

    // Open the ROOT file and retrieve the calibrated energy histogram
    TFile* file = TFile::Open(("Calib_Run" + std::to_string(run_number) + "_Ron100.root").c_str(), "READ"); 
    if (!file || file->IsZombie()) {
        std::cerr << "Error: Could not open ROOT file!" << std::endl;
        return;
    }

    // Retrieve the calibrated energy histogram
    TH1F* energyHist = (TH1F*)file->Get("energyHist");
    if (!energyHist) {
        std::cerr << "Error: Could not find 'energyHist'!" << std::endl;
        file->Close();
        return;
    }

    // Define ROIs for three peaks
    struct Peak {
        double peak_energy;
        int bin_min, bin_max;
        int bkg_left_min, bkg_left_max;
        int bkg_right_min, bkg_right_max;
    };

    // Example peak settings (You can modify them)
    std::vector<Peak> peaks = {
        {1081, energyHist->FindBin(1072), energyHist->FindBin(1090),
               energyHist->FindBin(1055), energyHist->FindBin(1070),
               energyHist->FindBin(1095), energyHist->FindBin(1110)},

        {2471, energyHist->FindBin(2455), energyHist->FindBin(2485),
              energyHist->FindBin(2440), energyHist->FindBin(2454),
              energyHist->FindBin(2490), energyHist->FindBin(2505)},

        {4524, energyHist->FindBin(4505), energyHist->FindBin(4550),
               energyHist->FindBin(4470), energyHist->FindBin(4500),
               energyHist->FindBin(4610), energyHist->FindBin(4640)}
    };

    // Open text file to store peak counts
    std::ofstream outFile("PeakCounts_Run" + std::to_string(run_number) + ".txt");
    outFile << "Run Number: " << run_number << "\n";
    outFile << "Peak Energy (keV) | Gross Counts | Background Counts | Net Counts\n";
    outFile << "-------------------------------------------------------------\n";

    // Create canvas for plotting
    TCanvas* c1 = new TCanvas("c1", "Peak1", 800, 600);
    c1->cd();
    energyHist->GetXaxis()->SetRangeUser(1050, 1110); // Range for the first peak
    energyHist->SetLineColor(kBlue);
    energyHist->Draw("HIST");

    TCanvas* c2 = new TCanvas("c2", "Peak2", 800, 600);
    c2->cd();
    energyHist->GetXaxis()->SetRangeUser(2440, 2510); // Range for the second peak
    energyHist->SetLineColor(kBlue);
    energyHist->Draw("HIST");

    TCanvas* c3 = new TCanvas("c3", "Peak3", 800, 600);
    c3->cd();
    energyHist->GetXaxis()->SetRangeUser(4470, 4640); // Range for the third peak
    energyHist->SetLineColor(kBlue);
    energyHist->Draw("HIST");

    // Loop over peaks and process them
    for (const auto& peak : peaks) {
        double peak_counts = energyHist->Integral(peak.bin_min, peak.bin_max);
        double bkg_left_counts = energyHist->Integral(peak.bkg_left_min, peak.bkg_left_max);
        double bkg_right_counts = energyHist->Integral(peak.bkg_right_min, peak.bkg_right_max);
        double bkg_left_avg = bkg_left_counts / (peak.bkg_left_max - peak.bkg_left_min + 1);
        double bkg_right_avg = bkg_right_counts / (peak.bkg_right_max - peak.bkg_right_min + 1);
        double avg_bkg_per_bin = (bkg_left_avg + bkg_right_avg) / 2.0;
        double net_peak_counts = peak_counts - avg_bkg_per_bin * (peak.bin_max - peak.bin_min + 1);

        // Write results to text file
        outFile << peak.peak_energy << " keV | "
                << peak_counts << " | "
                << avg_bkg_per_bin * (peak.bin_max - peak.bin_min + 1) << " | "
                << net_peak_counts << "\n";

        std::cout << "Peak at " << peak.peak_energy << " keV:\n";
        std::cout << "  Gross counts: " << peak_counts << "\n";
        std::cout << "  Background counts: " << avg_bkg_per_bin * (peak.bin_max - peak.bin_min + 1) << "\n";
        std::cout << "  Net counts: " << net_peak_counts << "\n\n";

        // Draw lines for peak ROI
        TLine* line1 = new TLine(peak.bin_min, 0, peak.bin_min, energyHist->GetMaximum() * 0.5);
        TLine* line2 = new TLine(peak.bin_max, 0, peak.bin_max , energyHist->GetMaximum() * 0.5);
        line1->SetLineColor(kRed);
        line2->SetLineColor(kRed);
        line1->Draw("same");
        line2->Draw("same");

        // Draw lines for background regions
        TLine* bkg1 = new TLine(peak.bkg_left_min, 0, peak.bkg_left_min, energyHist->GetMaximum() * 0.3);
        TLine* bkg2 = new TLine(peak.bkg_left_max, 0, peak.bkg_left_max, energyHist->GetMaximum() * 0.3);
        TLine* bkg3 = new TLine(peak.bkg_right_min, 0, peak.bkg_right_min, energyHist->GetMaximum() * 0.3);
        TLine* bkg4 = new TLine(peak.bkg_right_max, 0, peak.bkg_right_max, energyHist->GetMaximum() * 0.3);

        bkg1->SetLineColor(kGreen);
        bkg2->SetLineColor(kGreen);
        bkg3->SetLineColor(kGreen);
        bkg4->SetLineColor(kGreen);

        bkg1->Draw("same");
        bkg2->Draw("same");
        bkg3->Draw("same");
        bkg4->Draw("same");
    }

    //outFile.close();
    c1->Update();
    c2->Update();
    c3->Update();
    // Save canvases as images
    c1->SaveAs(("Peak1_Run" + std::to_string(run_number) + ".png").c_str());
    c2->SaveAs(("Peak2_Run" + std::to_string(run_number) + ".png").c_str());
    c3->SaveAs(("Peak3_Run" + std::to_string(run_number) + ".png").c_str());

    // Close the ROOT file
    //file->Close();
}
