#include "TFile.h"
#include "TH1.h"
#include "TCanvas.h"
#include "TLegend.h"
#include "TROOT.h"
#include "TStyle.h"
#include "TPad.h"

void spectra_1618() {
    // Set overall style
    gStyle->SetOptStat(0);
    gStyle->SetLegendBorderSize(1);
    gStyle->SetLegendFillColor(0);
    
    // Open the ROOT files
    TFile *file1 = TFile::Open("/felsdata-till08apr2026/n14ag_Felsenkeller_2025/Rootfiles_high/Run4414_high_Tree.root", "READ");
    if (!file1 || file1->IsZombie()) {
        std::cerr << "Error: Could not open file!" << std::endl;
        return;
    }
    // Get the histograms
    TH1 *hist1 = (TH1*)file1->Get("histoCal/histoCal[06]");

    // Create canvas with better dimensions
    TCanvas *canvas = new TCanvas("canvas", "", 1000, 600);
    // Set common histogram styling
    const double titleSize = 0.03;
    const double labelSize = 0.03;
    const double titleOffset = 0.9;
    
    int color1 = kBlue;
    int color3 = kOrange+7;
    int color2 = kBlack;
    // Line widths
    int lineWidth = 1;

    // Pad 1: First spectrum
    // canvas->cd(1);
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
    hist1->GetXaxis()->SetNdivisions(520, kTRUE); // Increase number of divisions (labels)
    hist1->GetXaxis()->SetDecimals(kTRUE);        // Show decimal digits
    // hist1->GetXaxis()->SetLabelFormat("%.1f");    // Show one decimal point
    hist1->SetMaximum(2e5);
    hist1->SetTitle("");
    hist1->GetXaxis()->SetTitle("E_{#gamma} [MeV]");
    hist1->GetYaxis()->SetTitle("Counts / 2 keV");
    // set the title in center
    hist1->GetXaxis()->CenterTitle(true);
    hist1->GetYaxis()->CenterTitle(true);
    // hist1->GetXaxis()->SetTitleSize(0);
    hist1->GetYaxis()->SetTitleSize(titleSize);
    // hist1->GetXaxis()->SetLabelSize(0);
    hist1->GetYaxis()->SetLabelSize(labelSize);
    hist1->GetXaxis()->SetTitleOffset(1.2);
    hist1->GetYaxis()->SetTitleOffset(titleOffset);
    hist1->SetTitleSize(titleSize, "t"); // Title size
    hist1->SetTitleFont(62, "t"); // Bold title
    hist1->Draw();
    // Label the peaks
    // Define common label properties
    const double labelTextSize = 0.025;
    const int labelTextFont = 42;
    const double labelTextAngle = -90; // Rotate by 180 degrees

    TLatex *label1a = new TLatex(1.450, 2.5e4, "^{40}K");
    label1a->SetTextFont(42);
    label1a->SetTextSize(labelTextSize);
    // label1a->SetTextColor(kBlue+2);
    label1a->SetTextAngle(-90);
    label1a->Draw();

    TLatex *label1b = new TLatex(2.600, 1e4, "^{208}Tl");
    label1b->SetTextFont(42);
    label1b->SetTextSize(labelTextSize);
    // label1a->SetTextColor(kBlue+2);
    label1b->SetTextAngle(-90);
    label1b->Draw();

    TLatex *label1c = new TLatex(4.570, 4e4, "R #rightarrow 1080"); // 4.590 gamma
    label1c->SetTextFont(42);
    label1c->SetTextSize(labelTextSize);
    label1c->SetTextAngle(-90);
    label1c->Draw();

    TLatex *label1d = new TLatex(2.520, 8e4, "R #rightarrow 3133");  // 2.539 gamma 
    label1d->SetTextFont(42);
    label1d->SetTextSize(labelTextSize);
    label1d->SetTextAngle(-90);
    label1d->Draw();

    TLatex *label2d = new TLatex(1.680, 1e4, "1700 #rightarrow 0");  // 2.539 gamma 
    label2d->SetTextFont(42);
    label2d->SetTextSize(labelTextSize);
    label2d->SetTextAngle(-90);
    label2d->Draw();

    

    TLatex *label1e = new TLatex(1.084, 2e5, "1080 #rightarrow 0");
    label1e->SetTextFont(labelTextFont);
    label1e->SetTextSize(labelTextSize);
    label1e->SetTextAngle(labelTextAngle);
    label1e->Draw();
    TLatex *label2e = new TLatex(1.0204, 2e5, "1041 #rightarrow 0");
    label2e->SetTextFont(labelTextFont);
    label2e->SetTextSize(labelTextSize);
    label2e->SetTextAngle(labelTextAngle);
    label2e->Draw();
    TLatex *label3e = new TLatex(0.915, 4.5e4, "937 #rightarrow 0");
    label3e->SetTextFont(labelTextFont);
    label3e->SetTextSize(labelTextSize);
    label3e->SetTextAngle(labelTextAngle);
    label3e->Draw();
    TLatex *label4e = new TLatex(0.500, 1.5e5, "511");
    label4e->SetTextFont(labelTextFont);
    label4e->SetTextSize(labelTextSize);
    label4e->SetTextAngle(labelTextAngle);
    label4e->Draw();

    TLatex *label1f = new TLatex(5.653, 3e3, "R #rightarrow 0");
    label1f->SetTextFont(labelTextFont);
    label1f->SetTextSize(labelTextSize);
    label1f->SetTextAngle(labelTextAngle);
    label1f->Draw();

    TLatex *label1g = new TLatex(4.645, 6.2e3, "R #rightarrow 1042");
    label1g->SetTextFont(labelTextFont);
    label1g->SetTextSize(labelTextSize);
    label1g->SetTextAngle(labelTextAngle);
    label1g->Draw();

    TLatex *label1h = new TLatex(3.542, 4e3, "DE");
    label1h->SetTextFont(labelTextFont);
    label1h->SetTextSize(labelTextSize);
    label1h->SetTextAngle(labelTextAngle);
    label1h->Draw();
    TLatex *label2h = new TLatex(3.592, 1.5e3, "DE");
    label2h->SetTextFont(labelTextFont);
    label2h->SetTextSize(labelTextSize);
    label2h->SetTextAngle(labelTextAngle);
    label2h->Draw();

    TLatex *label1i = new TLatex(3.115, 2.0e4, "3133 #rightarrow 0");
    label1i->SetTextFont(labelTextFont);
    label1i->SetTextSize(labelTextSize);
    label1i->SetTextAngle(labelTextAngle);
    label1i->Draw();
    TLatex *label1l = new TLatex(2.075, 9e4, "3133 #rightarrow 1041"); // 2.091 gamma
    label1l->SetTextFont(labelTextFont);
    label1l->SetTextSize(labelTextSize);
    label1l->SetTextAngle(labelTextAngle);
    label1l->Draw();

    TLatex *label1m = new TLatex(2.010, 4.5e4, "3133 #rightarrow 1080"); // 2.052 gamma
    label1m->SetTextFont(labelTextFont);
    label1m->SetTextSize(labelTextSize);
    label1m->SetTextAngle(labelTextAngle);
    label1m->Draw();

    TLatex *label2m = new TLatex(1.390, 1.9e4, "3133 #rightarrow 1700"); // 2.052 gamma
    label2m->SetTextFont(labelTextFont);
    label2m->SetTextSize(labelTextSize);
    label2m->SetTextAngle(labelTextAngle);
    label2m->Draw();

    TLatex *label1n = new TLatex(2.140, 1.6e4, "3061 #rightarrow 937"); // 2.125 gamma
    label1n->SetTextFont(labelTextFont);
    label1n->SetTextSize(labelTextSize);
    label1n->SetTextAngle(labelTextAngle);
    label1n->Draw();

    TLatex *label1o = new TLatex(3.040, 3.5e3, "3061 #rightarrow 0"); // 3.061 gamma
    label1o->SetTextFont(labelTextFont);
    label1o->SetTextSize(labelTextSize);
    label1o->SetTextAngle(labelTextAngle);
    label1o->Draw();
    
    TLatex *label1p = new TLatex(5.142, 1.3e3, "SE"); // 3.061 gamma
    label1p->SetTextFont(labelTextFont);
    label1p->SetTextSize(labelTextSize);
    label1p->SetTextAngle(labelTextAngle);
    label1p->Draw();
    TLatex *label1q = new TLatex(4.110, 2e3, "SE"); // 3.061 gamma
    label1q->SetTextFont(labelTextFont);
    label1q->SetTextSize(labelTextSize);
    label1q->SetTextAngle(labelTextAngle);
    label1q->Draw();
    TLatex *label1r = new TLatex(4.050, 8e3, "SE"); // 3.061 gamma
    label1r->SetTextFont(labelTextFont);
    label1r->SetTextSize(labelTextSize);
    label1r->SetTextAngle(labelTextAngle);
    label1r->Draw();
    TLatex *label1s = new TLatex(3.940, 5e3, "R #rightarrow 1700"); // 3.061 gamma
    label1s->SetTextFont(labelTextFont);
    label1s->SetTextSize(labelTextSize);
    label1s->SetTextAngle(labelTextAngle);
    label1s->Draw();

    TString output_dir = "/felsdata-till08apr2026/n14ag_Felsenkeller_2025/Anup/Paper/";
    gSystem->mkdir(output_dir, true);
    TString output_file = output_dir + "spectra_1618_paper.pdf";
    TString output_root_file = output_dir + "spectra_1618_paper.root";
    canvas->SaveAs(output_file);
    canvas->SaveAs(output_root_file);
   

    std::cout << "Enter to continue" << std::endl;
    std::cin.ignore();
    
    // Clean up
    file1->Close();
}