// // Addback Tree analysis
// #include "TFile.h"
// #include "TTree.h"
// #include "TH1F.h"
// #include "TCanvas.h"
// #include "TString.h"

// void addback_analysis_v1() {
//     // Input file
//     //TString input_file = "Run4557_high_250502_0942_Tree.root";
//     int run_number = 4450;
//     TString input_file = TString::Format("/felsdata/n14ag_Felsenkeller_2025/Rootfiles_high/Run%d_high_fromBuffer_250430_1824_Tree.root", run_number);
//     TString output_dir = TString::Format("/felsdata/n14ag_Felsenkeller_2025/Anup/Run%d/", run_number);
//     TString output_rootfile = output_dir + "Addback_hist_spectra.root";
    
    
//     // Open input file
//     TFile* readFile = new TFile(input_file, "READ");
//     if (!readFile || readFile->IsZombie()) {
//         std::cerr << "Error: Could not open ROOT file!" << std::endl;
//         return;
//     }
//     TTree* felsTree = (TTree*)readFile->Get("Tree/felsTree");
//     if (!felsTree) {
//         std::cerr << "Error: Could not find tree!" << std::endl;
//         readFile->Close();
//         return;
//     }
    
//     gSystem->mkdir(output_dir, true);
//     TFile* outFile = new TFile(output_rootfile, "RECREATE");


//     TDirectory* addbackDir = outFile->mkdir("Addback");
//     addbackDir->cd();
//     // Create addback histogram
//     TH1F* EB18Addback = new TH1F("EB18Addback", "Addback EB18;Energy (MeV);Counts", 3000, 0, 6.);
//     TH1F* MB1Addback = new TH1F("MB1Addback", "Addback MB1;Energy (MeV);Counts", 3000, 0, 6.);
//     TH1F* MB2Addback = new TH1F("MB2Addback", "Addback M22;Energy (MeV);Counts", 3000, 0, 6.);
    
//     TCanvas* c1 = new TCanvas("c1", "Addback analysis c1", 1200, 1200);
//     c1->Divide(2,2);
//     c1->cd(1);
//     felsTree->Draw("(MeV00+MeV01+MeV02+MeV03+MeV04+MeV05+MeV06)>>EB18Addback", 
//                   "(MeV00+MeV01+MeV02+MeV03+MeV04+MeV05+MeV06) > 0");
//     felsTree->SetMarkerColor(kBlue);

//     c1->cd(2);
//     felsTree->Draw("(MeV23+MeV24+MeV25)>>MB1Addback", "(MeV23+MeV24+MeV25)> 0");

//     c1->cd(3);
//     felsTree->Draw("(MeV07+MeV08+MeV09)>>MB2Addback", "(MeV07+MeV08+MeV09)> 0");

//     // Save the canvas
//     c1->SaveAs(output_dir + "Addback_Tree.pdf");

//     EB18Addback->Write();
//     MB1Addback->Write();
//     MB2Addback->Write();   
//     c1->Write();
//     outFile->Close();

//     // Clean up
//     //readFile->Close();
//     std::cout << "\nProcessing complete! Results saved in " << output_dir << std::endl;
//     std::cout << "Press ENTER to exit..." << std::endl;
//     std::cin.ignore();
// }


