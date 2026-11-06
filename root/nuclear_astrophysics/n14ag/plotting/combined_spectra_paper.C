#include <iostream>
#include "TFile.h"
#include "TH1.h"
#include "TCanvas.h"
#include "TLegend.h"
#include "TROOT.h"
#include "TStyle.h"
#include "TPad.h"
#include "TLatex.h"
#include "TSystem.h"

void combined_spectra_paper() {
    // ==========================================
    // 1. Set Global Styles
    // ==========================================
    gStyle->SetOptStat(0);
    gStyle->SetLegendBorderSize(1);
    gStyle->SetLegendFillColor(0);
    
    // ==========================================
    // 2. Open Both ROOT Files
    // ==========================================
    TFile *file_573 = TFile::Open("/felsdata-till08apr2026/n14ag_Felsenkeller_2025/Rootfiles_high/Run4620_high_Tree.root", "READ");
    if (!file_573 || file_573->IsZombie()) {
        std::cerr << "Error: Could not open 573 file!" << std::endl;
        return;
    }

    TFile *file_1618 = TFile::Open("/felsdata-till08apr2026/n14ag_Felsenkeller_2025/Rootfiles_high/Run4414_high_Tree.root", "READ");
    if (!file_1618 || file_1618->IsZombie()) {
        std::cerr << "Error: Could not open 1618 file!" << std::endl;
        return;
    }

    // Get Histograms
    TH1 *hist_573 = (TH1*)file_573->Get("histoCal/histoCal[06]");
    TH1 *hist_1618 = (TH1*)file_1618->Get("histoCal/histoCal[06]");

    // ==========================================
    // 3. Create 1x2 Canvas
    // ==========================================
    // 1200 width gives 600px per pad, similar to your original individual canvases
    // TCanvas *canvas = new TCanvas("canvas", "Combined Spectra", 600, 600);
    // canvas->Divide(1, 2); 
    TCanvas *canvas = new TCanvas("canvas", "Combined Spectra", 600, 600);

    // Top pad (45%)
    TPad *pad1 = new TPad("pad1", "", 0.0, 0.55, 1.0, 1.0);
    pad1->Draw();

    // Bottom pad (55%)
    TPad *pad2 = new TPad("pad2", "", 0.0, 0.00, 1.0, 0.53);
    pad2->Draw();
// draw bottom histogram

    // Common styling variables
    int color1 = kBlue;
    int lineWidth = 1;
    const int labelTextFont = 42;
    const double labelTextAngle = -90;

    // ==========================================
    // 4. PAD 1: 573 keV Spectrum
    // ==========================================
    pad1->cd();
    gPad->SetLogy();
    gPad->SetTopMargin(0.02);
    gPad->SetRightMargin(0.03);
    gPad->SetLeftMargin(0.08);
    gPad->SetBottomMargin(0.0);

    const double titleSize_573 = 0.06;
    const double labelSize_573 = 0.053;

    hist_573->SetLineColor(color1);
    hist_573->SetLineWidth(lineWidth);
    hist_573->GetYaxis()->SetTickLength(0.015); 
    // hist_573->GetYaxis()->SetRangeUser(1e0, 6e4);
    hist_573->SetMaximum(6e4);
    hist_573->GetXaxis()->SetRangeUser(0.0, 6.0);
    hist_573->GetXaxis()->SetNdivisions(520, kTRUE);
    hist_573->GetXaxis()->SetDecimals(kTRUE);
    hist_573->SetMaximum(5e4);
    hist_573->SetTitle("");
    // hist_573->GetXaxis()->SetTitle("E_{#gamma} [MeV]");
    // Remove the X-axis title
    hist_573->GetXaxis()->SetTitle("");
    // Hide the X-axis labels (the numbers)
    hist_573->GetXaxis()->SetLabelSize(0);
    hist_573->GetYaxis()->SetTitle("Counts / 2 keV");
    hist_573->GetXaxis()->CenterTitle(true);
    hist_573->GetYaxis()->CenterTitle(true);
    hist_573->GetYaxis()->SetTitleSize(titleSize_573);
    hist_573->GetXaxis()->SetTitleSize(titleSize_573);
    hist_573->GetYaxis()->SetLabelSize(labelSize_573);
    hist_573->GetXaxis()->SetTitleOffset(1.2);
    hist_573->GetYaxis()->SetTitleOffset(0.6);
    hist_573->SetFillColorAlpha(kBlue-9, 0.35);
    hist_573->SetFillStyle(1001);
    hist_573->Draw();

    // Zoomed inset pad for Pad 1
    TH1 *histZoom = (TH1*)hist_573->Clone("histZoom");
    TPad *padZoom = new TPad("padZoom", "Zoom", 0.58, 0.35, 0.95, 0.95);
    padZoom->SetFillColor(0);
    padZoom->SetLineWidth(1);
    padZoom->SetLeftMargin(0.15);
    padZoom->SetRightMargin(0.05);
    padZoom->SetBottomMargin(0.19);
    padZoom->SetTopMargin(0.05);
    padZoom->Draw();
    padZoom->cd();
    padZoom->SetLogy(0);

    histZoom->GetXaxis()->SetRangeUser(3.7, 3.9);
    histZoom->GetYaxis()->SetRangeUser(1, 10);
    histZoom->GetXaxis()->SetTitle("E_{#gamma} [MeV]");
    histZoom->GetYaxis()->SetTitle("Counts / 2 keV");
    histZoom->GetXaxis()->SetTitleSize(0.09);
    histZoom->GetYaxis()->SetTitleSize(0.09);
    histZoom->GetXaxis()->SetTitleOffset(0.9);
    histZoom->GetYaxis()->SetTitleOffset(0.7);
    histZoom->GetXaxis()->SetLabelSize(0.08);
    histZoom->GetYaxis()->SetLabelSize(0.08);
    histZoom->GetXaxis()->SetNdivisions(505);
    histZoom->GetYaxis()->SetNdivisions(505);
    histZoom->Draw("HIST");

    TLatex *zoomLabel = new TLatex(3.785, 8.5, "R #rightarrow 1041");
    zoomLabel->SetTextSize(0.09);
    zoomLabel->SetTextFont(42);
    zoomLabel->Draw();

    // Back to Pad 1 to draw main labels
    // canvas->cd(1); 
    pad1->cd();
    const double labelTextSize_573 = 0.050;

    TLatex *label3a = new TLatex(1.380, 2.5e4, "^{40}K");
    label3a->SetTextFont(42); label3a->SetTextSize(labelTextSize_573); label3a->Draw();

    TLatex *label3b = new TLatex(2.450, 8e3, "^{208}Tl");
    label3b->SetTextFont(42); label3b->SetTextSize(labelTextSize_573); label3b->Draw();

    TLatex *label3d = new TLatex(3.600, 10, "R #rightarrow 1041");
    label3d->SetTextFont(42); label3d->SetTextSize(labelTextSize_573); label3d->Draw();
    
    TLatex *label3u = new TLatex(1.00, 10, "E_{#alpha} = 602 keV "); 
    label3u->SetTextFont(42); label3u->SetTextSize(0.055); label3u->SetTextAngle(0); label3u->Draw();


    // ==========================================
    // 5. PAD 2: 1618 keV Spectrum
    // ==========================================
    // canvas->cd(2);
    canvas->cd();
    pad2->cd();
    gPad->SetLogy();
    gPad->SetTopMargin(0.0);
    gPad->SetRightMargin(0.03);
    gPad->SetLeftMargin(0.08); // Adjusted slightly to match Pad 1 cleanly
    gPad->SetBottomMargin(0.12); // Fixed to 0.1 so X-axes align beautifully across the canvas

    const double titleSize_1618 = 0.050;
    const double labelSize_1618 = 0.047;
    
    hist_1618->SetLineColor(color1);
    hist_1618->SetLineWidth(lineWidth);
    hist_1618->GetYaxis()->SetTickLength(0.01); 
    hist_1618->GetXaxis()->SetRangeUser(0.0, 6.0);
    hist_1618->GetXaxis()->SetNdivisions(520, kTRUE);
    hist_1618->GetXaxis()->SetDecimals(kTRUE);
    hist_1618->SetMaximum(2e5);
    hist_1618->SetTitle("");
    hist_1618->GetXaxis()->SetTitle("E_{#gamma} [MeV]");
    hist_1618->GetYaxis()->SetTitle("Counts / 2 keV");
    hist_1618->GetXaxis()->CenterTitle(true);
    hist_1618->GetYaxis()->CenterTitle(true);
    hist_1618->GetYaxis()->SetTitleSize(titleSize_1618);
    hist_1618->GetXaxis()->SetTitleSize(0.052);
    hist_1618->GetYaxis()->SetLabelSize(labelSize_1618);
    hist_1618->GetXaxis()->SetLabelSize(0.051);
    hist_1618->GetXaxis()->SetTitleOffset(1.0);
    hist_1618->GetYaxis()->SetTitleOffset(0.8);
    hist_1618->SetFillColorAlpha(kBlue-9, 0.35);
    hist_1618->SetFillStyle(1001);
    hist_1618->Draw();

    const double labelTextSize_1618 = 0.042;

    // Draw all the 1618 labels
    TLatex *label1a = new TLatex(1.440, 5e4, "^{40}K"); label1a->SetTextFont(42); label1a->SetTextSize(labelTextSize_1618); label1a->SetTextAngle(-90); label1a->Draw();
    TLatex *label1b = new TLatex(2.600, 2e4, "^{208}Tl"); label1b->SetTextFont(42); label1b->SetTextSize(labelTextSize_1618); label1b->SetTextAngle(-90); label1b->Draw();
    TLatex *label1c = new TLatex(4.530, 1.5e5, "R #rightarrow 1080"); label1c->SetTextFont(42); label1c->SetTextSize(labelTextSize_1618); label1c->SetTextAngle(-90); label1c->Draw();
    TLatex *label1d = new TLatex(2.460, 1.8e5, "R #rightarrow 3133"); label1d->SetTextFont(42); label1d->SetTextSize(labelTextSize_1618); label1d->SetTextAngle(-90); label1d->Draw();
    TLatex *label2d = new TLatex(1.700, 5e4, "1700 #rightarrow 0"); label2d->SetTextFont(42); label2d->SetTextSize(labelTextSize_1618); label2d->SetTextAngle(-90); label2d->Draw();
    TLatex *label1e = new TLatex(1.100, 2e5, "1080 #rightarrow 0"); label1e->SetTextFont(labelTextFont); label1e->SetTextSize(labelTextSize_1618); label1e->SetTextAngle(labelTextAngle); label1e->Draw();
    // TLatex *label2e = new TLatex(1.0204, 2e5, "1041 #rightarrow 0"); label2e->SetTextFont(labelTextFont); label2e->SetTextSize(labelTextSize_1618); label2e->SetTextAngle(labelTextAngle); label2e->Draw();
    TLatex *label3e = new TLatex(0.880, 1.5e5, "937 #rightarrow 0"); label3e->SetTextFont(labelTextFont); label3e->SetTextSize(labelTextSize_1618); label3e->SetTextAngle(labelTextAngle); label3e->Draw();
    TLatex *label4e = new TLatex(0.410, 1.8e5, "511"); label4e->SetTextFont(labelTextFont); label4e->SetTextSize(labelTextSize_1618); label4e->SetTextAngle(labelTextAngle); label4e->Draw();
    TLatex *label1f = new TLatex(5.610, 9e3, "R #rightarrow 0"); label1f->SetTextFont(labelTextFont); label1f->SetTextSize(labelTextSize_1618); label1f->SetTextAngle(labelTextAngle); label1f->Draw();
    TLatex *label1g = new TLatex(4.650, 1e4, "R #rightarrow 1042"); label1g->SetTextFont(labelTextFont); label1g->SetTextSize(labelTextSize_1618); label1g->SetTextAngle(labelTextAngle); label1g->Draw();
    TLatex *label1h = new TLatex(3.522, 7e3, "DE"); label1h->SetTextFont(labelTextFont); label1h->SetTextSize(labelTextSize_1618); label1h->SetTextAngle(labelTextAngle); label1h->Draw();
    // TLatex *label2h = new TLatex(3.598, 2e3, "DE"); label2h->SetTextFont(labelTextFont); label2h->SetTextSize(labelTextSize_1618); label2h->SetTextAngle(labelTextAngle); label2h->Draw();
    TLatex *label1i = new TLatex(3.105, 6e4, "3133 #rightarrow 0"); label1i->SetTextFont(labelTextFont); label1i->SetTextSize(labelTextSize_1618); label1i->SetTextAngle(labelTextAngle); label1i->Draw();
    TLatex *label1l = new TLatex(2.000, 1.8e5, "3133 #rightarrow 1041"); label1l->SetTextFont(labelTextFont); label1l->SetTextSize(labelTextSize_1618); label1l->SetTextAngle(labelTextAngle); label1l->Draw();
    // TLatex *label1m = new TLatex(1.980, 1.8e5, "3133 #rightarrow 1080"); label1m->SetTextFont(labelTextFont); label1m->SetTextSize(labelTextSize_1618); label1m->SetTextAngle(labelTextAngle); label1m->Draw();
    // TLatex *label2m = new TLatex(1.360, 1.5e5, "3133 #rightarrow 1700"); label2m->SetTextFont(labelTextFont); label2m->SetTextSize(labelTextSize_1618); label2m->SetTextAngle(labelTextAngle); label2m->Draw();
    TLatex *label1n = new TLatex(2.140, 1.8e5, "3061 #rightarrow 937"); label1n->SetTextFont(labelTextFont); label1n->SetTextSize(labelTextSize_1618); label1n->SetTextAngle(labelTextAngle); label1n->Draw();
    TLatex *label1o = new TLatex(3.000, 1.3e4, "3061 #rightarrow 0"); label1o->SetTextFont(labelTextFont); label1o->SetTextSize(labelTextSize_1618); label1o->SetTextAngle(labelTextAngle); label1o->Draw();
    TLatex *label1p = new TLatex(5.112, 2e3, "SE"); label1p->SetTextFont(labelTextFont); label1p->SetTextSize(labelTextSize_1618); label1p->SetTextAngle(labelTextAngle); label1p->Draw();
    TLatex *label1q = new TLatex(4.110, 4e3, "SE"); label1q->SetTextFont(labelTextFont); label1q->SetTextSize(labelTextSize_1618); label1q->SetTextAngle(labelTextAngle); label1q->Draw();
    TLatex *label1r = new TLatex(4.040, 1e4, "SE"); label1r->SetTextFont(labelTextFont); label1r->SetTextSize(labelTextSize_1618); label1r->SetTextAngle(labelTextAngle); label1r->Draw();
    TLatex *label1s = new TLatex(3.920, 2e4, "R #rightarrow 1700"); label1s->SetTextFont(labelTextFont); label1s->SetTextSize(labelTextSize_1618); label1s->SetTextAngle(labelTextAngle); label1s->Draw();
    TLatex *label1t = new TLatex(1.00, 10, "E_{#alpha} = 1628 keV "); label1t->SetTextFont(labelTextFont); label1t->SetTextSize(0.051); label1t->SetTextAngle(0); label1t->Draw();
    // ==========================================
    // 6. Save and Cleanup
    // ==========================================
    TString output_dir = "/felsdata-till08apr2026/n14ag_Felsenkeller_2025/Anup/Paper/";
    gSystem->mkdir(output_dir, true);
    
    TString output_file_pdf = output_dir + "spectra_combined_paper_v1_260630.pdf";
    TString output_file_root = output_dir + "spectra_combined_paper_v1_260630.root";
    
    canvas->SaveAs(output_file_pdf);
    canvas->SaveAs(output_file_root);

    std::cout << "Successfully saved combined plots!" << std::endl;
    std::cout << "Enter to continue..." << std::endl;
    std::cin.ignore();
    
    file_573->Close();
    file_1618->Close();
}