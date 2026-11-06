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

void peakCountsmRun_addback_v4() {
    //std::vector<int> run_numbers = {4618, 4620}; 
    std::vector<int> run_numbers = {4587, 4588}; 
    //std::vector<int> run_numbers = {4702}; 

    TCanvas tmpCanvas("tmpCanvas", "Temporary Canvas", 800, 600);

    // TString bg_input = "/felsdata/n14ag_Felsenkeller_2025/Rootfiles_high/Run4725_high_Tree.root";
    // TFile* bgFile = TFile::Open(bg_input, "READ");
    // if (!bgFile || bgFile->IsZombie()) {
    //     std::cerr << "Error: Could not open background ROOT file" << std::endl;
    //     return;
    // }
    // TTree* bgTree = (TTree*)bgFile->Get("Tree/felsTree");

    // // Beam induced background subtractions
    // TH1F* beam_bg = new TH1F("Beam_Background", "Beam Induced Background;Energy (MeV);Counts", 6000, 0, 6.0);
    // bgTree->Draw("(MeV00+MeV01+MeV02+MeV03+MeV04+MeV05+MeV06+MeV23+MeV24+MeV25+MeV07+MeV08+MeV09)>>beam_bg", "(MeV00+MeV01+MeV02+MeV03+MeV04+MeV05+MeV06+MeV23+MeV24+MeV25+MeV07+MeV08+MeV09)", "goff");
    // beam_bg->SetLineColor(kOrange);
    // beam_bg->Draw("same");

    std::map<std::string, std::vector<std::string>> detector_clusters = {
        {"EB18", {"MeV00", "MeV01", "MeV02", "MeV03", "MeV04", "MeV05", "MeV06"}},
        {"EB18nDE", {"MeV00", "MeV01", "MeV02", "MeV05", "MeV06"}},
        {"MB1", {"MeV23", "MeV24", "MeV25"}},
        {"MB2", {"MeV07", "MeV08", "MeV09"}},
        {"All", {"MeV00", "MeV01", "MeV02", "MeV03", "MeV04", "MeV05", "MeV06",
                 "MeV23", "MeV24", "MeV25", "MeV07", "MeV08", "MeV09"}},
        {"AllnDE", {"MeV00", "MeV01", "MeV02", "MeV05", "MeV06",
                 "MeV23", "MeV24", "MeV25", "MeV07", "MeV08", "MeV09"}}
    };

    // ROI definitions
    std::vector<std::pair<double, double>> peak_rois = {{3.790, 3.850}};

    // Background parameters
    const double roi_expansion_left = 0.040;
    const double roi_expansion_right = 0.040;
    const double bkg_left_offset = -0.090;
    const double bkg_right_offset = +0.090;
    const double bkg_width = 0.050;

    // Create summed histograms for all runs
    std::map<std::string, TH1F*> summed_histograms;
    for (const auto& detector_pair : detector_clusters) {
        const std::string& cluster_name = detector_pair.first;
        summed_histograms[cluster_name] = new TH1F(
            Form("Summed_%s", cluster_name.c_str()),
            Form("Summed Addback %s;Energy (MeV);Counts", cluster_name.c_str()),
            6000, 0, 6.0
        );
    }

    // Create canvases for visualization (moved outside run loop)
    std::vector<TCanvas*> peak_canvases;
    for (size_t i = 0; i < peak_rois.size(); i++) {
        TCanvas* c = new TCanvas(Form("peak_%zu_canvas", i), 
                Form("Peak %zu Fitting", i+1), 1200, 700);
        c->Divide(3, 2); // For 6 clusters
        peak_canvases.push_back(c);
    }
    static double total_rt = 0; 
    // Process each specified run
    for (int run_number : run_numbers) {
        TString input_file = TString::Format("/felsdata/n14ag_Felsenkeller_2025/Rootfiles_high/Run%d_high_Tree.root", run_number);
        // TString output_dir = TString::Format("/felsdata/n14ag_Felsenkeller_2025/Anup/Run%d/", run_number);
        // gSystem->mkdir(output_dir, true);

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
        // Read realtime for this run and accumulate total real time
        TNamed* named = (TNamed *)readFile->Get("RunInfo/Realtime[s]");
        double rt = 0;
        if (named) {
            sscanf(named->GetTitle(), "%lf", &rt);
            total_rt += rt;
        } else {
            std::cerr << "Warning: Could not read realtime for run " << run_number << std::endl;
        }

        // Process each detector cluster
        for (const auto& detector_pair : detector_clusters) {
            const std::string& cluster_name = detector_pair.first;
            const std::vector<std::string>& detectors = detector_pair.second;

            TH1F* h = new TH1F(
                Form("%s_run%d", cluster_name.c_str(), run_number),
                Form("%s (Run %d);Energy (MeV);Counts", cluster_name.c_str(), run_number),
                6000, 0, 6.0
            );

            // Build the draw command
            TString draw_cmd;
            for (size_t i = 0; i < detectors.size(); i++) {
                if (i > 0) draw_cmd += "+";
                draw_cmd += detectors[i];
            }
            draw_cmd += Form(">>%s", h->GetName());

            // Fill the histogram
            // Build selection string: (det1>0)||(det2>0)||...
            TString selection;
            for (size_t i = 0; i < detectors.size(); i++) {
                if (i > 0) selection += "||";
                selection += Form("(%s>0)", detectors[i].c_str());
            }
            felsTree->Draw(draw_cmd, selection, "goff");

            // Add to summed histogram
            summed_histograms[cluster_name]->Add(h);
            delete h;
        }

        readFile->Close();
        std::cout << "Processed run " << run_number << std::endl;
    }
    // Beam-induced background parameters
    int background_run = 4725;
    double background_time = 73249.0; // Background run time in seconds
    double signal_time = total_rt; 

    std::cout << "Total real time: " << total_rt << " seconds" << std::endl;

    // Load background histogram
    TString bg_file = TString::Format("/felsdata/n14ag_Felsenkeller_2025/Rootfiles_high/Run%d_high_Tree.root", background_run);
    TFile* bg_input = TFile::Open(bg_file, "READ");
    if (!bg_input || bg_input->IsZombie()) {
        std::cerr << "Error: Could not open background file!" << std::endl;
        return;
    }
    // Fill background histogram
        TTree* bg_tree = (TTree*)bg_input->Get("Tree/felsTree");
        if (!bg_tree) {
            std::cerr << "Error: Could not find tree in background run " << background_run << std::endl;
            bg_input->Close();
            return;
        }
        
    // Process background subtraction
    std::map<std::string, TH1F*> background_subtracted;
    for (const auto& detector_pair : detector_clusters) {
        const std::string& cluster_name = detector_pair.first;
        const std::vector<std::string>& detectors = detector_pair.second;
        // Get background histogram for this cluster
        TH1F* bg_hist = new TH1F(Form("%s_bg_run%d", cluster_name.c_str(), background_run),
                            Form("Background %s;Energy (MeV);Counts", cluster_name.c_str()),
                            6000, 0, 6.0);

        // Build the draw command
        TString draw_cmd;
        for (size_t i = 0; i < detectors.size(); i++) {
            if (i > 0) draw_cmd += "+";
            draw_cmd += detectors[i];
        }
        draw_cmd += Form(">>%s", bg_hist->GetName());

        // Fill the histogram
        TString selection;
        for (size_t i = 0; i < detectors.size(); i++) {
            if (i > 0) selection += "||";
            selection += Form("(%s>0)", detectors[i].c_str());
        }
        
        bg_tree->Draw(draw_cmd, selection, "goff");

        
        // Create background-subtracted version
        background_subtracted[cluster_name] = (TH1F*)summed_histograms[cluster_name]->Clone(
            Form("%s_bg_subtracted", cluster_name.c_str()));
        
        // Scale background to match signal time and subtract
        bg_hist->Scale(signal_time/background_time);
        background_subtracted[cluster_name]->Add(bg_hist, -1.0);

        std::cout << cluster_name << " background subtraction: "
          << "Original=" << summed_histograms[cluster_name]->GetEntries()
          << ", Background=" << bg_hist->GetEntries()
          << ", Result=" << background_subtracted[cluster_name]->GetEntries() << std::endl;
        
        delete bg_hist;
    }
    //bg_input->Close();
     // Create output directory for combined results
    TString combined_output_dir = "/felsdata/n14ag_Felsenkeller_2025/Anup/573keVresonance/602keV_v1/";
    gSystem->mkdir(combined_output_dir, true);
    TFile* combined_outFile = new TFile(combined_output_dir + "Addback_hist.root", "RECREATE");

    // Create analysis output file
    std::ofstream peak_file(combined_output_dir + "/Peak3820keV_602keV_bgSub.txt");
    peak_file << "Real time: " << total_rt << " s\n";
    peak_file << "Cluster\tPeak Energy (MeV)\tGross Counts\tNet Counts\tNet Error\tUpper Limit\tLower Limit\n";
    peak_file << "----------------------------------------------------------------------------------------------\n";

    // Create folders/directories
    TDirectory* summed_dir = combined_outFile->mkdir("summed_hist");
    TDirectory* bgsub_dir  = combined_outFile->mkdir("bg_sub_hist");

    // Write summed_histograms to "summed_hist"
    summed_dir->cd();
    for (const auto& pair : summed_histograms) {
        pair.second->Write();
    }

    // Write background_subtracted histograms to "bg_sub_hist"
    bgsub_dir->cd();
    for (const auto& pair : background_subtracted) {
        pair.second->Write();
    }
    combined_outFile->cd();



    // Analyze summed histograms
    for (const auto& detector_pair : detector_clusters) {
        const std::string& cluster_name = detector_pair.first;
        TH1F* summed_hist = background_subtracted[cluster_name];
        // summed_hist->Write();
        std::cout << "Analyzing cluster: " << cluster_name << std::endl;

        // Rebin summed histogram
        TH1F* h1 = (TH1F*)summed_hist->Clone(Form("%s_rebinned", cluster_name.c_str()));
        h1->Rebin(2);

        std::cout << "Rebinned histogram for cluster: " << cluster_name << std::endl;

        // Process each peak ROI
        for (size_t peak_idx = 0; peak_idx < peak_rois.size(); peak_idx++) {
            auto roi = peak_rois[peak_idx];
            
            TH1F* hroi = (TH1F*)h1->Clone(Form("%s_roi_%.1f_%.1f", cluster_name.c_str(), roi.first, roi.second));
            hroi->GetXaxis()->SetRangeUser(roi.first, roi.second);
            
            TSpectrum spec(10);
            int npeaks = spec.Search(hroi, 1, "nodraw", 0.5);
            if (npeaks == 0) {
                delete hroi;
                continue;
            }
            std::cout << "Found " << npeaks << " peaks in cluster: " << cluster_name << std::endl;

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
            
            // Calculate limits
            double L_c = 0.4677 * sqrt(bkg_counts * (1 + 1.0*roi_bins / 
                ((h1->FindBin(bkg_left_max) - h1->FindBin(bkg_left_min) + 1 + 
                h1->FindBin(bkg_right_max) - h1->FindBin(bkg_right_min) + 1))));

            double L_u = net_counts + 1.0 * sqrt(net_counts + bkg_counts * (1 + 1.0*roi_bins / 
                ((h1->FindBin(bkg_left_max) - h1->FindBin(bkg_left_min) + 1 + 
                h1->FindBin(bkg_right_max) - h1->FindBin(bkg_right_min) + 1))));

            // Save results
            peak_file << std::fixed << std::setprecision(3) 
                     << cluster_name << "\t" << peak_energy << "\t"
                     << std::setprecision(0) << peak_counts << "\t"
                     << net_counts << "\t" << net_counts_err << "\t"
                     << L_u << "\t" << L_c << "\n";
            
            std::cout << "Cluster: " << cluster_name 
                      << ", Peak Energy: " << peak_energy 
                      << ", Gross Counts: " << peak_counts 
                      << ", Net Counts: " << net_counts 
                      << ", Net Error: " << net_counts_err 
                      << ", Upper Limit: " << L_u 
                      << ", Lower Limit: " << L_c << std::endl;

            // Visualization
            TCanvas* current_canvas = peak_canvases[peak_idx];
            int pad_index = 0;
            if (cluster_name == "EB18") pad_index = 1;
            else if (cluster_name == "EB18nDE") pad_index = 2;
            else if (cluster_name == "MB1") pad_index = 3;
            else if (cluster_name == "MB2") pad_index = 4;
            else if (cluster_name == "All") pad_index = 5;
            else if (cluster_name == "AllnDE") pad_index = 6;

            current_canvas->cd(pad_index);
            //gPad->SetLogy(true);

            TH1F* hdisplay = (TH1F*)h1->Clone(Form("%s_Tree", cluster_name.c_str()));
            hdisplay->SetTitle(Form("%s - %.2f MeV;Energy [MeV];Counts", cluster_name.c_str(), peak_energy));
            hdisplay->GetXaxis()->SetRangeUser(peak_energy - 0.12, peak_energy + 0.12);
            hdisplay->Draw();

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

        delete h1;
        // peak_file.close();
    }

    std::cout << "Enter to continue" << std::endl;
    std::cin.ignore();
    // Save canvases
    for (size_t i = 0; i < peak_canvases.size(); i++) {
        peak_canvases[i]->SaveAs(combined_output_dir + TString::Format("/Peak%d_Addback_Combined_bgSub.pdf", (int)i+1));
        delete peak_canvases[i];
    }
    combined_outFile->Close();
    std::cout << "Combined analysis complete! Results saved in " << combined_output_dir << std::endl;
}