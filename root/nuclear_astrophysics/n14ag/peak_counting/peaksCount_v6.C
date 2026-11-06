#include "TFile.h"
#include "TCanvas.h"
#include "TH1F.h"
#include "TF1.h"
#include "TLine.h"
#include <iostream>
#include <fstream>
#include <sys/stat.h>
#include <sys/types.h>

void peaksCount_v6() {
    int run_number = 4340;

    TFile* file = TFile::Open(("/felsdata/n14ag_Felsenkeller_2025/Rootfiles_high/Run" + std::to_string(run_number) + "_high.root").c_str(), "READ"); 
    if (!file || file->IsZombie()) {
        std::cerr << "Error: Could not open ROOT file!" << std::endl;
        return;
    }

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

    // Display region in energy units
    double display_min_keV = 320;
    double display_max_keV = 380;

    // Define ROI and background window sizes (in keV)
    double roi_left_offset = -10;
    double roi_right_offset = +9;
    double bkg_left_start_offset = -25;
    double bkg_left_end_offset = -15;
    double bkg_right_start_offset = +10;
    double bkg_right_end_offset = +19;

    std::string output_dir = "/felsdata/n14ag_Felsenkeller_2025/Anup/Output_" + std::to_string(run_number);
    struct stat info;
    if (stat(output_dir.c_str(), &info) != 0) {
        mkdir(output_dir.c_str(), 0777);
    }

    std::ofstream outFile(output_dir + "/PeakCounts_v6_Run" + std::to_string(run_number) + ".txt");
    outFile << "Run Number:\t" << run_number << "\n";
    outFile << "Detector\tGross Counts\tNet Counts\tPeak Position (keV)\tFWHM (keV)\n";
    outFile << "-------------------------------------------------------------------------------\n";

    std::vector<TCanvas*> canvases;
    int canvas_index = 1;

    for (size_t i = 0; i < energyHists.size(); ++i) {
        TH1F* hist = energyHists[i];
        if (!hist) continue;

        // Peak search in display energy range
        int bin_search_min = hist->FindBin(display_min_keV);
        int bin_search_max = hist->FindBin(display_max_keV);
        int peak_bin = bin_search_min;
        for (int b = bin_search_min; b <= bin_search_max; ++b) {
            if (hist->GetBinContent(b) > hist->GetBinContent(peak_bin)) {
                peak_bin = b;
            }
        }
        double peak_energy = hist->GetBinCenter(peak_bin);

        // Define ROI and BKG based on energy
        double roi_min = peak_energy + roi_left_offset;
        double roi_max = peak_energy + roi_right_offset;
        double bkg_left_min = peak_energy + bkg_left_start_offset;
        double bkg_left_max = peak_energy + bkg_left_end_offset;
        double bkg_right_min = peak_energy + bkg_right_start_offset;
        double bkg_right_max = peak_energy + bkg_right_end_offset;

        int roi_bin_min = hist->FindBin(roi_min);
        int roi_bin_max = hist->FindBin(roi_max);
        int bkg_left_bin_min = hist->FindBin(bkg_left_min);
        int bkg_left_bin_max = hist->FindBin(bkg_left_max);
        int bkg_right_bin_min = hist->FindBin(bkg_right_min);
        int bkg_right_bin_max = hist->FindBin(bkg_right_max);

        int roi_bins = roi_bin_max - roi_bin_min + 1;
        double peak_counts = hist->Integral(roi_bin_min, roi_bin_max);
        double bkg_left_counts = hist->Integral(bkg_left_bin_min, bkg_left_bin_max);
        double bkg_right_counts = hist->Integral(bkg_right_bin_min, bkg_right_bin_max);

        double bkg_left_density = bkg_left_counts / (bkg_left_bin_max - bkg_left_bin_min + 1);
        double bkg_right_density = bkg_right_counts / (bkg_right_bin_max - bkg_right_bin_min + 1);

        double total_interp_bkg = 0.0;
        for (int j = 0; j < roi_bins; ++j) {
            double interp_bkg = bkg_left_density + (j / (double)(roi_bins - 1)) * (bkg_right_density - bkg_left_density);
            total_interp_bkg += interp_bkg;
        }

        double net_counts = peak_counts - total_interp_bkg;

        // Draw
        TCanvas* c = new TCanvas(("c" + std::to_string(canvas_index)).c_str(), ("Peak" + std::to_string(canvas_index)).c_str(), 800, 600);
        canvases.push_back(c);
        c->cd();

        TH1F* hclone = (TH1F*)hist->Clone(("hclone_" + std::to_string(canvas_index)).c_str());
        hclone->SetLineColor(kBlue);
        hclone->SetTitle(("Peak ~" + std::to_string((int)peak_energy) + " keV; Energy [keV]; Counts").c_str());
        hclone->GetXaxis()->SetRangeUser(peak_energy-40, peak_energy+40);
        hclone->Draw();

        // Fit with gaus + pol0
        TF1* fitFunc = new TF1("fitFunc", "gaus + pol0(3)", roi_min, roi_max);
        fitFunc->SetParameters(hist->GetBinContent(peak_bin), peak_energy, 2, 10);
        hclone->Fit(fitFunc, "RQ");

        double fit_peak_pos = fitFunc->GetParameter(1);
        double sigma = fitFunc->GetParameter(2);
        double FWHM = 2.355 * sigma;

        outFile << detector_names[i] << " \t " << peak_counts << " \t" << net_counts << "  \t " << fit_peak_pos << " \t " << FWHM << "\n";

        auto drawLine = [&](double x, int color) {
            TLine* line = new TLine(x, 0, x, hclone->GetMaximum() * 0.3);
            line->SetLineColor(color);
            line->Draw("same");
        };

        drawLine(roi_min, kRed); drawLine(roi_max, kRed);
        drawLine(bkg_left_min, kGreen); drawLine(bkg_left_max, kGreen);
        drawLine(bkg_right_min, kGreen); drawLine(bkg_right_max, kGreen);

        c->SetLogy();
        c->Update();
        c->SaveAs((output_dir + "/peak_356_v6_" + detector_names[i] + "_Run" + std::to_string(run_number) + ".png").c_str());
        canvas_index++;
    }
}
