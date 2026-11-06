#include "TFile.h"
#include "TH1.h"
#include "TCanvas.h"
#include "TLegend.h"
#include "TROOT.h"
#include "TStyle.h"
#include "TPad.h"

void multiplot_v3() {
    // Set overall style
    gStyle->SetOptStat(0);
    // gStyle->SetPadTickX(1);
    // gStyle->SetPadTickY(1);
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
    TH1 *hist1 = (TH1*)file1->Get("histoCal/histoCal[06]");
    TH1 *hist2 = (TH1*)file3->Get("histoCal/histoCal[06]");
    // TH1 *hist3 = (TH1*)file2->Get("histoCal/histoCal[06]");

    // Get the histograms
    // TH1 *hist1 = (TH1*)file1->Get("histoCal/histoEB18noDE");
    // TH1 *hist2 = (TH1*)file3->Get("histoCal/histoEB18noDE");
    // TH1 *hist3 = (TH1*)file2->Get("histoCal/histoEB18noDE");


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
    TCanvas *canvas = new TCanvas("canvas", "", 1000, 600);
    // canvas->Divide(1, 3, 0.001, 0.001); // Divide with small margins between pads
    // canvas->SetLeftMargin(0.00);
    // canvas->SetRightMargin(0.0);
    
    // Set common histogram styling
    const double titleSize = 0.03;
    const double labelSize = 0.03;
    const double titleOffset = 0.9;
    
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

    // double x_peak = 3.930;   // X of the peak
    // double y_peak = 5000;    // Y where arrow should point

    // // Vertical arrow pointing down to the spectrum
    // TArrow *arrowV = new TArrow(x_peak, y_peak + 3000, x_peak, y_peak, 0.005, "|>");
    // arrowV->SetLineColor(kBlack);
    // // arrowV->SetType(2);
    // arrowV->SetLineWidth(1);
    // arrowV->Draw();

    // // Horizontal connector line
    // double x_text = x_peak + 0.4;  // shift right for label
    // TLine *bar = new TLine(x_peak, y_peak + 600, x_text, y_peak + 600);
    // bar->SetLineColor(kBlack);
    // bar->SetLineWidth(1);
    // bar->Draw();

    // // Text above the bar
    // TLatex *label = new TLatex(x_text + 0.1, y_peak + 650, "R #rightarrow 1700");
    // label->SetTextFont(labelTextFont);
    // label->SetTextSize(labelTextSize);
    // label->Draw();


    // // Add grid
    // gPad->SetGridx();
    // gPad->SetGridy();
    
    // Pad 2: Second spectrum
    // canvas->cd(2);
    // canvas->SetTopMargin(0.01);
    // canvas->SetRightMargin(0.01);
    // gPad->SetLogy();
    // gPad->SetLeftMargin(0.06);
    // gPad->SetRightMargin(0.03);
    // gPad->SetBottomMargin(0.1);
    // gPad->SetTopMargin(0.02);
    
    // hist2->SetLineColor(color1);
    // hist2->SetLineWidth(lineWidth);
    // hist2->GetYaxis()->SetTickLength(0.01); 
    // hist2->GetXaxis()->SetRangeUser(0.0, 6.0);
    // hist2->GetXaxis()->SetNdivisions(520, kTRUE); // Increase number of divisions (labels)
    // hist2->GetXaxis()->SetDecimals(kTRUE);        // Show decimal digits
    // // hist1->GetXaxis()->SetLabelFormat("%.1f");    // Show one decimal point
    // hist2->SetMaximum(4e4);
    // hist2->SetTitle("");
    // hist2->GetXaxis()->SetTitle("E_{#gamma} [MeV]");
    // hist2->GetYaxis()->SetTitle("Counts / 2 keV");
    // // set the title in center
    // hist2->GetXaxis()->CenterTitle(true);
    // hist2->GetYaxis()->CenterTitle(true);
    // // hist1->GetXaxis()->SetTitleSize(0);
    // hist2->GetYaxis()->SetTitleSize(titleSize);
    // // hist1->GetXaxis()->SetLabelSize(0);
    // hist2->GetYaxis()->SetLabelSize(labelSize);
    // hist2->GetXaxis()->SetTitleOffset(1.2);
    // hist2->GetYaxis()->SetTitleOffset(titleOffset);
    // hist2->SetTitleSize(titleSize, "t"); // Title size
    // hist2->SetTitleFont(62, "t"); // Bold title
    // hist2->Draw();
     
    // TLatex *label2a = new TLatex(1.440, 3.5e4, "^{40}K");
    // label2a->SetTextFont(42);
    // label2a->SetTextSize(labelTextSize);
    // label2a->SetTextAngle(-90);
    // label2a->Draw();

    // TLatex *label2b = new TLatex(2.600, 9.5e3, "^{208}Tl");
    // label2b->SetTextFont(42);
    // label2b->SetTextSize(labelTextSize);
    // label2b->SetTextAngle(-90);
    // label2b->Draw();

    // TLatex *label2c = new TLatex(2.760, 1.5e4, "R #rightarrow 2523");
    // label2c->SetTextFont(42);
    // label2c->SetTextSize(labelTextSize);
    // label2c->SetTextAngle(-90);
    // label2c->Draw();

    // TLatex *label2d = new TLatex(4.340, 1e3, "R #rightarrow 937");
    // label2d->SetTextFont(42);
    // label2d->SetTextSize(labelTextSize);
    // label2d->SetTextAngle(-90);
    // label2d->Draw();
    // TLatex *label2e = new TLatex(2.500, 1.7e4, "2523 #rightarrow 0");
    // label2e->SetTextFont(42);
    // label2e->SetTextSize(labelTextSize);
    // label2e->SetTextAngle(-90);
    // label2e->Draw();
   

    // TLatex *label2g = new TLatex(4.155, 1.5e3, "R #rightarrow 1121");
    // label2g->SetTextFont(42);
    // label2g->SetTextSize(labelTextSize);
    // label2g->SetTextAngle(-90);
    // label2g->Draw();
    // TLatex *label2g1 = new TLatex(3.825, 2.7e2, "SE");
    // label2g1->SetTextFont(42);
    // label2g1->SetTextSize(labelTextSize);
    // label2g1->SetTextAngle(-90);
    // label2g1->Draw();
    // TLatex *label2h = new TLatex(3.640, 2.8e2, "SE");
    // label2h->SetTextFont(42);
    // label2h->SetTextSize(labelTextSize);
    // label2h->SetTextAngle(-90);
    // label2h->Draw();

    // TLatex *label2i = new TLatex(1.920, 4e3, "R #rightarrow 3358");
    // label2i->SetTextFont(42);
    // label2i->SetTextSize(labelTextSize);
    // label2i->SetTextAngle(-90);
    // label2i->Draw();

   

    // TLatex *label2k = new TLatex(1.570, 1.8e4, "2523 #rightarrow 937");
    // label2k->SetTextFont(42);
    // label2k->SetTextSize(labelTextSize);
    // label2k->SetTextAngle(-90);
    // label2k->Draw();

    // TLatex *label2l = new TLatex(0.920, 2.5e4, "937 #rightarrow 0");
    // label2l->SetTextFont(42);
    // label2l->SetTextSize(labelTextSize);
    // label2l->SetTextAngle(-90);
    // label2l->Draw();

    
    // TLatex *label2n = new TLatex(3.345, 4.5e2, "3358 #rightarrow 0");
    // label2n->SetTextFont(42);
    // label2n->SetTextSize(labelTextSize);
    // label2n->SetTextAngle(-90);
    // label2n->Draw();

    // TLatex *label2o = new TLatex(1.640, 4e3, "3358 #rightarrow 1700");
    // label2o->SetTextFont(42);
    // label2o->SetTextSize(labelTextSize);
    // label2o->SetTextAngle(-90);
    // label2o->Draw();

    // TLatex *label2p = new TLatex(3.131, 1.5e2, "DE");
    // label2p->SetTextFont(42);
    // label2p->SetTextSize(labelTextSize);
    // label2p->SetTextAngle(-90);
    // label2p->Draw();

    // TLatex *label2q = new TLatex(3.515, 5.5e2, "4652 #rightarrow 1121");
    // label2q->SetTextFont(42);
    // label2q->SetTextSize(labelTextSize);
    // label2q->SetTextAngle(-90);
    // label2q->Draw();

    // TLatex *label2r = new TLatex(2.420, 2.5e3, "3358 #rightarrow 937");
    // label2r->SetTextFont(42);
    // label2r->SetTextSize(labelTextSize);
    // label2r->SetTextAngle(-90);
    // label2r->Draw();

    // TLatex *label2s = new TLatex(3.290, 1.4e2, "DE");
    // label2s->SetTextFont(42);
    // label2s->SetTextSize(labelTextSize);
    // label2s->SetTextAngle(-90);
    // label2s->Draw();

    // TLatex *label2t = new TLatex(2.245, 1.4e3, "SE");
    // label2t->SetTextFont(42);
    // label2t->SetTextSize(labelTextSize);
    // label2t->SetTextAngle(-90);
    // label2t->Draw();

    // TLatex *label2u = new TLatex(1.990, 1.2e3, "SE");
    // label2u->SetTextFont(42);
    // label2u->SetTextSize(labelTextSize);
    // label2u->SetTextAngle(-90);
    // label2u->Draw();

    // TLatex *label2f = new TLatex(0.490, 3.1e4, "511");
    // label2f->SetTextFont(42);
    // label2f->SetTextSize(labelTextSize);
    // label2f->SetTextAngle(-90);
    // label2f->Draw();
    // TLatex *label2j = new TLatex(0.922, 4e4, "937 #rightarrow 0");
    // label2j->SetTextFont(42);
    // label2j->SetTextSize(labelTextSize);
    // label2j->SetTextAngle(-90);
    // label2j->Draw();

    // TLatex *label2m = new TLatex(0.822, 4e4, "2523 #rightarrow 1700");
    // label2m->SetTextFont(42);
    // label2m->SetTextSize(labelTextSize);
    // label2m->SetTextAngle(-90);
    // label2m->Draw();







    // // Add grid
    // gPad->SetGridx();
    // gPad->SetGridy();
    
    // // // Pad 3: Third spectrum
    // canvas->SetTopMargin(0.01);
    // canvas->SetRightMargin(0.01);
    // gPad->SetLogy();
    // gPad->SetLeftMargin(0.06);
    // gPad->SetRightMargin(0.03);
    // gPad->SetBottomMargin(0.1);
    // gPad->SetTopMargin(0.02);
    
    // hist3->SetLineColor(color1);
    // hist3->SetLineWidth(lineWidth);
    // hist3->GetYaxis()->SetTickLength(0.01); 
    // hist3->GetXaxis()->SetRangeUser(0.0, 6.0);
    // hist3->GetXaxis()->SetNdivisions(520, kTRUE); // Increase number of divisions (labels)
    // hist3->GetXaxis()->SetDecimals(kTRUE);        // Show decimal digits
    //          // Show one decimal point
    // hist3->SetMaximum(4e4);
    // hist3->SetTitle("");
    // hist3->GetXaxis()->SetTitle("E_{#gamma} [MeV]");
    // hist3->GetYaxis()->SetTitle("Counts / 2 keV");
    // hist3->GetXaxis()->CenterTitle(true);
    // hist3->GetYaxis()->CenterTitle(true);
    // hist3->GetYaxis()->SetTitleSize(titleSize);
    // hist3->GetYaxis()->SetLabelSize(labelSize);
    // hist3->GetXaxis()->SetTitleOffset(1.2);
    // hist3->GetYaxis()->SetTitleOffset(titleOffset);
    // hist3->SetTitleSize(titleSize, "t"); // Title size
    // hist3->SetTitleFont(62, "t"); // Bold title
    // hist3->Draw();
    // //legend->Draw();
    // TLatex *label3a = new TLatex(1.440, 3.8e4, "^{40}K");
    // label3a->SetTextFont(42);
    // label3a->SetTextSize(labelTextSize);
    // label3a->SetTextAngle(-90);
    // label3a->Draw();

    // TLatex *label3b = new TLatex(2.600, 1.3e4, "^{208}Tl");
    // label3b->SetTextFont(42);
    // label3b->SetTextSize(labelTextSize);
    // label3b->SetTextAngle(-90);
    // label3b->Draw();
    // TLatex *label3c = new TLatex(1.798, 2e5, "R #rightarrow 3161");
    // label3c->SetTextFont(42);
    // label3c->SetTextSize(0.05);
    // label3c->SetTextAngle(-90);
    // label3c->Draw();
    // TLatex *label3d = new TLatex(3.805, 30, "R #rightarrow 1041");
    // label3d->SetTextFont(42);
    // label3d->SetTextSize(labelTextSize);
    // label3d->SetTextAngle(-90);
    // label3d->Draw();

    // TLatex *label3e = new TLatex(1.041, 5e5, "1041 #rightarrow 0");
    // label3e->SetTextFont(42);
    // label3e->SetTextSize(0.05);
    // label3e->SetTextAngle(-90);
    // label3e->Draw();

    // TLatex *label3f = new TLatex(3.779, 2e2, "R #rightarrow 1080");
    // label3f->SetTextFont(42);
    // label3f->SetTextSize(0.05);
    // label3f->SetTextAngle(-90);
    // label3f->Draw();


    // // Add grid
    // gPad->SetGridx();
    // gPad->SetGridy();

    // canvas->cd(1);
    // TLegend *leg1 = new TLegend(0.80, 0.8, 0.95, 0.93);
    // leg1->SetTextSize(0.08);
    // leg1->AddEntry(hist1, "E_{#alpha} = 1618 keV", "l");
    // leg1->Draw();

    // canvas->cd(2); 
    // TLegend *leg2 = new TLegend(0.80, 0.8, 0.95, 0.93);
    // leg2->SetTextSize(0.08);
    // leg2->AddEntry(hist2, "E_{#alpha} = 1136 keV", "l");
    // leg2->Draw();

    // canvas->cd(3);
    // TLegend *leg3 = new TLegend(0.80, 0.8, 0.95, 0.93);
    // leg3->SetTextSize(0.08); 
    // leg3->AddEntry(hist3, "E_{#alpha} = 573 keV", "l");
    // leg3->Draw();
    
    // // // Add a common title
    // // canvas->cd();
    // // TLatex *title = new TLatex(0.5, 0.98, "Gamma-ray Spectra from Different Resonances");
    // // title->SetTextAlign(22);
    // // title->SetTextSize(0.03);
    // // title->SetNDC();
    // // title->Draw();
    TString output_dir = "/felsdata/n14ag_Felsenkeller_2025/Anup/Paper/";
    gSystem->mkdir(output_dir, true);
    // TString output_file = output_dir + "scan278keV_EB18_G_p1_v3.pdf";
    TString output_root_file = output_dir + "spectra1618_EB18G.root";
    // c->SaveAs(output_file);
    canvas->SaveAs(output_root_file);
    // Save the canvas
    // canvas->SaveAs("/felsdata/n14ag_Felsenkeller_2025/Anup/Paper/spectra573_G_v2.pdf");
    
    // canvas->SaveAs("/felsdata/n14ag_Felsenkeller_2025/Anup/Paper/multiplot_EB18noDE_H25.jpg", "jpg,dpi=600");
   

    std::cout << "Enter to continue" << std::endl;
    std::cin.ignore();
    
    // Clean up
    file1->Close();
    file2->Close();
    file3->Close();
}