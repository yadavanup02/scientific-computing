#include "TFile.h"
#include "TCanvas.h"
#include "TH1F.h"
#include "TLegend.h"
#include "TF1.h"
#include "TPad.h"
#include <vector>
#include <iostream>
#include <fstream>
#include "TStyle.h"

void plot_multiple_spectra_v1() {
    // List of ROOT files
    std::vector<std::string> fileNames = {
        "Output_4232/Calib_Run4232_Ron100.root",
        "Output_4230/Calib_Run4230_Ron100.root",
        "Output_4225/Calib_Run4225_Ron100.root"
    };
    std::string histName = "energyHist";

    // Define the fitting ROI
    double ROI_min = 4540, ROI_max = 4660; 

    // Create canvas with subpads
    TCanvas* c = new TCanvas("c", "Multiple Spectra", 800, 1000);
    int nFiles = fileNames.size();
    c->Divide(1, nFiles);  // Create vertical subplots

    // std::ofstream outFile("fit_results.txt");
    // outFile << "FileName, PeakPosition, FWHM, GrossCounts\n";
    // Set style
    gStyle->SetStatFontSize(0.06); 
    gStyle->SetStatW(0.2);         
    gStyle->SetStatH(0.4);

    // Loop over files and plot histograms
    // Loop over files and plot histograms
    for (size_t i = 0; i < nFiles; ++i) {
        c->cd(i + 1);  
        TPad* pad = (TPad*)gPad;
        pad->SetTopMargin(0.0);

        TFile* file = TFile::Open(fileNames[i].c_str(), "READ");
        if (!file || file->IsZombie()) {
            std::cerr << "Error: Could not open file " << fileNames[i] << std::endl;
            continue;
        }

        // Get histogram
        TH1F* hist = (TH1F*)file->Get(histName.c_str());
        if (!hist) {
            std::cerr << "Error: Could not find histogram in " << fileNames[i] << std::endl;
            file->Close();
            continue;
        }

        // Clone and customize histogram
        TH1F* histClone = (TH1F*)hist->Clone(("histClone" + std::to_string(i)).c_str());
        //histClone->SetLineColor(colors[i % colors.size()]);
        histClone->SetLineWidth(1);
        histClone->GetXaxis()->SetRangeUser(ROI_min, ROI_max);
        histClone->SetTitle("; Energy [keV]; Counts");
        histClone->SetStats(0);
        histClone->GetXaxis()->SetTitleSize(0.06);
        histClone->GetYaxis()->SetTitleSize(0.06);
        histClone->GetXaxis()->SetLabelSize(0.06);
        histClone->GetYaxis()->SetLabelSize(0.06);
        histClone->SetTitle("");
        histClone->SetTitle("; Energy [keV]; Counts");
        histClone->SetStats(1);
        // gStyle->SetStatFontSize(0.02); 
        // gStyle->SetStatW(0.1);         
        // gStyle->SetStatH(0.1);
        histClone->Draw("HIST");

        // Fit the histogram with Gaussian + Pol3
        TF1* fitFunc = new TF1("fitFunc", "gaus(0) + pol0(3)", 4570, 4640);
        fitFunc->SetLineColor(kRed);
        fitFunc->SetLineWidth(2);

        // Set initial parameters
        fitFunc->SetParameters(100, 4600, 10);
        
        histClone->Fit(fitFunc, "R");
        fitFunc->Draw("SAME");

        // // Get fit results
        // double peakPos = fitFunc->GetParameter(1);
        // double sigma = fitFunc->GetParameter(2);
        // double FWHM = 2.355 * sigma;
        // Display results
        //std::cout << "File: " << fileNames[i] << ", Peak: " << peakPos << " keV, FWHM: " << FWHM << " keV, Gross Counts: " << grossCounts << std::endl;

        // Save to file
        //outFile << fileNames[i] << ", " << peakPos << ", " << FWHM << ", " << grossCounts << "\n";

        //file->Close();
    }

    // Save canvas
    c->Update();
    c->SaveAs("Subplot_Spectra_Fit_4592keV.png");
    std::cout << "Plot saved as Subplot_Spectra_Fit.png" << std::endl;

    //outFile.close();
}
