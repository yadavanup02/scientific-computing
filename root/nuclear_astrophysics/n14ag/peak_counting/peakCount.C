#include "TFile.h"
#include "TCanvas.h"
#include "TH1F.h"
#include "TF1.h"
#include "TLine.h"
#include <iostream>

void peakCount() {
    // Open the ROOT file and retrieve the calibrated energy histogram
    TFile* file = TFile::Open("Calib_Run4176_Ron100.root", "READ"); // Update with correct filename
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

    // Define ROI for the peak at 4524 keV
    int bin_min = energyHist->FindBin(4505);
    int bin_max = energyHist->FindBin(4550);

    // Background ROIs
    int bkg_left_min = energyHist->FindBin(4470);
    int bkg_left_max = energyHist->FindBin(4500);
    int bkg_right_min = energyHist->FindBin(4610);
    int bkg_right_max = energyHist->FindBin(4640);

    // Integrate over the peak ROI
    double peak_counts = energyHist->Integral(bin_min, bin_max);

    // Estimate background from both side ROIs
    double bkg_left_counts = energyHist->Integral(bkg_left_min, bkg_left_max);
    double bkg_right_counts = energyHist->Integral(bkg_right_min, bkg_right_max);
    double bkg_left_avg = bkg_left_counts / (bkg_left_max - bkg_left_min + 1);
    double bkg_right_avg = bkg_right_counts / (bkg_right_max - bkg_right_min + 1);
    double avg_bkg_per_bin = (bkg_left_avg + bkg_right_avg) / 2.0;

    // Subtract background
    double net_peak_counts = peak_counts - avg_bkg_per_bin * (bin_max - bin_min + 1);

    std::cout << "Gross peak counts: " << peak_counts << std::endl;
    std::cout << "Estimated background counts: " << avg_bkg_per_bin * (bin_max - bin_min + 1) << std::endl;
    std::cout << "Net peak counts: " << net_peak_counts << std::endl;

    // Plot the spectrum with ROIs
    TCanvas* c1 = new TCanvas("c1", "Energy Spectrum Analysis", 800, 600);
    energyHist->SetLineColor(kBlue);
    energyHist->Draw("HIST");
    energyHist->GetXaxis()->SetRangeUser(4460, 4660);

    // Draw ROI and background regions
    TLine* line1 = new TLine(4505, 0, 4505, energyHist->GetMaximum() * 0.5);
    TLine* line2 = new TLine(4550, 0, 4550, energyHist->GetMaximum() * 0.5);
    TLine* bkg1 = new TLine(4470, 0, 4470, energyHist->GetMaximum() * 0.3);
    TLine* bkg2 = new TLine(4500, 0, 4500, energyHist->GetMaximum() * 0.3);
    TLine* bkg3 = new TLine(4610, 0, 4610, energyHist->GetMaximum() * 0.3);
    TLine* bkg4 = new TLine(4640, 0, 4640, energyHist->GetMaximum() * 0.3);

    line1->SetLineColor(kRed);
    line2->SetLineColor(kRed);
    bkg1->SetLineColor(kGreen);
    bkg2->SetLineColor(kGreen);
    bkg3->SetLineColor(kGreen);
    bkg4->SetLineColor(kGreen);

    line1->Draw("same");
    line2->Draw("same");
    bkg1->Draw("same");
    bkg2->Draw("same");
    bkg3->Draw("same");
    bkg4->Draw("same");

    c1->Update();

    //file->Close();
}
