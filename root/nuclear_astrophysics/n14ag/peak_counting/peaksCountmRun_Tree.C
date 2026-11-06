#include "TFile.h"
#include "TCanvas.h"
#include "TH1F.h"
#include "TF1.h"
#include "TLine.h"
#include "TSpectrum.h"
#include "TTree.h"
#include "TMarker.h"
#include <iostream>
#include <fstream>
#include <iomanip>
#include <sys/stat.h>
#include <sys/types.h>


void peaksCountmRun_Tree(int first_run, int last_run) {
 
    TCanvas tmpCanvas("tmpCanvas", "Temporary Canvas", 800, 600);
    
    // loop over run numbers 
    int step=1;
    for (int run_number = first_run; run_number <= last_run; run_number += step){
        //int rebin_factor = 2;
        TFile* file = TFile::Open(("/felsdata/n14ag_Felsenkeller_2025/Rootfiles_high/Run" + std::to_string(run_number) + "_high_Tree.root").c_str(), "READ"); 
        if (!file || file->IsZombie()) {
            std::cerr << "Error: Could not open ROOT file!" << std::endl;
            return;
        }
        
        std::map<std::string, std::string> detector_map = {
            {"MeV00", "EB18_A"}, {"MeV01", "EB18_B"}, {"MeV02", "EB18_C"},
            {"MeV03", "EB18_D"}, {"MeV04", "EB18_E"}, {"MeV05", "EB18_F"},
            {"MeV06", "EB18_G"}, 
            {"MeV23", "MB1_1"}, {"MeV24", "MB1_2"}, {"MeV25", "MB1_3"}, 
            {"MeV07", "MB2_1"}, {"MeV08", "MB2_2"}, {"MeV09", "MB2_3"}
            
        };

        std::vector<std::string> detector_names;
        for (const auto& pair : detector_map) {
            detector_names.push_back(pair.first);
        }

    //     // Define ROI ranges for peak search (min, max) in MeV
    //    std::vector<std::pair<double, double>> peak_rois = {
    //         //{1.143, 1.203}, {1.302, 1.362} // Co-60
    //         //{0.645, 0.695}                  // Cs-137
    //         //{0.880, 0.920}, {1.806, 1.866}    // Y-88
    //         {0.320, 0.375}                    // Ba-133        
    //    };

       std::vector<std::pair<double, double>> peak_rois = {
                //{1.040, 1.200},     // 1618 kev Resonance 
                //{2.010, 2.080},
                //{2.050, 2.120},
                //{2.503, 2.585}
                //{3.100, 3.170},                    
                //{4.500, 4.670}
                // {2.74, 2.84},          // 1136 keV resonance
                // {2.50, 2.580}
                // {4.330, 4.390},   //4361 keV
                // {4.150, 4.200},   // 4177 keV
                // {2.750, 2.800},   // 2775 keV
                // {1.910, 1.970}   // 1940 keV
                //{0.630, 0.665}      // 646 keV


                //{2.345, 2.400},             // 278 keV Resonance in 14N(p, gamma)15O, run 4579   
                //{5.150, 5.200},
                // {1.360, 1.400},
                // {6.140, 6.200}
                //{0.745, 0.785},
                //{6.775, 6.810}  
                // {1.360, 1.390}            // Natural background peak
                
                // {5.200, 5.280}                // 1058 keV resonance in 14N(p, gamma)15O
                // {8.220, 8.340}

                {4.350, 4.500}                  // 434 keV resonance in 14N(p, alpha,gamma)15O

       };

        // Background and ROI parameters
        double roi_expansion = 0.040;
        double roi_expansion_left = 0.045;
        double roi_expansion_right = 0.045;
        double bkg_left_offset = -0.080;
        double bkg_right_offset = +0.080;
        double bkg_width = 0.040;

        // Create output directory
        std::string output_dir = "/felsdata/n14ag_Felsenkeller_2025/Anup/Run" + std::to_string(run_number)+ "/";
        struct stat info;
        if (stat(output_dir.c_str(), &info) != 0) {
            mkdir(output_dir.c_str(), 0777);
        }

        // Create output files
        std::vector<std::ofstream> peak_files;
        for (size_t i = 0; i < peak_rois.size(); i++) {
            std::string filename = output_dir + "/Peak" + std::to_string(i+1) + "_LcLu_Run" + std::to_string(run_number) + ".txt";
            //std::string filename = output_dir + "/Peak6_LcLu_Run" + std::to_string(run_number) + ".txt";
            peak_files.emplace_back(filename);
            peak_files[i] << "Run Number:\t" << run_number << "\n";
            peak_files[i] << "Detector\tPeak Energy (MeV)\tGross Counts\tNet Counts\tNet Counts Error\tUpper limit\t Lower limit\n"; //Net Counts Error1\n";
            peak_files[i] << "----------------------------------------------------------------------------------------------\n";
        }
        std::vector<TCanvas*> peak_canvases;
        for (size_t i = 0; i < peak_rois.size(); i++) {
            TCanvas* c = new TCanvas(Form("peak_%zu_canvas", i), Form("Peak %zu Fitting", i+1), 1200, 700);
            c->Divide(4, 4);
            //c->Update();
            peak_canvases.push_back(c);
        }

        // Process each detector
        for (const auto& detector : detector_names) {
            TTree* tree = (TTree*)file->Get("Tree/felsTree");
            if (!tree) {
                std::cerr << "Warning: Could not find 'Tree/felsTree'!" << std::endl;
                continue;
            }

            if (!tree->GetBranch(detector.c_str())) {
                std::cerr << "Warning: Branch '" << detector << "' does not exist in the tree!" << std::endl;
                continue;
            }
            // tree->Print();  // Print tree structure

            tmpCanvas.cd();


            TH1F* h = new TH1F(Form("%s_%d", detector.c_str(), run_number), Form("TreeHisto_%s", detector.c_str()), 9000, 0, 9.0);
            tree->Draw(Form("%s>>%s", detector.c_str(), h->GetName()), Form("%s>0", detector.c_str()));

            TH1F* h1 = (TH1F*)h->Clone(Form("%s_rebinned", detector.c_str()));
            h1->Rebin(2);  //rebin_factor
        

            std::string human_readable_name = detector_map[detector];
            //std::cout << "Processing detector: " << detector << std::endl;

            // Process each peak ROI
            for (size_t peak_idx = 0; peak_idx < peak_rois.size(); peak_idx++) {
                auto roi = peak_rois[peak_idx];
                
                TH1F * hroi = (TH1F*)h1->Clone(Form("%s_roi%zu", detector.c_str(), peak_idx));
                hroi->GetXaxis()->SetRangeUser(roi.first, roi.second);
                
                // std::cout << "Processing ROI: " << roi.first << "-" << roi.second 
                //         << " MeV for detector " << detector << std::endl;
                // Find peaks
                TSpectrum spec(10);
                int npeaks = spec.Search(hroi, 1, "nodraw", 0.5);
                delete hroi;
                if (npeaks == 0) {
                    std::cerr << "Warning: No peaks found in ROI " << roi.first << "-" << roi.second 
                            << " MeV for detector " << detector << std::endl;
                    //delete hroi;
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
                //std::cout << "Found peak at: " << peak_energy << " MeV" << std::endl;
                //double roi_min = peak_energy - roi_expansion;
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
                
                //std:cout <<" bkg_left_width: " << (h1->FindBin(bkg_left_max) - h1->FindBin(bkg_left_min) + 1) <<std::endl;
                //std::cout <<" bkg_right_width: " << (h1->FindBin(bkg_right_max) - h1->FindBin(bkg_right_min) + 1) <<std::endl;

                
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
                    
                // if (net_counts < 0) {
                //     net_counts = 0;
                //     net_counts_err = L_u;
                // }

                //double net_counts_err1 = sqrt(peak_counts + std::pow((1.0*roi_bins / 2), 2) * (bkg_left_counts / std::pow((h1->FindBin(bkg_left_max) - h1->FindBin(bkg_left_min) + 1), 2) + bkg_right_counts / std::pow((h1->FindBin(bkg_right_max) - h1->FindBin(bkg_right_min) + 1), 2)));
                
                //double net_counts_errr1 = sqrt(peak_counts + ((roi_bins/2)^2) *(bkg_left_counts/(h1->FindBin(bkg_left_max) - h1->FindBin(bkg_left_min) +1)^2 + bkg_right_counts/(h1->FindBin(bkg_right_max) - h1->FindBin(bkg_right_min) + 1)^2));

                // std::cout << "Peak Counts: " << peak_counts << ", Net Counts: " << net_counts 
                //         << ", Net Counts Error: " << net_counts_err << std::endl;
                // Plotting
                TCanvas* current_canvas = peak_canvases[peak_idx];
                int pad_index = std::distance(detector_names.begin(), 
                            std::find(detector_names.begin(), detector_names.end(), detector)) + 1;
                current_canvas->cd(pad_index);
                gPad->SetLogy(true);

                TH1F* hclone = (TH1F*)h1->Clone(Form("%s", human_readable_name.c_str()));
                hclone->SetLineColor(kBlue);
                hclone->SetTitle(Form("%s - Peak %zu; Energy [MeV]; Counts", human_readable_name.c_str(), peak_idx+1));
                hclone->Draw();
                hclone->GetXaxis()->SetRangeUser(peak_energy - 0.120, peak_energy + 0.1200);
                hclone->GetXaxis()->SetTitleSize(0.05);
                hclone->GetYaxis()->SetTitleSize(0.05);
                hclone->GetXaxis()->SetLabelSize(0.05);
                hclone->GetYaxis()->SetLabelSize(0.05);
                //current_canvas->Update();

                // Save results
                peak_files[peak_idx] << std::fixed;
                peak_files[peak_idx] << std::setprecision(3) << human_readable_name << "\t" 
                                << peak_energy << "\t";
                peak_files[peak_idx] << std::setprecision(0) 
                                << peak_counts << "\t"
                                << net_counts << "\t"
                                << net_counts_err << "\t"
                                << L_u << "\t"
                                << L_c << "\n";
                                //<< net_counts_err1 << "\n";

                // Draw regions
                auto drawLine = [&](double x, int color, double ymax_frac = 0.7) {
                    TLine* line = new TLine(x, 0, x, hclone->GetMaximum() * ymax_frac);
                    line->SetLineColor(color);
                    line->Draw("same");
                };
                drawLine(roi_min, kRed); drawLine(roi_max, kRed);
                drawLine(bkg_left_min, kGreen); drawLine(bkg_left_max, kGreen);
                drawLine(bkg_right_min, kGreen); drawLine(bkg_right_max, kGreen);

                TMarker* mark = new TMarker(peak_energy, hclone->GetMaximum()*0.9, 23);
                mark->SetMarkerColor(kMagenta);
                mark->Draw();
                //current_canvas->Update();
                //delete hclone;
                //delete hroi;
            }
            delete h1;
            //delete h;
        }
        for (size_t i = 0; i < peak_canvases.size(); i++) {
            //peak_canvases[i]->Update();
            peak_canvases[i]->SaveAs(Form("%s/Peak%zu_LcLu_Run%d.pdf", output_dir.c_str(), i+1, run_number));
            //peak_canvases[i]->SaveAs(Form("%s/Peak6_LcLu_Run%d.pdf", output_dir.c_str(), run_number));
            //TFile* canvas_file = new TFile(Form("%s/Peak%zu_Run%d.root", output_dir.c_str(), i+1, run_number), "RECREATE");
            peak_files[i].close();
        }
        file->Close();
        std::cout << "\nProcessing complete! Results saved in " << output_dir << std::endl;
        std::cout << "Press ENTER to exit..." << std::endl;
        std::cin.ignore();

    }
}
    

//         // Save outputs
//         for (size_t i = 0; i < peak_canvases.size(); i++) {
//             peak_canvases[i]->Update();
//             peak_canvases[i]->SaveAs(Form("%s/Peak%zu_Run%d.pdf", output_dir.c_str(), i+1, run_number));
//             peak_canvases[i]->SaveAs(Form("%s/Peak%zu_Run%d.root", output_dir.c_str(), i+1, run_number));
//             peak_files[i].close();
//         }

//         file->Close();
//         std::cout << "\nProcessing complete! Results saved in " << output_dir << std::endl;
//         std::cout << "Press ENTER to exit..." << std::endl;
//         std::cin.ignore();
    
//     }
//     cout << " All runs processed!" << endl;
//     std::cout << "Press ENTER to exit..." << std::endl;
//     std::cin.ignore();
// }