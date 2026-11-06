#include "TFile.h"
#include "TCanvas.h"
#include "TH1F.h"
#include "TF1.h"
#include "TLine.h"
#include "TSpectrum.h"
#include <iostream>
#include <fstream>
#include <iomanip>
#include <sys/stat.h>
#include <sys/types.h>
void peaksCounts_573() {
    int rebin_factor = 1;
    // Open input file
    TFile* file = TFile::Open("res573/602keV_250826/combined_spectra.root", "READ"); 
    if (!file || file->IsZombie()) {
        std::cerr << "Error: Could not open ROOT file!" << std::endl;
        return;
    }
    std::vector<std::string> detector_names = {
        "EB18_A", "EB18_B", "EB18_C", "EB18_D", "EB18_E", "EB18_F", "EB18_G",
        "MB1_1", "MB1_2", "MB1_3",
        "MB2_1", "MB2_2", "MB2_3"
    };
    std::vector<std::pair<double, double>> peak_rois = {
        // {1.715, 1.735},
        // {1.790, 1.810}
        // /{3.770, 3.800},
        {3.800, 3.840}
    };

    // Background and ROI parameters
    double roi_expansion = 0.020;
    double roi_expansion_left = 0.013;
    double roi_expansion_right = 0.013;
    double bkg_left_offset = -0.020;
    double bkg_right_offset = +0.020;
    double bkg_width = 0.013;

    // Create output directory
    std::string output_dir = "res573/602keV_250826";
    struct stat info;
    if (stat(output_dir.c_str(), &info) != 0) {
        mkdir(output_dir.c_str(), 0777);
    }

    // Create output files
    std::vector<std::ofstream> peak_files;
    for (size_t i = 0; i < peak_rois.size(); i++) {
        // std::string filename = output_dir + "/Peak" + std::to_string(i+1) + "_Run4414.txt";
        std::string filename = output_dir + "/Peak4_602keV.txt";
        peak_files.emplace_back(filename);
        peak_files[i] << "Detector\tPeak Energy (MeV)\tGross Counts\tNet Counts\tNet Counts Error\tUpper limit\t Lower limit\n"; //Net Counts Error1\n";
        peak_files[i] << "---------------------------------------------------------------------\n";
    }

    // Create canvases
    std::vector<TCanvas*> peak_canvases;
    for (size_t i = 0; i < peak_rois.size(); i++) {
        TCanvas* c = new TCanvas(Form("peak_%zu_canvas", i), Form("Peak %zu Fitting", i+1), 1600, 1000);
        c->Divide(4, 4);
        peak_canvases.push_back(c);
    }

    // Process each detector
    for (const auto& detector : detector_names) {
        TH1F* hist = (TH1F*)file->Get(("Crystals/" + detector).c_str());
        if (!hist) {
            std::cerr << "Warning: Could not find '" << detector << "' histogram!" << std::endl;
            continue;
        }

        // Rebin the histogram
        TH1F* h1 = (TH1F*)hist->Clone(Form("%s_rebinned", detector.c_str()));
        std::string human_readable_name = detector;

        // Process each peak ROI
        for (size_t peak_idx = 0; peak_idx < peak_rois.size(); peak_idx++) {
            auto roi = peak_rois[peak_idx];
            
            // Clone rebinned histogram for peak finding
            TH1F* hroi = (TH1F*)h1->Clone(Form("%s_roi%zu", detector.c_str(), peak_idx));
            hroi->GetXaxis()->SetRangeUser(roi.first, roi.second);
            
            // Find peaks
            TSpectrum spec(10);
            int npeaks = spec.Search(hroi, 1, "nodraw", 0.5);
            delete hroi;

            if (npeaks == 0) {
                std::cerr << "Warning: No peaks found in ROI " << roi.first << "-" << roi.second 
                        << " KeV for detector " << detector << std::endl;
                continue;
            }

            // Select highest peak
            double* xpeaks = spec.GetPositionX();
            double peak_energy = xpeaks[0];
            for (int p = 1; p < npeaks; p++) {
                if (h1->GetBinContent(h1->FindBin(xpeaks[p])) > 
                    h1->GetBinContent(h1->FindBin(peak_energy))) {
                    peak_energy = xpeaks[p];
                }
            }
            double roi_min = peak_energy - roi_expansion_left;
            //double roi_max = peak_energy + roi_expansion;
            double roi_max = peak_energy + roi_expansion_right;
            double bkg_left_min = peak_energy + bkg_left_offset - bkg_width/2;
            double bkg_left_max = peak_energy + bkg_left_offset + bkg_width/2;
            double bkg_right_min = peak_energy + bkg_right_offset - bkg_width/2;
            double bkg_right_max = peak_energy + bkg_right_offset + bkg_width/2;

            // Calculate counts using rebinned histogram
            int roi_bin_min = h1->FindBin(roi_min);
            int roi_bin_max = h1->FindBin(roi_max);
            int roi_bins = roi_bin_max - roi_bin_min + 1;
            
            double peak_counts = h1->Integral(roi_bin_min, roi_bin_max);
            double bkg_left_counts = h1->Integral(h1->FindBin(bkg_left_min), h1->FindBin(bkg_left_max));
            double bkg_right_counts = h1->Integral(h1->FindBin(bkg_right_min), h1->FindBin(bkg_right_max));
            
            double bkg_left_avg = bkg_left_counts / (h1->FindBin(bkg_left_max) - h1->FindBin(bkg_left_min) + 1);
            double bkg_right_avg = bkg_right_counts / (h1->FindBin(bkg_right_max) - h1->FindBin(bkg_right_min) + 1);
            double bkg_counts = ((bkg_left_avg + bkg_right_avg) * roi_bins) / 2.0;
            double net_counts = peak_counts - bkg_counts;
            double net_counts_err = sqrt((net_counts + bkg_counts * (1 + 1.0*roi_bins / 
                ((h1->FindBin(bkg_left_max) - h1->FindBin(bkg_left_min) + 1 + 
                h1->FindBin(bkg_right_max) - h1->FindBin(bkg_right_min) + 1)))));

            //double net_counts_err1 = sqrt(peak_counts + std::pow((1.0*roi_bins / 2), 2) * (bkg_left_counts / std::pow((h1->FindBin(bkg_left_max) - h1->FindBin(bkg_left_min) + 1), 2) + bkg_right_counts / std::pow((h1->FindBin(bkg_right_max) - h1->FindBin(bkg_right_min) + 1), 2)));
                
            // Lower limit and upper limit calculations with 68% confidence level for the 1-tailedconfidence,  k_alpha =0.4677
                double L_c = 0.4677 * sqrt(bkg_counts * (1 + 1.0*roi_bins / 
                ((h1->FindBin(bkg_left_max) - h1->FindBin(bkg_left_min) + 1 + 
                h1->FindBin(bkg_right_max) - h1->FindBin(bkg_right_min) + 1))));

            double L_u = net_counts + 1.0 * sqrt(net_counts + bkg_counts * (1 + 1.0*roi_bins / 
                ((h1->FindBin(bkg_left_max) - h1->FindBin(bkg_left_min) + 1 + 
                h1->FindBin(bkg_right_max) - h1->FindBin(bkg_right_min) + 1))));

            // Plotting
            TCanvas* current_canvas = peak_canvases[peak_idx];
            // Change the number 16 below to match the total number of pads in your canvas (e.g., 15 for 5x3)
            int pad_index = (std::distance(detector_names.begin(), 
                        std::find(detector_names.begin(), detector_names.end(), detector)) % 16) + 1;
            current_canvas->cd(pad_index);
            // gPad->SetLogy(true);
            TH1F* hclone = (TH1F*)h1->Clone(Form("%s_peak%zu", detector.c_str(), peak_idx));
            hclone->SetLineColor(kBlue);
            hclone->SetLineWidth(1);
            hclone->SetTitle(Form("%s - Peak %zu; Energy [keV]; Counts", human_readable_name.c_str(), peak_idx+1));
            hclone->GetXaxis()->SetRangeUser(peak_energy - 0.035, peak_energy + 0.035);
            hclone->GetXaxis()->SetTitleSize(0.05);
            hclone->GetYaxis()->SetTitleSize(0.05);
            hclone->GetXaxis()->SetLabelSize(0.05);
            hclone->GetYaxis()->SetLabelSize(0.05);
            gPad->SetBottomMargin(0.1);
            gPad->SetRightMargin(0.01);
            // hclone->GetYaxis()->SetRangeUser(hclone->GetMaximum() * 0.3, hclone->GetMaximum() * 1.2);
            hclone->Draw();

            // Save results
            peak_files[peak_idx] << std::fixed;
            peak_files[peak_idx] << std::setprecision(1) << human_readable_name << "\t" 
                            << peak_energy << "\t";
            peak_files[peak_idx] << std::setprecision(0) 
                            << peak_counts << "\t"
                            << net_counts << "\t"
                            << net_counts_err << "\t"
                            << L_u << "\t"
                            << L_c << "\n";
            // Create a transparent box for the ROI
            auto fillROI = [&](double xmin, double xmax, int color, double ymax_frac = 0.999) {
            // Get the current Y-axis minimum from the pad (not fixed at 0)
            double pad_ymin = gPad->GetUymin()*0.50;
            double pad_ymax = hclone->GetMaximum() * ymax_frac;

            // Create a box that respects the pad's Y-axis limits
            TBox *box = new TBox(
                xmin,      // xmin (left edge)
                pad_ymin,  // ymin (clamped to pad's minimum Y)
                xmax,      // xmax (right edge)
                pad_ymax   // ymax (top of box)
            );
            box->SetFillColorAlpha(color, 0.5);  // 30% opacity
            box->SetLineColor(color);            // Border color
            box->SetFillStyle(1001);             // Solid fill
            box->Draw("same");
        };

        // Fill Peak ROI (red)
        fillROI(roi_min, roi_max, 46);

        // Fill Background ROIs (green)
        fillROI(bkg_left_min, bkg_left_max, 14);
        fillROI(bkg_right_min, bkg_right_max, 14);

        // Optional: Keep the marker for the peak center
        TMarker* mark = new TMarker(peak_energy, hclone->GetMaximum()*0.9, 23);
        mark->SetMarkerColor(kMagenta);
        mark->Draw();
        };
        delete h1;
    }

    // Save outputs
    for (size_t i = 0; i < peak_canvases.size(); i++) {
        peak_canvases[i]->Update();
        // peak_canvases[i]->SaveAs(Form("%s/Peak%zu_Run4414.pdf", output_dir.c_str(), i+1));
        peak_canvases[i]->SaveAs(Form("%s/Peak4_602keV.pdf", output_dir.c_str()));

        // peak_canvases[i]->SaveAs(Form("%s/Peak%zu_Run%d.root", output_dir.c_str(), i+1, run_number));
        peak_files[i].close();
    }

    file->Close();
    std::cout << "\nProcessing complete! Results saved in " << output_dir << std::endl;
    std::cout << "Press ENTER to exit..." << std::endl;
    std::cin.ignore();
    
}