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

void plot_multiple_spectra_v2() {
    // List of ROOT files
    std::vector<std::string> fileNames = {
        "Output_4232/Calib_Run4232_Ron100.root",
        "Output_4229/Calib_Run4229_Ron100.root",
        "Output_4227/Calib_Run4227_Ron100.root"
    };
    std::string histName = "energyHist";

     // List of charge values corresponding to each file
    std::vector<double> charges = {5.05, 6.26, 5.62}; // (in mC) Example values, replace with actual charges
    
    // Define the fitting ROIs and peak positions for each file
    std::vector<double> ROI_min = {2515, 2520, 2525};
    std::vector<double> ROI_max = {2555, 2555, 2565};
    std::vector<double> peakPositions = {2538, 2542, 2545};


    // Define the fitting ROI
    // double ROI_min = 2510, ROI_max = 2570; 

    // Create canvas with subpads
    TCanvas* c = new TCanvas("c", "Multiple Spectra");
    int nFiles = fileNames.size();
    //c->Divide(1,nFiles);  // Create vertical subplots
    c->Divide(nFiles,1);  // Create horizontal subplots

    // std::ofstream outFile("fit_results.txt");
    // outFile << "FileName, PeakPosition, FWHM, GrossCounts\n";
    // Set style
    //gstyle->SetStat(0);
    // gStyle->SetStatFontSize(0.06); 
    // gStyle->SetStatW(0.2);         
    // gStyle->SetStatH(0.4);

    // Loop over files and plot histograms
    for (size_t i = 0; i < nFiles; ++i) {
        c->cd(i + 1);  
        TPad* pad = (TPad*)gPad;
        pad->SetTopMargin(0.0);
        pad->SetBottomMargin(0.1);
        pad->SetRightMargin(0.02);
        pad->SetLeftMargin(0.2);
        // pad->SetLogy();

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
        histClone->Scale(1.0 / charges[i]); // Normalize by charge
        histClone->SetLineWidth(1);
        histClone->GetXaxis()->SetRangeUser(2510, 2570);
        histClone->GetYaxis()->SetRangeUser(0, 34);
        //histClone->SetTitle("; Energy [keV]; Counts/C");
        histClone->SetStats(0);
        histClone->GetXaxis()->SetTitleSize(0.06);
        histClone->GetYaxis()->SetTitleSize(0.06);
        histClone->GetXaxis()->SetLabelSize(0.06);
        histClone->GetYaxis()->SetLabelSize(0.06);
        histClone->GetXaxis()->SetTitleOffset(0.8);
        histClone->GetYaxis()->SetTitleOffset(1.0);
        histClone->GetYaxis()->SetMoreLogLabels(); //
        histClone->GetXaxis()->SetNdivisions(505);


        //histClone->GetYaxis()->SetNoExponent(false); // 
        // histClone->GetYaxis()->SetDecimals(true); // Set y-axis to scientific notation
        // histClone->GetYaxis()->SetTitleOffset(0.8);
        // histClone->GetXaxis()->SetTitleOffset(0.8);
        // histClone->GetXaxis()->SetNdivisions(505);
        //histClone->SetTitle("");
        histClone->SetTitle("; Energy [keV]; Counts/mC");
        histClone->SetStats(0);
        // gStyle->SetStatFontSize(0.02); 
        // gStyle->SetStatW(0.1);         
        // gStyle->SetStatH(0.1);
        histClone->Draw("HIST");

        // Draw a vertical line at the fixed peak position 2539 keV
        TLine* line = new TLine(2539, 0, 2539, histClone->GetMaximum());
        line->SetLineColor(kOrange);
        line->SetLineStyle(2);
        line->Draw("SAME");
        // Fit the histogram with Gaussian + Pol3
        TF1* fitFunc = new TF1("fitFunc", "gaus(0) + pol0(3)", ROI_min[i], ROI_max[i]);
        fitFunc->SetLineColor(kRed);
        fitFunc->SetLineWidth(2);

        // Set initial parameters
        fitFunc->SetParameters(0.03, peakPositions[i], 10);
        
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
    c->SaveAs("Subplot_Spectra_DPG_2539keV.png");
    std::cout << "Plot saved as Subplot_Spectra_Fit.png" << std::endl;

    //outFile.close();
}