// Addback histogram analysis
/*
#include "TFile.h"
#include "TH1F.h"
#include "TCanvas.h"
#include "TString.h"
#include <vector>

void addback_histogram_v1() {
    // Input file
    int run_number = 4450;
    TString input_file = TString::Format("/felsdata/n14ag_Felsenkeller_2025/Rootfiles_high/Run%d_high_fromBuffer_250430_1824_Tree.root", run_number);
    TString output_dir = TString::Format("/felsdata/n14ag_Felsenkeller_2025/Anup/Run%d/", run_number);
    TString output_rootfile = output_dir + "Addback_hist_spectra.root";
    
    // Open input file
    TFile* file = TFile::Open(input_file, "READ");
    if (!file || file->IsZombie()) {
        std::cerr << "Error: Could not open ROOT file!" << std::endl;
        return;
    }

    // Get the directory
    TDirectory* dir = (TDirectory*)file->Get("histoRaw");
    if (!dir) {
        std::cerr << "Error: Could not find histoRaw directory!" << std::endl;
        file->Close();
        return;
    }
    gSystem->mkdir(output_dir, true);
    TFile* outFile = new TFile(output_rootfile, "RECREATE");
    TDirectory* addbackDir = outFile->mkdir("Addback_spectra");
    addbackDir->cd();

    // Define detector groups
    std::vector<TString> EB18A = {"histoRaw[00]", "histoRaw[01]", "histoRaw[02]", 
                                 "histoRaw[03]", "histoRaw[04]", "histoRaw[05]", "histoRaw[06]"};
    std::vector<TString> MB1 = {"histoRaw[23]", "histoRaw[24]", "histoRaw[25]"};
    std::vector<TString> MB2 = {"histoRaw[07]", "histoRaw[08]", "histoRaw[09]"};


    // Define the output binning (3000 bins from 0-6 MeV)
    const int nbins = 1;
    const double xmin = 0.0;
    const double xmax = 6.0;

    TH1F* h_sum1 = new TH1F("sum_EB18", "EB18 Spectra;Energy [MeV];Counts", nbins, xmin, xmax);
    TH1F* h_sum2 = new TH1F("sum_MB1", "MB1 Spectra;Energy [MeV];Counts", nbins, xmin, xmax);
    TH1F* h_sum3 = new TH1F("sum_MB2", "MB2 Spectra;Energy [MeV];Counts", nbins, xmin, xmax);

    // Function to rebin and add a histogram
    // auto add_histogram = [](TH1F* target, TH1F* source) {
    //     // Rebin by copying content bin-by-bin
    //     for (int i = 1; i <= source->GetNbinsX(); i++) {
    //         double x = source->GetBinCenter(i);
    //         if (x >= target->GetXaxis()->GetXmin() && x <= target->GetXaxis()->GetXmax()) {
    //             target->Fill(x, source->GetBinContent(i));
    //         }
    //     }
    // };
    // Sum EB18
    for (const auto& name : EB18A) {
    TH1F* h = (TH1F*)dir->Get(name);
    if (!h) {
        std::cerr << "Warning: Could not find histogram " << name << std::endl;
        continue;
    }
        h_sum1->Add(h);
    }

    // <MB1>
    for (const auto& name : MB1) {
        TH1F* h = (TH1F*)dir->Get(name);
        if (!h) {
            std::cerr << "Warning: Could not find histogram " << name << std::endl;
            continue;
        }
        h_sum2->Add(h);
    }

    // MB2
    for (const auto& name : MB2) {
        TH1F* h = (TH1F*)dir->Get(name);
        if (!h) {
            std::cerr << "Warning: Could not find histogram " << name << std::endl;
            continue;
        }
        if (!h_sum3) {
            h_sum3 = (TH1F*)h->Clone("sum_MB2");
            //h_sum3->SetTitle("Summed Detectors 07-09;Energy;Counts");
        } else {
            h_sum3->Add(h);
        }
    }
    // // Sum group 2 (23-25)
    // for (const auto& name : MB1) {
    //     TH1F* h = (TH1F*)dir->Get(name);
    //     if (!h) {
    //         std::cerr << "Warning: Could not find histogram " << name << std::endl;
    //         continue;
    //     }
    //     if (!h_sum2) {
    //         h_sum2 = (TH1F*)h->Clone("sum_23_25");
    //         h_sum2->SetTitle("Summed Detectors 23-25;Energy;Counts");
    //     } else {
    //         h_sum2->Add(h);
    //     }
    // }

    // Sum group 3 (07-09)
    // for (const auto& name : MB2) {
    //     TH1F* h = (TH1F*)dir->Get(name);
    //     if (!h) {
    //         std::cerr << "Warning: Could not find histogram " << name << std::endl;
    //         continue;
    //     }
    //     if (!h_sum3) {
    //         h_sum3 = (TH1F*)h->Clone("sum_07_09");
    //         h_sum3->SetTitle("Summed Detectors 07-09;Energy;Counts");
    //     } else {
    //         h_sum3->Add(h);
    //     }
    // }

    // Create canvas and draw
    TCanvas* c = new TCanvas("c", "Summed Spectra", 1200, 900);
    c->Divide(2,2);

    if (h_sum1) {
        c->cd(1);
        h_sum1->Draw();
    }

    if (h_sum2) {
        c->cd(2);
        h_sum2->Draw();
    }

    if (h_sum3) {
        c->cd(3);
        h_sum3->Draw();
    }

    c->SaveAs(output_dir + "Summed_histograms.pdf");

    // Save results
    // TFile* outFile = new TFile("Summed_histograms.root", "RECREATE");
    if (h_sum1) h_sum1->Write();
    if (h_sum2) h_sum2->Write();
    if (h_sum3) h_sum3->Write();
    c->Write();
    outFile->Close();

    // Clean up
    file->Close();
    
    std::cout << "Histogram summing complete. Results saved to summed_histograms.root" << std::endl;
}
*/

