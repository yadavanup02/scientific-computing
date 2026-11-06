#include "TFile.h"
#include "TCanvas.h"
#include "TH1F.h"
#include "TLegend.h"
#include <vector>
#include <iostream>

void plot_multiple_spectra() {
    std::vector<std::string> fileNames = {
        "Output_4230/Calib_Run4230_Ron100.root",
        "Output_4233/Calib_Run4233_Ron100.root",
        "Output_4227/Calib_Run4227_Ron100.root"
    };

    // Define colors for different spectra
    std::vector<int> colors = {kRed, kBlue, kGreen + 2, kMagenta, kOrange + 7};
    TFile* file = TFile::Open(fileNames[0].c_str(), "READ");
    if (!file || file->IsZombie()) {
        std::cerr << "Error: Could not open file " << fileNames[0] << std::endl;
        return;
    }

    // Get histogram name (modify this if your histogram has a different name)
    TH1F* histRef = (TH1F*)file->Get("energyHist");
    if (!histRef) {
        std::cerr << "Error: Could not find histogram in " << fileNames[0] << std::endl;
        file->Close();
        return;
    }

    TCanvas* c = new TCanvas("c", "Multiple Spectra", 800, 600);
    c->cd();

    // Create legend
    TLegend* legend = new TLegend(0.7, 0.7, 0.9, 0.9);
    legend->SetBorderSize(0);
    legend->SetTextSize(0.03);

    bool firstHist = true;
    std::vector<TH1F*> histograms; // Store histograms

    // Loop over files and plot histograms
    for (size_t i = 0; i < fileNames.size(); ++i) {
        TFile* file = TFile::Open(fileNames[i].c_str(), "READ");
        if (!file || file->IsZombie()) {
            std::cerr << "Error: Could not open file " << fileNames[i] << std::endl;
            continue;
        }

        // Get histogram
        TH1F* hist = (TH1F*)file->Get("energyHist");
        if (!hist) {
            std::cerr << "Error: Could not find histogram in " << fileNames[i] << std::endl;
            file->Close();
            continue;
        }

        // Clone histogram so original is not modified
        TH1F* histClone = (TH1F*)hist->Clone(("histClone" + std::to_string(i)).c_str());
        histClone->SetLineColor(colors[i % colors.size()]); // Assign color
        //histClone->SetLineWidth(2);

        // Normalize histograms to the same area (optional)
        histClone->Scale(1.0 / histClone->Integral());
        histClone->GetXaxis()->SetRangeUser(2490, 2590);
        histClone->SetTitle("Energy Spectrum; Energy [keV]; Counts");
        histClone->SetStats(0); 
        //c->SetLogy();
        // Smooth the histogram
        histClone->Smooth(2);

        histograms.push_back(histClone);
        histClone->Draw("HIST SAME");

        // Draw histogram
        // if (firstHist) {
        //     histClone->Draw("HIST"); // First histogram sets axis range
        //     firstHist = false;
        // } else {
        //     histClone->Draw("HIST SAME");
        // }
        // Find the maximum bin content for setting the Y-axis range
        double maxBinContent = histClone->GetMaximum();
        if (maxBinContent > histRef->GetMaximum()) {
            histRef->SetMaximum(maxBinContent * 1.1); // Set a bit higher than the max bin content
        }
        TString legendEntry = fileNames[i].substr(0, fileNames[i].find("/"));
        legend->AddEntry(histClone, legendEntry, "l");

        //file->Close();
    }
    legend->Draw();
    c->SaveAs("MultipleSpectra.png");

    std::cout << "Plot saved as MultipleSpectra.png" << std::endl;
}
