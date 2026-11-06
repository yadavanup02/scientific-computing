#include "TFile.h"
#include "TCanvas.h"
#include "TH1F.h"
#include "TF1.h"
#include "TLine.h"
#include <iostream>
#include <fstream>
#include <sys/stat.h>
#include <sys/types.h>

void peaksCount_v5() {
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
        {1335, 1310, 1360,
            1280, 1300,
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
    std::ofstream outFile(output_dir + "/PeakCounts_cut_count_Run" + std::to_string(run_number) + ".txt");
    outFile << "Run Number:\t" << run_number << "\n";
    outFile << "Detector\tGross Counts\tNet CountsPeak\tFitted Position (keV)\tFWHM (keV)\n";
    outFile << "-------------------------------------------------------------------------------\n";

    std::vector<TCanvas*> canvases; // Store canvases
    int canvas_index = 1; // To differentiate canvas names

    // Loop through all detectors for the single peak
    for (size_t i = 0; i < energyHists.size(); ++i) {
        TH1F* energyHist = energyHists[i];
        if (!energyHist) continue;

        // Only one peak is used here
        peak p = peaks[0];

        int bin_start = energyHist->FindBin(p.bin_min);
        int bin_end   = energyHist->FindBin(p.bin_max);
        int signal_bins = bin_end - bin_start + 1;

        int bkg_left_start  = energyHist->FindBin(p.bkg_left_min);
        int bkg_left_end    = energyHist->FindBin(p.bkg_left_max);
        int bkg_right_start = energyHist->FindBin(p.bkg_right_min);
        int bkg_right_end   = energyHist->FindBin(p.bkg_right_max);

        double bkg_left_counts = energyHist->Integral(bkg_left_start, bkg_left_end);
        double bkg_right_counts = energyHist->Integral(bkg_right_start, bkg_right_end);

        int bkg_left_bins = bkg_left_end - bkg_left_start + 1;
        int bkg_right_bins = bkg_right_end - bkg_right_start + 1;

        double bkg_left_density = bkg_left_counts / bkg_left_bins;
        double bkg_right_density = bkg_right_counts / bkg_right_bins;

        // Interpolated background under the signal region
        double total_interp_bkg = 0.0;
        for (int j = 0; j < signal_bins; ++j) {
            double interp_bkg_bin = bkg_left_density + (j / (double)(signal_bins - 1)) * (bkg_right_density - bkg_left_density);
            total_interp_bkg += interp_bkg_bin;
        }

        double peak_counts = energyHist->Integral(bin_start, bin_end);
        double net_peak_counts = peak_counts - total_interp_bkg;

        // Create a new canvas for each peak
        TCanvas* c = new TCanvas(("c" + std::to_string(canvas_index)).c_str(), ("Peak" + std::to_string(canvas_index)).c_str(), 800, 600);
        canvases.push_back(c);
        c->cd();

        // Clone histogram for display
        TH1F* hist_clone = (TH1F*)energyHist->Clone(("hist_clone_" + std::to_string(canvas_index)).c_str());
        hist_clone->GetXaxis()->SetRangeUser(p.x_min, p.x_max);
        hist_clone->SetLineColor(kBlue);
        hist_clone->SetTitle(("Peak at " + std::to_string(p.peak_energy) + " keV; Energy [keV]; Counts").c_str());
        hist_clone->Draw("");

        // Optional: fit the peak for position and FWHM estimation
        TF1* fitFunc = new TF1(("fitFunc" + std::to_string(canvas_index)).c_str(), "gaus + pol0(3)", p.bin_min, p.bin_max);
        fitFunc->SetParameter(1, p.peak_energy);
        fitFunc->SetParameter(2, 2);
        hist_clone->Fit(fitFunc, "RQ");

        double peak_position = fitFunc->GetParameter(1);
        double sigma = fitFunc->GetParameter(2);
        double FWHM = 2.355 * sigma;

        // Write results to file
        outFile << std::fixed << std::setprecision(2);
        outFile << detector_names[i] << " \t "
                << net_peak_counts << " \t "
                << peak_counts << " \t "
                << peak_position << " \t "
                << FWHM << "\n";

        std::cout << "Detector: " << detector_names[i] << "\n";
        std::cout << "  Peak position: " << peak_position << " keV\n";
        std::cout << "  FWHM: " << FWHM << " keV\n";
        std::cout << "  Gross counts: " << peak_counts << "\n";
        std::cout << "  Interpolated bkg under peak: " << total_interp_bkg << "\n";
        std::cout << "  Net counts: " << net_peak_counts << "\n\n";

        // Draw ROI and background lines
        auto drawLine = [&](double x, int color) {
            TLine* line = new TLine(x, 0, x, hist_clone->GetMaximum() * 0.3);
            line->SetLineColor(color);
            line->Draw("same");
        };

        drawLine(p.bin_min, kRed); drawLine(p.bin_max, kRed);
        drawLine(p.bkg_left_min, kGreen); drawLine(p.bkg_left_max, kGreen);
        drawLine(p.bkg_right_min, kGreen); drawLine(p.bkg_right_max, kGreen);

        c->SetLogy();
        c->Update();
        c->SaveAs((output_dir + "/Cut_count" + detector_names[i] + "_Run" + std::to_string(run_number) + ".png").c_str());

        canvas_index++;
    }
}
