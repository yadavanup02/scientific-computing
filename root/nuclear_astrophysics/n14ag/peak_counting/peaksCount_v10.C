#include "TFile.h"
#include "TCanvas.h"
#include "TH1F.h"
#include "TF1.h"
#include "TLine.h"
#include <iostream>
#include <fstream>
#include <sys/stat.h>
#include <sys/types.h>

void peaksCount_v10() {
    int run_number = 4437;

    TFile* file = TFile::Open(("/felsdata/n14ag_Felsenkeller_2025/Rootfiles_high/Anup/Run" + std::to_string(run_number) + "/Calibrated_Run" + std::to_string(run_number) + ".root").c_str(), "READ"); 
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

    std::vector<double> peak_energies = {2523.35, 2775.0, 1586};

    double roi_left_offset = -20;
    double roi_right_offset = +25;
    double bkg_left_start_offset = -40;
    double bkg_left_end_offset = -25;
    double bkg_right_start_offset = +30;
    double bkg_right_end_offset = +45;

    std::string output_dir = "/felsdata/n14ag_Felsenkeller_2025/Rootfiles_high/Anup/Run" + std::to_string(run_number);
    struct stat info;
    if (stat(output_dir.c_str(), &info) != 0) {
        mkdir(output_dir.c_str(), 0777);
    }

    std::ofstream outFile(output_dir + "/PeakCounts_10_Run.txt");
    outFile << "Run Number:\t" << run_number << "\n";
    outFile << "Detector\tPeak Energy (keV)\tGross Counts\tNet Counts\tNet Counts Error\tPeak Position (keV)\tFWHM (keV)\n";
    outFile << "---------------------------------------------------------------------------------------------------------\n";

    TCanvas* canvas = new TCanvas("canvas", "Peak Fitting", 1600, 1200);
    canvas->Divide(5, 5);
    int pad_index = 1;

    for (size_t i = 0; i < energyHists.size(); ++i) {
        TH1F* hist = energyHists[i];
        if (!hist) continue;

        for (const auto& target_peak_energy : peak_energies) {
            if (pad_index > 26) {
                canvas->Update();
                canvas->SaveAs((output_dir + "/peak_v9_Run" + std::to_string(run_number) + ".pdf").c_str());
                canvas->Clear();
                canvas->Divide(5, 5);
                pad_index = 1;
            }
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

            canvas->cd(pad_index);

            TH1F* hclone = (TH1F*)hist->Clone(("hclone_" + std::to_string(pad_index)).c_str());
            hclone->SetLineColor(kBlue);
            hclone->SetTitle(("" + detector_names[i] + "; Energy [keV]; Counts").c_str());
            hclone->GetXaxis()->SetRangeUser(target_peak_energy-40, target_peak_energy+40);
            hclone->Draw();

            TF1* fitFunc = new TF1("fitFunc", "gaus", roi_min, roi_max);
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
            
            pad_index++; 

            // c->SetLogy();
            // c->Update();
            // c->SaveAs((output_dir + "/peak_v9_" + detector_names[i] + "_" + std::to_string((int)target_peak_energy) + "keV_Run" + std::to_string(run_number) + ".pdf").c_str());
        }
    }

    canvas->SetLogy();
    canvas->Update();
    canvas->SaveAs((output_dir + "/AllPeaks_Run" + std::to_string(run_number) + ".pdf").c_str());
    outFile.close();
    //file->Close();
}
