#include "TFile.h"
#include "TCanvas.h"
#include "TH1F.h"
#include "TF1.h"
#include "TLine.h"
#include <iostream>
#include <fstream>
#include <sys/stat.h>
#include <sys/types.h>

void peaksCount_v9() {
    int run_number = 4418;

    TFile* file = TFile::Open(("/felsdata/n14ag_Felsenkeller_2025/Rootfiles_high/Anup/Run" + std::to_string(run_number) + "/Calibrated_Run" + std::to_string(run_number) + ".root").c_str(), "READ"); 
    if (!file || file->IsZombie()) {
        std::cerr << "Error: Could not open ROOT file!" << std::endl;
        return;
    }

    std::vector<std::string> detector_names = {
        "EB18_A_E", "EB18_B_E", "EB18_C_E", "EB18_D_E", "EB18_E_E", "EB18_F_E", "EB18_G_E",
        "MB1_1_E", "MB1_2_E", "MB1_3_E", "MB2_1_E", "MB2_2_E", "MB2_3_E"
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

    std::vector<double> peak_energies = {2775, 2523};

    double roi_left_offset = -20;
    double roi_right_offset = +20;
    double bkg_left_start_offset = -40;
    double bkg_left_end_offset = -25;
    double bkg_right_start_offset = +25;
    double bkg_right_end_offset = +40;

    std::string output_dir = "/felsdata/n14ag_Felsenkeller_2025/Anup/Output_" + std::to_string(run_number);
    struct stat info;
    if (stat(output_dir.c_str(), &info) != 0) {
        mkdir(output_dir.c_str(), 0777);
    }

    std::ofstream outFile(output_dir + "/PeakCounts_v9_Run" + std::to_string(run_number) + ".txt");
    outFile << "Run Number:\t" << run_number << "\n";
    outFile << "Detector\tPeak Energy (keV)\tGross Counts\tNet Counts\tNet Counts Error\tPeak Position (keV)\tFWHM (keV)\n";
    outFile << "---------------------------------------------------------------------------------------------------------\n";

    std::vector<TCanvas*> canvases;
    int canvas_index = 1;

    for (size_t i = 0; i < energyHists.size(); ++i) {
        TH1F* hist = energyHists[i];
        if (!hist) continue;

        for (const auto& target_peak_energy : peak_energies) {
            double roi_min = target_peak_energy + roi_left_offset;
            double roi_max = target_peak_energy + roi_right_offset;
            double bkg_left_min = target_peak_energy + bkg_left_start_offset;
            double bkg_left_max = target_peak_energy + bkg_left_end_offset;
            double bkg_right_min = target_peak_energy + bkg_right_start_offset;
            double bkg_right_max = target_peak_energy + bkg_right_end_offset;

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

            double bkg_left_avg = bkg_left_counts / (bkg_left_bin_max - bkg_left_bin_min + 1);
            double bkg_right_avg = bkg_right_counts / (bkg_right_bin_max - bkg_right_bin_min + 1);
            double bkg_counts = ((bkg_left_avg + bkg_right_avg) * roi_bins ) / 2;

            double net_counts = peak_counts - bkg_counts;
            double net_counts_err = sqrt((net_counts + bkg_counts *( 1 + roi_bins / ((bkg_left_bin_max - bkg_left_bin_min + 1) + (bkg_right_bin_min - bkg_right_bin_max +1)))));

            TCanvas* c = new TCanvas(("c" + std::to_string(canvas_index)).c_str(), ("Peak" + std::to_string(canvas_index)).c_str(), 800, 600);
            canvases.push_back(c);
            c->cd();

            TH1F* hclone = (TH1F*)hist->Clone(("hclone_" + std::to_string(canvas_index)).c_str());
            hclone->SetLineColor(kBlue);
            hclone->SetTitle(("" + detector_names[i] + " keV; Energy [keV]; Counts").c_str());
            hclone->GetXaxis()->SetRangeUser(target_peak_energy-40, target_peak_energy+40);
            hclone->Draw();

            TF1* fitFunc = new TF1("fitFunc", "gaus + pol0(3)", roi_min, roi_max);
            fitFunc->SetParameters(hist->GetBinContent(hist->FindBin(target_peak_energy)), target_peak_energy, 2, 10);
            hclone->Fit(fitFunc, "RQ");

            double fit_peak_pos = fitFunc->GetParameter(1);
            double sigma = fitFunc->GetParameter(2);
            double FWHM = 2.355 * sigma;

            outFile << detector_names[i] << "\t" << target_peak_energy << "\t" << peak_counts << "\t" << net_counts << "\t" << net_counts_err << "\t" << fit_peak_pos << "\t" << FWHM << "\n";

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
            c->SaveAs((output_dir + "/peak_v9_" + detector_names[i] + "_" + std::to_string((int)target_peak_energy) + "keV_Run" + std::to_string(run_number) + ".pdf").c_str());
            canvas_index++;
        }
    }
}
