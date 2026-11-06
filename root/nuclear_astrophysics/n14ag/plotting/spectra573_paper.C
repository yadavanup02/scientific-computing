#include "TFile.h"
#include "TH1.h"
#include "TCanvas.h"
#include "TLegend.h"
#include "TROOT.h"
#include "TStyle.h"
#include "TPad.h"

void spectra573_paper() {
    // Set overall style
    gStyle->SetOptStat(0);
    // gStyle->SetPadTickX(1);
    // gStyle->SetPadTickY(1);
    gStyle->SetLegendBorderSize(1);
    gStyle->SetLegendFillColor(0);
    
    // Open the ROOT files
    TFile *file2 = TFile::Open("/felsdata/n14ag_Felsenkeller_2025/Rootfiles_high/Run4620_high_Tree.root", "READ");
    if (!file2 || file2->IsZombie()) {
        std::cerr << "Error: Could not open file!" << std::endl;
        return;
    }

    TH1 *hist3 = (TH1*)file2->Get("histoCal/histoCal[06]");
    // Create canvas with better dimensions
    TCanvas *canvas = new TCanvas("canvas", "", 600, 500);
    
    // Set common histogram styling
    const double titleSize = 0.040;
    const double labelSize = 0.035;
    const double titleOffset = 1.2;
    int color1 = kBlue;
    int color3 = kOrange+7;
    int color2 = kBlack;
    // Line widths
    int lineWidth = 1;
    // Label the peaks
    // Define common label properties
    const double labelTextSize = 0.035;
    const int labelTextFont = 42;
    const double labelTextAngle = -90; // Rotate by 180 degrees

    // // Pad 1: 573 keV
    canvas->cd(1);
    canvas->SetTopMargin(0.01);
    canvas->SetRightMargin(0.01);
    gPad->SetLogy();
    gPad->SetLeftMargin(0.08);
    gPad->SetRightMargin(0.03);
    gPad->SetBottomMargin(0.1);
    gPad->SetTopMargin(0.02);

    // Create legend for all pads (position: x1, y1, x2, y2 in NDC)
    TLegend* legend = new TLegend(0.70, 0.75, 0.92, 0.92);

    hist3->SetLineColor(color1);
    hist3->SetLineWidth(lineWidth);
    hist3->GetYaxis()->SetTickLength(0.015); 

    hist3->GetYaxis()->SetRangeUser(1e0, 6e4);
    hist3->GetXaxis()->SetRangeUser(0.0, 6.0);
    hist3->GetXaxis()->SetNdivisions(520, kTRUE); // Increase number of divisions (labels)
    hist3->GetXaxis()->SetDecimals(kTRUE);        // Show decimal digits
             // Show one decimal point
    hist3->SetMaximum(5e4);
    hist3->SetTitle("");
    hist3->GetXaxis()->SetTitle("E_{#gamma} [MeV]");
    hist3->GetYaxis()->SetTitle("Counts / 2 keV");
    hist3->GetXaxis()->CenterTitle(true);
    hist3->GetYaxis()->CenterTitle(true);
    hist3->GetYaxis()->SetTitleSize(titleSize);
    hist3->GetXaxis()->SetTitleSize(titleSize);
    hist3->GetYaxis()->SetLabelSize(labelSize);
    hist3->GetXaxis()->SetTitleOffset(1.2);
    hist3->GetYaxis()->SetTitleOffset(1.0);
    hist3->SetTitleSize(titleSize, "t"); // Title size
    hist3->SetTitleFont(62, "t"); // Bold title
    hist3->SetFillColorAlpha(kBlue-9, 0.35);  // light blue with transparency
    hist3->SetFillStyle(1001);                // solid fill
    hist3->Draw();

   // =======================
    // Zoomed inset pad
    // =======================

    // Clone histogram for zoom (important!)
    TH1 *histZoom = (TH1*)hist3->Clone("histZoom");

    // Create inset pad (top right)
    TPad *padZoom = new TPad("padZoom", "Zoom", 0.58, 0.58, 0.95, 0.95);
    padZoom->SetFillColor(0);
    padZoom->SetLineWidth(1);
    padZoom->SetLeftMargin(0.15);
    padZoom->SetRightMargin(0.05);
    padZoom->SetBottomMargin(0.17);
    padZoom->SetTopMargin(0.05);
    padZoom->Draw();
    padZoom->cd();

    padZoom->SetLogy(0);  // linear scale usually better for zoom

    // Set zoom range ONLY for cloned hist
    histZoom->GetXaxis()->SetRangeUser(3.7, 3.9);
    histZoom->GetYaxis()->SetRangeUser(1, 10);  // adjust



    // Axis style for inset
    histZoom->GetXaxis()->SetTitle("E_{#gamma} [MeV]");
    histZoom->GetYaxis()->SetTitle("Counts / 2 keV");

    // Title sizes (bigger for inset)
    histZoom->GetXaxis()->SetTitleSize(0.09);
    histZoom->GetYaxis()->SetTitleSize(0.09);

    // Title offsets (important!)
    histZoom->GetXaxis()->SetTitleOffset(0.9);   // smaller number = closer to axis
    histZoom->GetYaxis()->SetTitleOffset(0.7);   // pull Y title closer for inset

    histZoom->GetXaxis()->SetLabelSize(0.08);
    histZoom->GetYaxis()->SetLabelSize(0.08);
    // histZoom->GetXaxis()->SetTitleSize(0.09);
    // histZoom->GetYaxis()->SetTitleSize(0.09);
    histZoom->GetXaxis()->SetNdivisions(505);
    histZoom->GetYaxis()->SetNdivisions(505);

    histZoom->Draw("HIST");

    // Label inside inset
    TLatex *zoomLabel = new TLatex(3.785, 8.5, "R #rightarrow 1041");
    zoomLabel->SetTextSize(0.09);
    zoomLabel->SetTextFont(42);
    zoomLabel->SetTextAngle(0);
    zoomLabel->Draw();

    canvas->cd();  // back to main pad


    TLatex *label3a = new TLatex(1.380, 2.8e4, "^{40}K");
    label3a->SetTextFont(42);
    label3a->SetTextSize(labelTextSize);
    label3a->SetTextAngle(0);
    label3a->Draw();

    TLatex *label3b = new TLatex(2.450, 8e3, "^{208}Tl");
    label3b->SetTextFont(42);
    label3b->SetTextSize(labelTextSize);
    label3b->SetTextAngle(0);
    label3b->Draw();
    TLatex *label3d = new TLatex(3.600, 10, "R #rightarrow 1041");
    label3d->SetTextFont(42);
    label3d->SetTextSize(labelTextSize);
    label3d->SetTextAngle(0);
    label3d->Draw();

    TString output_dir = "/felsdata/n14ag_Felsenkeller_2025/Anup/Paper/"; // Directory to save the output
    gSystem->mkdir(output_dir, true);
    TString output_file = output_dir + "spectra_573_paper.pdf";
    canvas->SaveAs(output_file);
    std::cout << "Enter to continue" << std::endl;
    std::cin.ignore();
    
    // Clean up
    file2->Close();
}