#include "TFile.h"
#include "TH1F.h"
#include "TCanvas.h"
#include "TLegend.h"

void addback_histogram_v1() {
    // Input file
    int run_number = 4451;
    TString input_file = TString::Format("/felsdata/n14ag_Felsenkeller_2025/Rootfiles_high/Run%d_high_fromBuffer_250430_1800_Tree.root", run_number);
    TString output_dir = TString::Format("/felsdata/n14ag_Felsenkeller_2025/Anup/Run%d/", run_number);
    TString output_rootfile = output_dir + "Addback_hist_spectra.root";
    
    // Open input file
    TFile* file = TFile::Open(input_file, "READ");
    if (!file || file->IsZombie()) {
        std::cerr << "Error: Could not open ROOT file!" << std::endl;
        return;
    }

    // Get the directory
    TDirectory* dir = (TDirectory*)file->Get("histoRaw");
    if (!dir) {
        std::cerr << "Error: Could not find histoRaw directory!" << std::endl;
        file->Close();
        return;
    }
    gSystem->mkdir(output_dir, true);
    TFile* outFile = new TFile(output_rootfile, "RECREATE");
    TDirectory* addbackDir = outFile->mkdir("Addback_spectra");
    addbackDir->cd();
    
    // Create output histograms with fixed binning (3000 bins, 0-6 MeV)
    TH1F* EB18Addback = new TH1F("EB18Addback", "EB18 Addback (00-06);Energy (MeV);Counts", 3000, 0, 6);
    TH1F* MB1Addback = new TH1F("MB1Addback", "MB1 Addback (23-25);Energy (MeV);Counts", 3000, 0, 6);
    TH1F* MB2Addback = new TH1F("MB2Addback", "MB2 Addback (07-09);Energy (MeV);Counts", 3000, 0, 6);

    // Function to add histograms with rebinning
    auto add_histogram = [](TH1F* target, TH1F* source) {
        for (int bin = 1; bin <= source->GetNbinsX(); bin++) {
            double energy = source->GetBinCenter(bin);
            double counts = source->GetBinContent(bin);
            if (energy >= 0 && energy <= 6) {
                target->Fill(energy, counts);
            }
        }
    };

    // Sum EB18 detectors (00-06)
    for (int i = 0; i <= 6; i++) {
        TString hist_name = TString::Format("histoRaw[%02d]", i);
        TH1F* h = (TH1F*)dir->Get(hist_name);
        if (h) add_histogram(EB18Addback, h);
    }

    // Sum MB1 detectors (23-25)
    for (int i = 23; i <= 25; i++) {
        TString hist_name = TString::Format("histoRaw[%02d]", i);
        TH1F* h = (TH1F*)dir->Get(hist_name);
        if (h) add_histogram(MB1Addback, h);
    }

    // Sum MB2 detectors (07-09)
    for (int i = 7; i <= 9; i++) {
        TString hist_name = TString::Format("histoRaw[%02d]", i);
        TH1F* h = (TH1F*)dir->Get(hist_name);
        if (h) add_histogram(MB2Addback, h);
    }

    // Create canvas
    TCanvas* c1 = new TCanvas("c1", "Addback Spectra", 1200, 900);
    c1->Divide(2,2);

    // Draw EB18 Addback
    c1->cd(1);
    EB18Addback->SetLineColor(kBlue);
    EB18Addback->Draw();

    // Draw MB1 Addback
    c1->cd(2);
    MB1Addback->SetLineColor(kBlue);
    MB1Addback->Draw();

    // Draw MB2 Addback
    c1->cd(3);
    MB2Addback->SetLineColor(kBlue);
    MB2Addback->Draw();

    // Draw all together
    c1->cd(4);
    EB18Addback->Draw();
    MB1Addback->Draw("SAME");
    MB2Addback->Draw("SAME");
    
    // Add legend
    TLegend* leg = new TLegend(0.7, 0.7, 0.9, 0.9);
    leg->AddEntry(EB18Addback, "EB18 (00-06)", "l");
    leg->AddEntry(MB1Addback, "MB1 (23-25)", "l");
    leg->AddEntry(MB2Addback, "MB2 (07-09)", "l");
    leg->Draw();

    // Save results
    //TFile* outFile = new TFile("addback_histograms.root", "RECREATE");
    EB18Addback->Write();
    MB1Addback->Write();
    MB2Addback->Write();
    c1->Write();
    outFile->Close();

    // Save canvas as image
    c1->SaveAs(output_dir + "addback_spectra1.pdf");

    // Clean up
    file->Close();
    
    std::cout << "Histogram addback complete. Results saved to addback_histograms.root" << std::endl;
}