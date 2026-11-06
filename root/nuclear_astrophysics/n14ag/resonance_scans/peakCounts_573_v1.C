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
void peakCounts_573_v1() {
    // === Inputs ===
    std::vector<int> run_numbers = {4450, 4553, 4557, 4563, 4585, 4591, 4592, 4595, 4596, 4599, 4602, 4604, 4607, 4610, 4612, 4614, 4617, 4562, 4588};
    std::vector<double> run_times = {59818., 74339., 77319.5, 41703.0, 103557, 24158.4, 47370.3, 26229.8, 54364.9, 76179.5, 75642.9, 30127.0, 74822.0, 57038.4, 18474.0, 58294.6, 1829352385.0, 28383.9, 50682.3}; // in seconds

    // std::vector<std::string> detector_names = {"EB18", "MB1", "All", "AllnDE"};
    std::vector<std::string> detector_names = {"All"};

    double roi_min = 3.200;
    double roi_max = 4.800;

    // --- Background ---
    TString bg_file = "Run4734/Addback_tree.root";
    // TString bg_file = "Run4725/Addback_tree.root";
    TFile* bg_f = TFile::Open(bg_file, "READ");
    if (!bg_f || bg_f->IsZombie()) { std::cerr << "Error opening background file\n"; return; }

    TH1F* bg_hist = (TH1F*)bg_f->Get("All");
    if (!bg_hist) { std::cerr << "Error: 'All' hist not found in bg file!\n"; return; }

    double t_bg = 69989.0; // 19:26:29  set background runtime here Run 4734
    // double t_bg = 39646.0; // 19:26:29  set background runtime here Run 4730 = 39646.0;
    // double t_bg = 74000.0; // 19:26:29  set background runtime here Run 4725 = 73249.0;

    

    int bg_bin_min = bg_hist->FindBin(roi_min);
    int bg_bin_max = bg_hist->FindBin(roi_max);
    double bg_counts = bg_hist->Integral(bg_bin_min, bg_bin_max);
    double bg_counts_err = sqrt(bg_counts);

    // === Loop over runs ===
    for (size_t idx = 0; idx < run_numbers.size(); idx++) {
        int run_number = run_numbers[idx];
        double t_run   = run_times[idx];

        TString input_file = TString::Format("Run%d/Addback_tree.root", run_number);
        TString output_dir = TString::Format("Run%d/250902", run_number);
        gSystem->mkdir(output_dir, true);

        TFile* f = TFile::Open(input_file, "READ");
        if (!f || f->IsZombie()) { std::cerr << "Error opening " << input_file << "\n"; continue; }

        // Prepare output text file
        TString outname = output_dir + "/Integral_2_8-3_8MeV.txt";
        std::ofstream outfile(outname);
        outfile << "Detector\tGross Counts\tBG Counts\tNet Counts\tNet Err\n";
        outfile << "---------------------------------------------------------------------\n";

        // One canvas per run
        TCanvas* c = new TCanvas(Form("c%d", run_number), Form("Run %d", run_number), 1200, 800);
        gPad->SetLogy();
        // c->Divide(2, 2);

        for (size_t i = 0; i < detector_names.size(); i++) {
            TH1F* h = (TH1F*)f->Get(detector_names[i].c_str());
            if (!h) { std::cerr << "Warning: missing hist " << detector_names[i] << "\n"; continue; }

            // Foreground counts
            int roi_bin_min = h->FindBin(roi_min);
            int roi_bin_max = h->FindBin(roi_max);
            double total_counts = h->Integral(roi_bin_min, roi_bin_max);
            double total_counts_err = sqrt(total_counts);

            // Normalized background subtraction
            double net_counts = total_counts - (t_run / t_bg) * bg_counts;
            double net_counts_err = sqrt(total_counts_err*total_counts_err +
                                         pow(t_run / t_bg,2) * bg_counts_err*bg_counts_err);

            // Save numbers
            outfile << std::fixed << std::setprecision(0);
            outfile << detector_names[i] << "\t"
                    << total_counts << "\t"
                    << bg_counts << "\t"
                    << net_counts << "\t"
                    << net_counts_err << "\n";

            // Plot
            // c->cd(i+1);

            TH1F* h_clone = (TH1F*)h->Clone();

            h_clone->SetLineColor(kBlue);
            h_clone->GetXaxis()->SetTitle("Energy (MeV)");
            h_clone->GetYaxis()->SetTitle("Counts");
            // set axis ranges
            h_clone->GetXaxis()->SetRangeUser(2.7, 5.0);
            // h_clone->GetYaxis()->SetRangeUser(1, h_clone->
            h_clone->Draw();

            TBox* roi_box = new TBox(roi_min, 0, roi_max, h_clone->GetMaximum()*0.9);
            roi_box->SetFillColorAlpha(kRed, 0.2);
            roi_box->Draw("same");

            TLatex latex;
            latex.SetNDC();
            latex.DrawLatex(0.2,0.85,Form("Net = %.0f #pm %.0f", net_counts, net_counts_err));
        }

        outfile.close();
        c->SaveAs(Form("%s/ROI_plot.pdf", output_dir.Data()));
        f->Close();
    }

    bg_f->Close();
    std::cout << "Processing complete!\n";
}
