#include "TFile.h"
#include "TH1.h"
#include "TCanvas.h"
#include "TLegend.h"
#include "TROOT.h"
#include "TStyle.h"
#include "TPad.h"

void multiplot_v2() {
    // Set overall style
    gStyle->SetOptStat(0);
    gStyle->SetPadTickX(1);
    gStyle->SetPadTickY(1);
    gStyle->SetLegendBorderSize(1);
    gStyle->SetLegendFillColor(0);
    
    // Open the ROOT files
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

    // Get the histograms
    // TH1 *hist1 = (TH1*)file1->Get("histoCal/histoCal[06]");
    // TH1 *hist2 = (TH1*)file3->Get("histoCal/histoCal[06]");
    // TH1 *hist3 = (TH1*)file2->Get("histoCal/histoCal[06]");

    // Get the histograms
    TH1 *hist1 = (TH1*)file1->Get("histoCal/histoEB18noDE");
    TH1 *hist2 = (TH1*)file3->Get("histoCal/histoEB18noDE");
    TH1 *hist3 = (TH1*)file2->Get("histoCal/histoEB18noDE");


    // // Get realtimes (unchanged from your original code)
    // TNamed* named1 = (TNamed *)file1->Get("RunInfo/Realtime[s]");
    // double rt1 = 0;
    // double rt = 0;
    // if (named1) {
    //     sscanf(named1->GetTitle(), "%lf", &rt);
    //     rt1 = rt;
    // } else {
    //     std::cerr << "Warning: Could not read realtime for run " << std::endl;
    // }
    // std::cout << "Run 4414 Realtime: " << rt1 << " seconds" << std::endl;
    
    // TNamed* named2 = (TNamed *)file2->Get("RunInfo/Realtime[s]");
    // double rt2 = 0;
    // if (named2) {
    //     sscanf(named2->GetTitle(), "%lf", &rt2);
    //     rt2 = rt2;
    // } else {
    //     std::cerr << "Warning: Could not read realtime for run " << std::endl;
    // }
    // std::cout << "Run 4422 Realtime: " << rt2 << " seconds" << std::endl;

    // Create canvas with better dimensions
    TCanvas *canvas = new TCanvas("canvas", "", 1400, 700);
    canvas->Divide(1, 3, 0.001, 0.001); // Divide with small margins between pads
    // canvas->SetLeftMargin(0.00);
    // canvas->SetRightMargin(0.0);
    
    // Set common histogram styling
    const double titleSize = 0.07;
    const double labelSize = 0.06;
    const double titleOffset = 0.4;
    
    // using ROOT's vibrant palette
    // int color1 = kAzure+4;      // Deep blue
    // int color2 = kRed+1;         // Bright red
    // int color3 = kGreen+2;       // Vibrant green

    // High-contrast colors
    // int color1 = kBlack;     // Black
    // int color2 = kViolet+2;  // Purple
    // int color3 = kOrange+7;  // Orange

    // ROOT's color palette
    // int color1 = kBlue;
    // int color2 = kMagenta+1;
    // int color3 = kCyan+2;
    
    int color1 = kBlue;
    int color2 = kOrange+7;
    int color3 = kBlack;
    // Line widths
    int lineWidth = 1;

    // Pad 1: First spectrum
    canvas->cd(1);
    canvas->SetTopMargin(0.03);
    canvas->SetRightMargin(0.02);
    gPad->SetLogy();
    gPad->SetLeftMargin(0.06);
    gPad->SetRightMargin(0.03);
    gPad->SetBottomMargin(0.02);
    gPad->SetTopMargin(0.02);
    
    hist1->SetLineColor(color1);
    hist1->SetLineWidth(lineWidth);
    hist1->GetYaxis()->SetTickLength(0.01); 
    hist1->GetXaxis()->SetRangeUser(0.0, 6.0);
    hist1->SetMaximum(1e6);
    hist1->SetTitle("");
    hist1->GetXaxis()->SetTitle("E_{#gamma} [MeV]");
    hist1->GetYaxis()->SetTitle("Counts / 2 keV");
    hist1->GetXaxis()->SetTitleSize(0);
    hist1->GetYaxis()->SetTitleSize(titleSize);
    hist1->GetXaxis()->SetLabelSize(0);
    hist1->GetYaxis()->SetLabelSize(labelSize);
    // hist1->GetXaxis()->SetTitleOffset(titleOffset);
    hist1->GetYaxis()->SetTitleOffset(titleOffset);
    hist1->SetTitleSize(titleSize, "t"); // Title size
    hist1->SetTitleFont(62, "t"); // Bold title
    hist1->Draw();
    // Label the peaks

    TLatex *label1a = new TLatex(1.460, 4e5, "BG");
    label1a->SetTextFont(42);
    label1a->SetTextSize(0.05);
    // label1a->SetTextColor(kBlue+2);
    label1a->SetTextAngle(-90);
    label1a->Draw();

    TLatex *label1b = new TLatex(2.610, 2e5, "^{208}Tl");
    label1b->SetTextFont(42);
    label1b->SetTextSize(0.05);
    // label1a->SetTextColor(kBlue+2);
    label1b->SetTextAngle(-90);
    label1b->Draw();

    TLatex *label1c = new TLatex(4.580, 7e5, "R #rightarrow 1080"); // 4.590 gamma
    label1c->SetTextFont(42);
    label1c->SetTextSize(0.05);
    label1c->SetTextAngle(-90);
    label1c->Draw();

    TLatex *label1d = new TLatex(2.520, 8e5, "R #rightarrow 3133");  // 2.539 gamma 
    label1d->SetTextFont(42);
    label1d->SetTextSize(0.05);
    label1d->SetTextAngle(-90);
    label1d->Draw();

    // Define common label properties
    const double labelTextSize = 0.05;
    const int labelTextFont = 42;
    const double labelTextAngle = -90; // Rotate by 180 degrees

    TLatex *label1e = new TLatex(1.084, 4e5, "1084 #rightarrow 0");
    label1e->SetTextFont(labelTextFont);
    label1e->SetTextSize(labelTextSize);
    label1e->SetTextAngle(labelTextAngle);
    label1e->Draw();

    TLatex *label1f = new TLatex(5.673, 4e4, "R #rightarrow 0");
    label1f->SetTextFont(labelTextFont);
    label1f->SetTextSize(labelTextSize);
    label1f->SetTextAngle(labelTextAngle);
    label1f->Draw();

    TLatex *label1g = new TLatex(4.640, 7e5, "R #rightarrow 1041");
    label1g->SetTextFont(labelTextFont);
    label1g->SetTextSize(labelTextSize);
    label1g->SetTextAngle(labelTextAngle);
    label1g->Draw();

    TLatex *label1h = new TLatex(3.572, 2e5, "R #rightarrow 2100");
    label1h->SetTextFont(labelTextFont);
    label1h->SetTextSize(labelTextSize);
    label1h->SetTextAngle(labelTextAngle);
    label1h->Draw();

    TLatex *label1i = new TLatex(3.134, 4e5, "3133 #rightarrow 0");
    label1i->SetTextFont(labelTextFont);
    label1i->SetTextSize(labelTextSize);
    label1i->SetTextAngle(labelTextAngle);
    label1i->Draw();

    // TLatex *label1j = new TLatex(2.611, 2e5, "R #rightarrow 3061");
    // label1j->SetTextFont(labelTextFont);
    // label1j->SetTextSize(labelTextSize);
    // label1j->SetTextAngle(labelTextAngle);
    // label1j->Draw();

    // TLatex *label1k = new TLatex(3.133, 4e5, "3133 #rightarrow 0");
    // label1k->SetTextFont(labelTextFont);
    // label1k->SetTextSize(labelTextSize);
    // label1k->SetTextAngle(labelTextAngle);
    // label1k->Draw();

    TLatex *label1l = new TLatex(2.091, 8e5, "3133 #rightarrow 1041"); // 2.091 gamma
    label1l->SetTextFont(labelTextFont);
    label1l->SetTextSize(labelTextSize);
    label1l->SetTextAngle(labelTextAngle);
    label1l->Draw();

    TLatex *label1m = new TLatex(2.020, 8e5, "3133 #rightarrow 1080"); // 2.052 gamma
    label1m->SetTextFont(labelTextFont);
    label1m->SetTextSize(labelTextSize);
    label1m->SetTextAngle(labelTextAngle);
    label1m->Draw();

    TLatex *label1n = new TLatex(2.150, 8e5, "3061 #rightarrow 937"); // 2.125 gamma
    label1n->SetTextFont(labelTextFont);
    label1n->SetTextSize(labelTextSize);
    label1n->SetTextAngle(labelTextAngle);
    label1n->Draw();

    TLatex *label1o = new TLatex(3.050, 1e5, "3061 #rightarrow 0"); // 3.061 gamma
    label1o->SetTextFont(labelTextFont);
    label1o->SetTextSize(labelTextSize);
    label1o->SetTextAngle(labelTextAngle);
    label1o->Draw();

    // Add grid
    gPad->SetGridx();
    gPad->SetGridy();
    
    // Pad 2: Second spectrum
    canvas->cd(2);
    canvas->SetTopMargin(0.01);
    canvas->SetRightMargin(0.01);
    gPad->SetLogy();
    gPad->SetLeftMargin(0.06);
    gPad->SetRightMargin(0.03);
    gPad->SetBottomMargin(0.02);
    gPad->SetTopMargin(0.02);
    
    hist2->SetLineColor(color2);
    hist2->SetLineWidth(lineWidth);
    hist2->GetYaxis()->SetTickLength(0.01); 
    hist2->GetXaxis()->SetRangeUser(0.0, 6.0);
    hist2->SetMaximum(1e6);
    hist2->SetTitle("");
    hist2->GetXaxis()->SetTitle("E_{#gamma} [MeV]");
    hist2->GetYaxis()->SetTitle("Counts / 2 keV");
    hist2->GetXaxis()->SetTitleSize(0);
    hist2->GetYaxis()->SetTitleSize(titleSize);
    hist2->GetXaxis()->SetLabelSize(0);
    hist2->GetYaxis()->SetLabelSize(labelSize);
    // hist2->GetXaxis()->SetTitleOffset(titleOffset);
    hist2->GetYaxis()->SetTitleOffset(titleOffset);
    hist2->SetTitleSize(titleSize, "t");
    hist2->SetTitleFont(62, "t");
    hist2->Draw();
     
    TLatex *label2a = new TLatex(1.460, 4e5, "BG");
    label2a->SetTextFont(42);
    label2a->SetTextSize(0.05);
    label2a->SetTextAngle(-90);
    label2a->Draw();

    TLatex *label2b = new TLatex(2.615, 1e5, "BG");
    label2b->SetTextFont(42);
    label2b->SetTextSize(0.05);
    label2b->SetTextAngle(-90);
    label2b->Draw();

    TLatex *label2c = new TLatex(2.775, 4e5, "R #rightarrow 2523");
    label2c->SetTextFont(42);
    label2c->SetTextSize(0.05);
    label2c->SetTextAngle(-90);
    label2c->Draw();

    TLatex *label2d = new TLatex(4.361, 1.5e4, "R #rightarrow 937");
    label2d->SetTextFont(42);
    label2d->SetTextSize(0.05);
    label2d->SetTextAngle(-90);
    label2d->Draw();
    TLatex *label2e = new TLatex(2.523, 3e5, "2523 #rightarrow 0");
    label2e->SetTextFont(42);
    label2e->SetTextSize(0.05);
    label2e->SetTextAngle(-90);
    label2e->Draw();
    TLatex *label2f = new TLatex(1.586, 4e5, "2523 #rightarrow 937");
    label2f->SetTextFont(42);
    label2f->SetTextSize(0.05);
    label2f->SetTextAngle(-90);
    label2f->Draw();

    TLatex *label2g = new TLatex(4.177, 5e4, "R #rightarrow 1121");
    label2g->SetTextFont(42);
    label2g->SetTextSize(0.05);
    label2g->SetTextAngle(-90);
    label2g->Draw();
    TLatex *label2h = new TLatex(5.292, 4e3, "R #rightarrow 0?");
    label2h->SetTextFont(42);
    label2h->SetTextSize(0.05);
    label2h->SetTextAngle(-90);
    label2h->Draw();

    TLatex *label2i = new TLatex(1.940, 2e5, "R #rightarrow 3358");
    label2i->SetTextFont(42);
    label2i->SetTextSize(0.05);
    label2i->SetTextAngle(-90);
    label2i->Draw();

    TLatex *label2j = new TLatex(0.646, 4e5, "R #rightarrow 4652");
    label2j->SetTextFont(42);
    label2j->SetTextSize(0.05);
    label2j->SetTextAngle(-90);
    label2j->Draw();

    TLatex *label2k = new TLatex(1.586, 4e5, "2523 #rightarrow 937");
    label2k->SetTextFont(42);
    label2k->SetTextSize(0.05);
    label2k->SetTextAngle(-90);
    label2k->Draw();

    TLatex *label2l = new TLatex(0.932, 4e5, "937 #rightarrow 0");
    label2l->SetTextFont(42);
    label2l->SetTextSize(0.05);
    label2l->SetTextAngle(-90);
    label2l->Draw();

    TLatex *label2m = new TLatex(0.822, 4e5, "2523 #rightarrow 1700");
    label2m->SetTextFont(42);
    label2m->SetTextSize(0.05);
    label2m->SetTextAngle(-90);
    label2m->Draw();
    TLatex *label2n = new TLatex(3.358, 1e4, "3358 #rightarrow 0");
    label2n->SetTextFont(42);
    label2n->SetTextSize(0.05);
    label2n->SetTextAngle(-90);
    label2n->Draw();

    TLatex *label2o = new TLatex(1.657, 4e5, "3358 #rightarrow 1700");
    label2o->SetTextFont(42);
    label2o->SetTextSize(0.05);
    label2o->SetTextAngle(-90);
    label2o->Draw();

    TLatex *label2p = new TLatex(3.715, 2e4, "4652 #rightarrow 937");
    label2p->SetTextFont(42);
    label2p->SetTextSize(0.05);
    label2p->SetTextAngle(-90);
    label2p->Draw();

    TLatex *label2q = new TLatex(3.531, 2e4, "4652 #rightarrow 1121");
    label2q->SetTextFont(42);
    label2q->SetTextSize(0.05);
    label2q->SetTextAngle(-90);
    label2q->Draw();

    TLatex *label2r = new TLatex(2.421, 2e5, "3358 #rightarrow 937");
    label2r->SetTextFont(42);
    label2r->SetTextSize(0.05);
    label2r->SetTextAngle(-90);
    label2r->Draw();









    // Add grid
    gPad->SetGridx();
    gPad->SetGridy();
    
    // Pad 3: Third spectrum
    canvas->cd(3);
    gPad->SetLogy();
    gPad->SetLeftMargin(0.06);
    gPad->SetRightMargin(0.03);
    gPad->SetBottomMargin(0.18);
    gPad->SetTopMargin(0.02);
    
    hist3->SetLineColor(color3);
    hist3->SetLineWidth(lineWidth);
    hist3->GetYaxis()->SetTickLength(0.01); 
    hist3->GetXaxis()->SetRangeUser(0.0, 6.0);
    // hist3->SetMinimum(0.1);
    hist3->SetMaximum(1e6);
    hist3->SetTitle("");
    hist3->GetXaxis()->SetTitle("E_{#gamma} [MeV]");
    hist3->GetYaxis()->SetTitle("Counts / 2 keV");
    hist3->GetXaxis()->SetTitleSize(titleSize);
    hist3->GetYaxis()->SetTitleSize(titleSize);
    hist3->GetXaxis()->SetLabelSize(labelSize);
    hist3->GetYaxis()->SetLabelSize(labelSize);
    // hist3->GetXaxis()->SetTitleOffset(titleOffset);
    hist3->GetYaxis()->SetTitleOffset(titleOffset);
    hist3->SetTitleSize(titleSize, "t");
    hist3->SetTitleFont(62, "t");
    hist3->Draw();
    //legend->Draw();
    TLatex *label3a = new TLatex(1.460, 5e5, "BG");
    label3a->SetTextFont(42);
    label3a->SetTextSize(0.05);
    label3a->SetTextAngle(-90);
    label3a->Draw();

    TLatex *label3b = new TLatex(2.615, 2e5, "BG");
    label3b->SetTextFont(42);
    label3b->SetTextSize(0.05);
    label3b->SetTextAngle(-90);
    label3b->Draw();
    TLatex *label3c = new TLatex(1.798, 2e5, "R #rightarrow 3161");
    label3c->SetTextFont(42);
    label3c->SetTextSize(0.05);
    label3c->SetTextAngle(-90);
    label3c->Draw();
    TLatex *label3d = new TLatex(3.818, 1.5e3, "R #rightarrow 1041");
    label3d->SetTextFont(42);
    label3d->SetTextSize(0.05);
    label3d->SetTextAngle(-90);
    label3d->Draw();

    TLatex *label3e = new TLatex(1.041, 5e5, "1041 #rightarrow 0");
    label3e->SetTextFont(42);
    label3e->SetTextSize(0.05);
    label3e->SetTextAngle(-90);
    label3e->Draw();

    // TLatex *label3f = new TLatex(3.779, 2e2, "R #rightarrow 1080");
    // label3f->SetTextFont(42);
    // label3f->SetTextSize(0.05);
    // label3f->SetTextAngle(-90);
    // label3f->Draw();


    // Add grid
    gPad->SetGridx();
    gPad->SetGridy();

    canvas->cd(1);
    TLegend *leg1 = new TLegend(0.80, 0.8, 0.95, 0.93);
    leg1->SetTextSize(0.08);
    leg1->AddEntry(hist1, "E_{#alpha} = 1618 keV", "l");
    leg1->Draw();

    canvas->cd(2); 
    TLegend *leg2 = new TLegend(0.80, 0.8, 0.95, 0.93);
    leg2->SetTextSize(0.08);
    leg2->AddEntry(hist2, "E_{#alpha} = 1136 keV", "l");
    leg2->Draw();

    canvas->cd(3);
    TLegend *leg3 = new TLegend(0.80, 0.8, 0.95, 0.93);
    leg3->SetTextSize(0.08); 
    leg3->AddEntry(hist3, "E_{#alpha} = 573 keV", "l");
    leg3->Draw();
    
    // // Add a common title
    // canvas->cd();
    // TLatex *title = new TLatex(0.5, 0.98, "Gamma-ray Spectra from Different Resonances");
    // title->SetTextAlign(22);
    // title->SetTextSize(0.03);
    // title->SetNDC();
    // title->Draw();
    
    // Save the canvas
    canvas->SaveAs("/felsdata/n14ag_Felsenkeller_2025/Anup/Paper/multiplot_EB18noDE_H25.pdf");
    canvas->SaveAs("/felsdata/n14ag_Felsenkeller_2025/Anup/Paper/multiplot_EB18noDE_H25.jpg", "jpg,dpi=600");

    std::cout << "Enter to continue" << std::endl;
    std::cin.ignore();
    
    // Clean up
    file1->Close();
    file2->Close();
    file3->Close();
}