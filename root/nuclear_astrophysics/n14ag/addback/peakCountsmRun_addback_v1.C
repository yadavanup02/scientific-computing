#include "TFile.h"
#include "TTree.h"
#include "TH1F.h"
#include "TCanvas.h"
#include "TString.h"
#include "TLine.h"
#include "TSpectrum.h"
#include "TMarker.h"
#include <iostream>
#include <fstream>
#include <iomanip>
#include <sys/stat.h>
#include <vector>
#include <map>

void peakCountsmRun_addback_v1(int start_run, int end_run) {
    TCanvas tmpCanvas("tmpCanvas", "Temporary Canvas", 800, 600);

    for (int run_number = start_run; run_number <= end_run; run_number++) {
        // File handling
        TString input_file = TString::Format("/felsdata/n14ag_Felsenkeller_2025/Rootfiles_high/Run%d_high_Tree.root", run_number);
        TString output_dir = TString::Format("/felsdata/n14ag_Felsenkeller_2025/Anup/Run%d/", run_number);
        gSystem->mkdir(output_dir, true);
        TString output_rootfile = output_dir + "Addback_tree.root";
        
        TFile* readFile = TFile::Open(input_file, "READ");
        if (!readFile || readFile->IsZombie()) {
            std::cerr << "Error: Could not open ROOT file for run " << run_number << std::endl;
            continue;
        }

        TTree* felsTree = (TTree*)readFile->Get("Tree/felsTree");
        if (!felsTree) {
            std::cerr << "Error: Could not find tree in run " << run_number << std::endl;
            readFile->Close();
            continue;
        }

        // Create output files
        TFile* outFile = new TFile(output_rootfile, "RECREATE");
        std::vector<std::pair<double, double>> peak_rois = {{3.790, 3.850}}; // 573 resonance PT

        // Background parameters
        const double roi_expansion_left = 0.030;
        const double roi_expansion_right = 0.030;
        const double bkg_left_offset = -0.060;
        const double bkg_right_offset = +0.060;
        const double bkg_width = 0.030;

        // Create text output files
        std::vector<std::ofstream> peak_files;
        for (size_t i = 0; i < peak_rois.size(); i++) {
            TString filename = output_dir + TString::Format("/Peak%d_Addback_Run%d.txt", (int)i+1, run_number);
            peak_files.emplace_back(filename.Data());
            peak_files.back() << "Run Number:\t" << run_number << "\n";
            peak_files.back() << "Detector\tPeak Energy (MeV)\tGross Counts\tNet Counts\tNet Error\tUpper Limit\tLower Limit\n";
            peak_files.back() << "----------------------------------------------------------------------------------------------\n";
        }

        // Create canvases
        std::vector<TCanvas*> peak_canvases;
        for (size_t i = 0; i < peak_rois.size(); i++) {
            TCanvas* c = new TCanvas(TString::Format("peak_%zu_canvas", i), TString::Format("Peak %zu Fitting", i+1), 1200, 700);
            c->Divide(2, 2); // For 4 detectors
            peak_canvases.push_back(c);
        }

        // Define detectors and their addback formulas
        std::vector<std::pair<std::string, std::string>> detectors = {
            {"EB18", "(MeV00+MeV01+MeV02+MeV03+MeV04+MeV05+MeV06)"},
            {"MB1", "(MeV23+MeV24+MeV25)"},
            {"MB2", "(MeV07+MeV08+MeV09)"},
            {"AddbackAll", "(MeV00+MeV01+MeV02+MeV03+MeV04+MeV05+MeV06+MeV23+MeV24+MeV25+MeV07+MeV08+MeV09)"}
        };
        

        // Process each detector
        for (size_t det_idx = 0; det_idx < detectors.size(); det_idx++) {
            const auto& detector_pair = detectors[det_idx];
            const std::string& det_name = detector_pair.first;
            const std::string& det_formula = detector_pair.second;

            
            // Create and fill histogram
            TH1F* h = new TH1F(det_name.c_str(), Form("%s;Energy (MeV);Counts", det_name.c_str()), 6000, 0, 6.0);
            felsTree->Draw(Form("%s>>%s", det_formula.c_str(), det_name.c_str()), 
                          Form("%s>0", det_formula.c_str()), "goff");
            
            // Rebin
            TH1F* h1 = (TH1F*)h->Clone(Form("%s_rebinned", det_name.c_str()));
            h1->Rebin(2);

            // Process each peak ROI
            for (size_t peak_idx = 0; peak_idx < peak_rois.size(); peak_idx++) {
                auto roi = peak_rois[peak_idx];
                
                // Find peak
                TH1F* hroi = (TH1F*)h1->Clone(Form("%s_roi_%.1f_%.1f", det_name.c_str(), roi.first, roi.second));
                hroi->GetXaxis()->SetRangeUser(roi.first, roi.second);
                
                TSpectrum spec(10);
                int npeaks = spec.Search(hroi, 1, "nodraw", 0.5);
                if (npeaks == 0) {
                    delete hroi;
                    continue;
                }

                // Get highest peak
                double peak_energy = spec.GetPositionX()[0];
                for (int p = 1; p < npeaks; p++) {
                    if (hroi->GetBinContent(hroi->FindBin(spec.GetPositionX()[p])) > 
                        hroi->GetBinContent(hroi->FindBin(peak_energy))) {
                        peak_energy = spec.GetPositionX()[p];
                    }
                }

                // Define regions
                double roi_min = peak_energy - roi_expansion_left;
                double roi_max = peak_energy + roi_expansion_right;
                double bkg_left_min = peak_energy + bkg_left_offset - bkg_width/2;
                double bkg_left_max = peak_energy + bkg_left_offset + bkg_width/2;
                double bkg_right_min = peak_energy + bkg_right_offset - bkg_width/2;
                double bkg_right_max = peak_energy + bkg_right_offset + bkg_width/2;

                // Calculate counts
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
                
                // Lower limit and upper limit calculations with 68% confidence level for the 1-tailedconfidence,  k_alpha =0.4677
                double L_c = 0.4677 * sqrt(bkg_counts * (1 + 1.0*roi_bins / 
                    ((h1->FindBin(bkg_left_max) - h1->FindBin(bkg_left_min) + 1 + 
                    h1->FindBin(bkg_right_max) - h1->FindBin(bkg_right_min) + 1))));

                double L_u = net_counts + 1.0 * sqrt(net_counts + bkg_counts * (1 + 1.0*roi_bins / 
                    ((h1->FindBin(bkg_left_max) - h1->FindBin(bkg_left_min) + 1 + 
                    h1->FindBin(bkg_right_max) - h1->FindBin(bkg_right_min) + 1))));
                    


                // double net_counts_err = sqrt(net_counts + bkg_counts * (1 + roi_bins/(2*bkg_width/h1->GetBinWidth(1))));
                
                // // Calculate limits
                // double L_c = 0.4677 * sqrt(bkg_counts * (1 + roi_bins/(2*bkg_width/h1->GetBinWidth(1))));
                // double L_u = net_counts + 1.0 * sqrt(net_counts + bkg_counts * (1 + roi_bins/(2*bkg_width/h1->GetBinWidth(1))));

                // Save results
                peak_files[peak_idx] << std::fixed << std::setprecision(3) 
                                   << det_name << "\t" << peak_energy << "\t"
                                   << std::setprecision(0) << peak_counts << "\t"
                                   << net_counts << "\t" << net_counts_err << "\t"
                                   << L_u << "\t" << L_c << "\n";

                // Visualization
                TCanvas* current_canvas = peak_canvases[peak_idx];
                int pad_index = det_idx + 1;
                current_canvas->cd(pad_index);
                gPad->SetLogy(true);

                TH1F* hdisplay = (TH1F*)h1->Clone(Form("%s_display", det_name.c_str()));
                hdisplay->SetTitle(Form("%s - %.1f MeV;Energy [MeV];Counts", det_name.c_str(), peak_energy));
                hdisplay->GetXaxis()->SetRangeUser(peak_energy - 0.12, peak_energy + 0.12);
                hdisplay->Draw();
                hdisplay->GetXaxis()->SetTitleSize(0.05);
                hdisplay->GetYaxis()->SetTitleSize(0.05);
                hdisplay->GetXaxis()->SetLabelSize(0.05);
                hdisplay->GetYaxis()->SetLabelSize(0.05);

                // Draw regions
                auto drawLine = [&](double x, int color) {
                    TLine* line = new TLine(x, 0, x, hdisplay->GetMaximum()*0.7);
                    line->SetLineColor(color);
                    line->Draw("same");
                };
                drawLine(roi_min, kRed);
                drawLine(roi_max, kRed);
                drawLine(bkg_left_min, kGreen);
                drawLine(bkg_left_max, kGreen);
                drawLine(bkg_right_min, kGreen);
                drawLine(bkg_right_max, kGreen);

                TMarker* mark = new TMarker(peak_energy, hdisplay->GetMaximum()*0.8, 23);
                mark->SetMarkerColor(kMagenta);
                mark->Draw();

                // delete hdisplay;
                // delete hroi;
            }
            
            // Save histogram to output file
            outFile->cd();
            h->Write();
            // delete h;
            delete h1;
        }

        // Save and clean up
        for (size_t i = 0; i < peak_canvases.size(); i++) {
            peak_canvases[i]->SaveAs(output_dir + TString::Format("/Peak%d_Addback_Run%d.pdf", (int)i+1, run_number));
            //delete peak_canvases[i];
        }

        // outFile->Close();
        // readFile->Close();
        std::cout << "Processed run " << run_number << std::endl;
        std::cin.ignore();
    }
    std::cout << "All runs processed!" << std::endl;
}