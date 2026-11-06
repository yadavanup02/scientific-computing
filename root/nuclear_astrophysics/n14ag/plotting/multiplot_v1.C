#include "TFile.h"
#include "TH1.h"
#include "TCanvas.h"
#include "TLegend.h"
#include "TROOT.h"
#include "TStyle.h"
#include "TPad.h"

void multiplot_v1() {
    // Open the ROOT file
    TFile *file1 = TFile::Open("/felsdata/n14ag_Felsenkeller_2025/Rootfiles_high/Run4414_high_Tree.root", "READ");
    if (!file1 || file1->IsZombie()) {
        std::cerr << "Error: Could not open file!" << std::endl;
        return;
    }
    TFile *file2 = TFile::Open("/felsdata/n14ag_Felsenkeller_2025/Rootfiles_high/Run4620_high_Tree.root", "READ");
    if (!file2 || file2->IsZombie()) {
        std::cerr << "Error: Could not open file!" << std::endl;
        return;
    }
    TFile *file3 = TFile::Open("/felsdata/n14ag_Felsenkeller_2025/Rootfiles_high/Run4443_high_Tree.root", "READ");
    if (!file3 || file3->IsZombie()) {
        std::cerr << "Error: Could not open file!" << std::endl;
        return;
    }

    // Get the histograms (replace "hist1_name" and "hist2_name" with your actual histogram names)
    TH1 *hist1 = (TH1*)file1->Get("histoCal/histoEB18noDE");
    TH1 *hist2 = (TH1*)file2->Get("histoCal/histoEB18noDE");
    TH1 *hist3 = (TH1*)file3->Get("histoCal/histoEB18noDE");

    TNamed* named1 = (TNamed *)file1->Get("RunInfo/Realtime[s]");
    double rt1 = 0;
    double rt = 0;
    if (named1) {
        sscanf(named1->GetTitle(), "%lf", &rt);
        rt1 = rt;
    } else {
        std::cerr << "Warning: Could not read realtime for run " << std::endl;
    }
    std::cout << "Run 4414 Realtime: " << rt1 << " seconds" << std::endl;
    TNamed* named2 = (TNamed *)file2->Get("RunInfo/Realtime[s]");
    double rt2 = 0;
    if (named2) {
        sscanf(named2->GetTitle(), "%lf", &rt2);
        rt2 = rt2;
    } else {
        std::cerr << "Warning: Could not read realtime for run " << std::endl;
    }
    std::cout << "Run 4422 Realtime: " << rt2 << " seconds" << std::endl;

    // Create canvas
    TCanvas *canvas = new TCanvas("canvas", "Two Histograms Comparison", 800, 1000);
    canvas->Divide(1, 3);
    canvas->SetLogy();
    canvas->cd(1);
    canvas->SetLogy();
    // Remove top and right margins to reduce empty space
    canvas->SetTopMargin(0.02);
    canvas->SetRightMargin(0.02);
    hist1->GetYaxis()->SetTitleOffset(0.8);
    hist1->GetXaxis()->SetTitleOffset(0.8);
    
    gStyle->SetOptStat(0);
    // Customize histograms
    hist1->SetLineColor(TColor::GetColor("#FF9900")); // Vibrant orange
    // hist1->SetLineWidth(2);
    hist2->SetLineColor(kBlue); 
    hist1->GetXaxis()->SetRangeUser(0.0, 6.0);
    hist1->SetTitle("");
    hist1->GetXaxis()->SetTitle("Energy [MeV]");
    hist1->GetYaxis()->SetTitle("Counts/2 keV");
    hist1->GetXaxis()->SetTitleSize(0.05);
    hist1->GetYaxis()->SetTitleSize(0.05);
    hist1->GetXaxis()->SetLabelSize(0.045);
    hist1->GetYaxis()->SetLabelSize(0.045);
    // Increase legend font size
    gStyle->SetLegendTextSize(0.05);

    // Normalize histograms‚
    // hist1->Scale(43200./rt1);
    // hist2->Scale(43200./53374.0);

    // Draw histograms
    hist1->Draw();
    canvas->cd(2);
    hist2->GetXaxis()->SetTitle("Energy [MeV]");
    hist2->GetYaxis()->SetTitle("Counts/2 keV");
    hist2->GetXaxis()->SetTitleSize(0.05);
    hist2->GetYaxis()->SetTitleSize(0.05);
    hist2->GetXaxis()->SetLabelSize(0.045);
    hist2->GetYaxis()->SetLabelSize(0.045);
    hist2->SetTitle("");
    canvas->SetLogy();
    hist2->Draw("SAME");
    canvas->cd(3);
    hist3->GetXaxis()->SetTitle("Energy [MeV]");
    hist3->GetYaxis()->SetTitle("Counts/2 keV");
    hist3->GetXaxis()->SetTitleSize(0.05);
    hist3->GetYaxis()->SetTitleSize(0.05);
    hist3->GetXaxis()->SetLabelSize(0.045);
    hist3->GetYaxis()->SetLabelSize(0.045);
    hist3->SetTitle("");
    canvas->SetLogy();
    hist3->Draw("SAME");  
    
    // Add legend
    TLegend *legend = new TLegend(0.13, 0.2, 0.42, 0.37); // Adjusted position and size for compactness
    //legend->SetMargin(0.1);
    //legend->SetTextSize(0.035); // Slightly smaller text
    //legend->SetEntrySeparation(0.005); // Minimize vertical gap

    legend->AddEntry(hist1, "resonance 1618 keV", "l");
    legend->AddEntry(hist3, "resonance 1136 keV", "l");
    legend->AddEntry(hist2, "resonance 573 keV", "l");
    legend->Draw();

    // Save the canvas
    canvas->SaveAs("/felsdata/n14ag_Felsenkeller_2025/Anup/multiplot_v2.pdf");
    
    std::cout << "Enter to continue" << std::endl;
    std::cin.ignore();
    
    // Clean up
    file1->Close();
    file2->Close();
    file3->Close();
}