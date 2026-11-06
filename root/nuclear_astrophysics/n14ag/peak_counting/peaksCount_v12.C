#include "TFile.h"
#include "TCanvas.h"
#include "TH1F.h"
#include "TF1.h"
#include "TLine.h"
#include "TSpectrum.h"
#include <iostream>
#include <fstream>
#include <iomanip>  // For std::setprecision
#include <sys/stat.h>
#include <sys/types.h>

void peaksCount_v12() {
    int run_number = 4557;

    // Open input file
    TFile* file = TFile::Open(("/felsdata/n14ag_Felsenkeller_2025/Rootfiles_high/Run" + std::to_string(run_number) + "_high_250502_0942_Tree.root").c_str(), "READ"); 
    if (!file || file->IsZombie()) {
        std::cerr << "Error: Could not open ROOT file!" << std::endl;
        return;
    }
    // Detector list with mapping to human-readable names
    std::map<std::string, std::string> detector_map = {
        {"histoRaw[00]", "EB18_A"}, {"histoRaw[01]", "EB18_B"}, {"histoRaw[02]", "EB18_C"},
        {"histoRaw[03]", "EB18_D"}, {"histoRaw[04]", "EB18_E"}, {"histoRaw[05]", "EB18_F"},
        {"histoRaw[06]", "EB18_G"}, {"histoRaw[23]", "MB1_1"}, {"histoRaw[24]", "MB1_2"},
        {"histoRaw[25]", "MB1_3"}, {"histoRaw[07]", "MB2_1"}, {"histoRaw[08]", "MB2_2"},
        {"histoRaw[09]", "MB2_3"}
    };

    // Extract only the keys (ROOT detector names) for processing
    std::vector<std::string> detector_names;
    for (const auto& pair : detector_map) {
        detector_names.push_back(pair.first);
    }
    // Define ROI ranges for peak search (min, max) in keV
    std::vector<std::pair<double, double>> peak_rois = {
        //{1.025, 1.055},   // ROI for first peak
        //{1.778, 1.818},   
        {3.795, 3.841}   
    };

    // Background and ROI parameters
    double roi_expansion = 0.020;  // How much to expand around found peak for final ROI
    double bkg_left_offset = -0.035;
    double bkg_right_offset = +0.035;
    double bkg_width = 0.020;

    // Create output directory
    std::string output_dir = "/felsdata/n14ag_Felsenkeller_2025/Anup/Run" + std::to_string(run_number);
    struct stat info;
    if (stat(output_dir.c_str(), &info) != 0) {
        mkdir(output_dir.c_str(), 0777);
    }

    // Create separate output files for each peak
    std::vector<std::ofstream> peak_files;
    for (size_t i = 0; i < peak_rois.size(); i++) {
        std::string filename = output_dir + "/Peak" + std::to_string(i+1) + "_Counts_Run" + std::to_string(run_number) + ".txt";
        peak_files.emplace_back(filename);
        
        peak_files[i] << "Run Number:\t" << run_number << "\n";
        peak_files[i] << "Detector\tPeak Energy (keV)\tGross Counts\tNet Counts\tNet Counts Error\n";
        peak_files[i] << "---------------------------------------------------------------------------------------------------------\n";
    }

    // Create master canvases for each peak ROI
    std::vector<TCanvas*> peak_canvases;
    for (size_t i = 0; i < peak_rois.size(); i++) {
        TCanvas* c = new TCanvas(Form("peak_%zu_canvas", i), Form("Peak %zu Fitting", i+1), 1200, 900);
        c->Divide(4, 4);
        peak_canvases.push_back(c);
    }

    // Process each detector
    for (const auto& detector : detector_names) {
        TH1F* hist = (TH1F*)file->Get(("histoRaw/" + detector).c_str());
        if (!hist) {
            std::cerr << "Warning: Could not find '" << detector << "' histogram!" << std::endl;
            continue;
        }

        // Get the human-readable name for the detector
        std::string human_readable_name = detector_map[detector];

        // Process each peak ROI
        for (size_t peak_idx = 0; peak_idx < peak_rois.size(); peak_idx++) {
            auto roi = peak_rois[peak_idx];
            
            // Clone histogram for this ROI to use with TSpectrum
            TH1F* hroi = (TH1F*)hist->Clone(Form("%s_roi%zu", detector.c_str(), peak_idx));
            hroi->GetXaxis()->SetRangeUser(roi.first, roi.second);
            
            // Find peaks in the ROI
            TSpectrum spec(10); // Find up to 5 peaks
            int npeaks = spec.Search(hroi, 2, "nodraw", 0.1);
            
            delete hroi; // Clean up cloned histogram

            if (npeaks == 0) {
                std::cerr << "Warning: No peaks found in ROI " << roi.first << "-" << roi.second 
                          << " keV for detector " << detector << std::endl;
                continue;
            }

            // Get the strongest peak
            double* xpeaks = spec.GetPositionX();
            double peak_energy = xpeaks[0];
            for (int p = 1; p < npeaks; p++) {
                if (hist->GetBinContent(hist->FindBin(xpeaks[p])) > hist->GetBinContent(hist->FindBin(peak_energy))) {
                    peak_energy = xpeaks[p];
                }
            }

            // Define final ROI around found peak
            double roi_min = peak_energy - roi_expansion;
            double roi_max = peak_energy + roi_expansion;
            
            // Define background regions
            double bkg_left_min = peak_energy + bkg_left_offset - bkg_width/2;
            double bkg_left_max = peak_energy + bkg_left_offset + bkg_width/2;
            double bkg_right_min = peak_energy + bkg_right_offset - bkg_width/2;
            double bkg_right_max = peak_energy + bkg_right_offset + bkg_width/2;

            // Calculate counts
            int roi_bin_min = hist->FindBin(roi_min);
            int roi_bin_max = hist->FindBin(roi_max);

            int roi_bins = roi_bin_max - roi_bin_min + 1;

            int bkg_left_bin_min = hist->FindBin(bkg_left_min);
            int bkg_left_bin_max = hist->FindBin(bkg_left_max);
            int bkg_right_bin_min = hist->FindBin(bkg_right_min);
            int bkg_right_bin_max = hist->FindBin(bkg_right_max);

            double peak_counts = hist->Integral(roi_bin_min, roi_bin_max);
            double bkg_left_counts = hist->Integral(bkg_left_bin_min, bkg_left_bin_max);
            double bkg_right_counts = hist->Integral(bkg_right_bin_min, bkg_right_bin_max);

            double bkg_left_avg = bkg_left_counts / (bkg_left_bin_max - bkg_left_bin_min + 1);
            double bkg_right_avg = bkg_right_counts / (bkg_right_bin_max - bkg_right_bin_min + 1);
            //double bkg_counts = ((bkg_left_avg + bkg_right_avg) * (roi_bin_max - roi_bin_min + 1)) / 2;
            double bkg_counts = ((bkg_left_avg + bkg_right_avg) * roi_bins ) / 2;

            //double net_counts = peak_counts - bkg_counts;
            //double net_counts_err = sqrt(peak_counts + bkg_counts);

            double net_counts = peak_counts - bkg_counts;
            double net_counts_err = sqrt((net_counts + bkg_counts *( 1 + roi_bins / ((bkg_left_bin_max - bkg_left_bin_min + 1) + (bkg_right_bin_min - bkg_right_bin_max +1)))));


            // Fit the peak
            TCanvas* current_canvas = peak_canvases[peak_idx];
            int pad_index = (std::distance(detector_names.begin(), 
                           std::find(detector_names.begin(), detector_names.end(), detector)) % 16) + 1;
            
            current_canvas->cd(pad_index);
            
            TH1F* hclone = (TH1F*)hist->Clone(Form("%s_peak%zu", detector.c_str(), peak_idx));
            hclone->SetLineColor(kBlue);
            hclone->SetTitle(Form("%s - Peak %zu; Energy [MeV]; Counts", human_readable_name.c_str(), peak_idx+1));
            hclone->GetXaxis()->SetRangeUser(roi.first, roi.second);
            hclone->Draw();

            TF1* fitFunc = new TF1(Form("fitFunc_%s_%zu", detector.c_str(), peak_idx), "gaus", roi_min, roi_max);
            fitFunc->SetParameters(hclone->GetBinContent(hclone->FindBin(peak_energy)), peak_energy, 2);
            hclone->Fit(fitFunc, "RQ");

            //double fit_peak_pos = fitFunc->GetParameter(1);
            //double sigma = fitFunc->GetParameter(2);
            //double FWHM = 2.355 * sigma;

            // Save results to the appropriate peak file with 1 decimal place formatting
            peak_files[peak_idx] << std::fixed << std::setprecision(2);
            peak_files[peak_idx] << human_readable_name  << "\t" 
                               << peak_energy << "\t"
                               << peak_counts << "\t"
                               << net_counts << "\t"
                               << net_counts_err << "\n";
                               //<< fit_peak_pos << "\t"
                               //<< FWHM << "\n";
            
            // Draw ROI and background regions
            auto drawLine = [&](double x, int color, double ymax_frac = 0.3) {
                TLine* line = new TLine(x, 0, x, hclone->GetMaximum() * ymax_frac);
                line->SetLineColor(color);
                line->Draw("same");
            };

            drawLine(roi_min, kRed); drawLine(roi_max, kRed);
            drawLine(bkg_left_min, kGreen); drawLine(bkg_left_max, kGreen);
            drawLine(bkg_right_min, kGreen); drawLine(bkg_right_max, kGreen);

            // Mark found peak position
            TMarker* mark = new TMarker(peak_energy, hclone->GetMaximum()*0.9, 23);
            mark->SetMarkerColor(kMagenta);
            mark->Draw();
        }
    }

    // Save all canvases
    for (size_t i = 0; i < peak_canvases.size(); i++) {
        peak_canvases[i]->Update();
        peak_canvases[i]->SaveAs(Form("%s/Peak%zu_Run%d.pdf", output_dir.c_str(), i+1, run_number));
        peak_canvases[i]->SaveAs(Form("%s/Peak%zu_Run%d.root", output_dir.c_str(), i+1, run_number));
        peak_files[i].close(); // Close each peak file
    }

    file->Close();

    // Keep canvases open
    std::cout << "\nProcessing complete! Results saved in:" << std::endl;
    for (size_t i = 0; i < peak_rois.size(); i++) {
        std::cout << "  " << output_dir << "/Peak" << i+1 << "_Counts_Run" << run_number << ".txt" << std::endl;
    }
    std::cout << "Press ENTER to exit..." << std::endl;
    std::cin.ignore();
}