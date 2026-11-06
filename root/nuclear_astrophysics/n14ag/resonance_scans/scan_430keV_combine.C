#include "TLine.h"
#include "TMarker.h"
#include "TGraph.h"
#include "TGraphErrors.h"
#include "TRandom.h"
#include "TStyle.h"
#include "TCanvas.h"
#include "TF1.h"
#include "TArrow.h"
#include "TLatex.h"
#include "TString.h"
#include "stdio.h"
#include "iostream"
#include "fstream"
#include <vector>
#include <sstream>

void scan_430keV_combine() {
    TObjArray objectsToKeep;

    // --- First Resonance (430 keV) ---
    gROOT->ProcessLine(".L scan_430keV.C");
    gROOT->ProcessLine("scan_430keV(4627, 4644)");
    TGraphErrors *gr1 = (TGraphErrors*)gROOT->FindObject("gr_430keV");
    if (!gr1) {
        std::cerr << "Error: Graph gr1 not found. Please check the macro execution." << std::endl;
        return;
    }
    gr1 = (TGraphErrors*)gr1->Clone("gr1_clone");
    objectsToKeep.Add(gr1);
    

    // --- Second Resonance (430 keV) ---
    gROOT->ProcessLine(".L scan_430keV_m.C");
    gROOT->ProcessLine("scan_430keV_m(4671, 4693)");
    TGraphErrors *gr2 = (TGraphErrors*)gROOT->FindObject("gr_430keV_m");
    if (!gr2) {
        std::cerr << "Error: Graph gr2 not found. Please check the macro execution." << std::endl;
        return;
    }
    gr2 = (TGraphErrors*)gr2->Clone("gr2_clone");
    objectsToKeep.Add(gr2);
    

 // --- Create Canvas ---
    TCanvas *c1 = new TCanvas("c", "Combined Resonance Scans", 800, 600);
    objectsToKeep.Add(c1);
    // c1->SetGrid();
    gStyle->SetOptStat(0);
    gPad->SetRightMargin(0.013);
    gPad->SetTopMargin(0.01);
    gPad->SetLeftMargin(0.1);
    gPad->SetBottomMargin(0.1);

    // --- Style Customization ---
    // First resonance
    gr1->SetMarkerStyle(kOpenSquare); // Open square
    gr1->SetMarkerColor(kBlue);
    gr1->SetLineColor(kBlue);
    // gr1->SetTitle("Combined Resonance Scans"); // Main title
    
    // Second resonance
    gr2->SetMarkerStyle(20); // Open square
    gr2->SetMarkerColor(kBlack);
    gr2->SetLineColor(kBlack);
    
    // --- Axis Settings ---
    // Find combined range
    double xmin = min(gr1->GetXaxis()->GetXmin(), gr2->GetXaxis()->GetXmin());
    double xmax = max(gr1->GetXaxis()->GetXmax(), gr2->GetXaxis()->GetXmax());
    double ymin = 0;
    double ymax = max(gr1->GetYaxis()->GetXmax(), gr2->GetYaxis()->GetXmax()) * 1.0;
    
    // Apply ranges
    // gr1->GetXaxis()->SetRangeUser(420, 450);
    gr1->GetXaxis()->SetLimits(420, 450);
    // gr1->GetXaxis()->SetRangeUser(gr2->GetXaxis()->GetXmin(), gr2->GetXaxis()->GetXmax());
    gr1->GetYaxis()->SetRangeUser(ymin, ymax);
    gr1->GetXaxis()->SetTitle("#it{E}_{#alpha} [keV]");
    gr1->GetYaxis()->SetTitle("Yield [uC^{-1}]");
    
    // --- Set fit colors ---
    TF1* fit1 = (TF1*)gr1->GetListOfFunctions()->FindObject("totalFit");
    if (fit1) {
        fit1->SetLineColor(kBlue); // or any color you want
        fit1->SetLineWidth(2);
    }
    double yMax1 = fit1->GetMaximum(426, 434);
    double Eres1 = fit1->GetX(0.50 * yMax1, 426, 434);
    double dEres1 = fit1->GetParError(4); // Error on resonance energy

    double Gamma1 = fit1->GetParameter(1); // gamma 1
    double dGamma1 = fit1->GetParError(1); // Error on width

   
    TF1* fit2 = (TF1*)gr2->GetListOfFunctions()->FindObject("totalFit");
    if (fit2) {
        fit2->SetLineColor(kPink+2); // or any color you want
        fit2->SetLineWidth(2);
    }

    double yMax2 = fit2->GetMaximum(425, 435);
    double Eres2 = fit2->GetX(0.50 * yMax2, 425, 435);
    double dEres2 = fit2->GetParError(4); // Error on resonance energy

    double Gamma2 = fit2->GetParameter(1); // gamma 2
    double dGamma2 = fit2->GetParError(1); // Error on width

    // --- Drawing ---
    gr1->Draw("AP");
    if (fit1) fit1->Draw("SAME");
    gr2->Draw("P SAME");
    if (fit2) fit2->Draw("SAME");

    TLatex *resLabel1 = new TLatex(0.75, 0.75, Form("E_{r} = %.3f #pm %.3f keV", Eres1, dEres1));
    resLabel1->SetNDC();
    resLabel1->SetTextFont(42); // Use a standard font
    resLabel1->SetTextColor(kBlue);    
    resLabel1->SetTextSize(0.030);
    resLabel1->Draw();
    TLatex *resLabel2 = new TLatex(0.75, 0.70, Form("#Gamma_{1} = %.3f #pm %.3f keV", Gamma1, dGamma1));
    resLabel2->SetNDC();
    resLabel2->SetTextFont(42); // Use a standard font
    resLabel2->SetTextColor(kBlue);
    resLabel2->SetTextSize(0.030);
    resLabel2->Draw();

    TLatex *resLabel3 = new TLatex(0.75, 0.65, Form("E_{r} = %.3f #pm %.3f keV", Eres2, dEres2));
    resLabel3->SetNDC();
    resLabel3->SetTextFont(42); // Use a standard font
    resLabel3->SetTextColor(kPink+2);    
    resLabel3->SetTextSize(0.030);
    resLabel3->Draw();
    TLatex *resLabel4 = new TLatex(0.75, 0.60, Form("#Gamma_{1} = %.3f #pm %.3f keV", Gamma2, dGamma2));
    resLabel4->SetNDC();
    resLabel4->SetTextFont(42); // Use a standard font
    resLabel4->SetTextColor(kPink+2);
    resLabel4->SetTextSize(0.030);
    resLabel4->Draw();  
    
    // --- Legend ---
    TLegend *leg = new TLegend(0.75, 0.80, 0.98, 0.98);
    leg->SetBorderSize(1);
    leg->SetTextSize(0.030);
    leg->SetTextFont(42); // Use a standard font
    leg->SetFillStyle(0); // Transparent background
    leg->AddEntry(gr1, "Atomic", "p");
    leg->AddEntry(gr1->GetListOfFunctions()->FindObject("totalFit"), "Atomic Fit", "l");
    leg->AddEntry(gr2, "Molecular", "p");
    leg->AddEntry(gr2->GetListOfFunctions()->FindObject("totalFit"), "Molecular Fit", "l");

    leg->Draw();
    c1->Update();
    // --- Save Output ---
    c1->SaveAs("/felsdata/n14ag_Felsenkeller_2025/Anup/Resonance_430keV/combined_resonances.pdf");
    std::cout << "Combined resonance scans saved as combined_resonances.pdf" << std::endl;

    // c1->SetEditable(kFALSE); 
    // gROOT->GetListOfCanvases()->Add(c1);

    std::cout << "Enter to continue..." << std::endl;
    std::cin.ignore();
    
}
// #include "TFile.h"
// #include "TCanvas.h"
// #include "TGraphErrors.h"
// #include "TLegend.h"

// #include "TLatex.h"
// #include "TStyle.h"

// void scan_430keV_combine() {
//     // Input files and resonance info
//     TString fileName[2] = {"Resonance_430keV/scan430keV_EB18_G_p1_v2.root", "Resonance_430keV_m/scan430keV_m_EB18_G_p1_v2.root"};
//     TString plotName[2] = {"430 keV, atomic", "430 keV, molecular"};
//     // double resoEn[2] = {430., 430.};
//     // TString cName[2] = {"c", "c"};
//     Color_t markerColor[2] = {kViolet, kBlack}; 
//     Color_t fitColor[2] = {kBlue, kRed}; 
//     TCanvas *c1 = new TCanvas("c1", "Resonance Scans", 600, 600);
//     gStyle->SetLabelSize(0.075, "XY");
//     gStyle->SetTitleSize(0.08, "XY");
//     gStyle->SetTitleOffset(1.1, "X");
//     gStyle->SetTitleOffset(1.3, "Y");

//     TLegend *legend = new TLegend(0.70, 0.87, 0.98, 0.97);
//     legend->SetBorderSize(1);
//     legend->SetFillStyle(0);
//     legend->SetTextSize(0.04);
//     legend->SetTextFont(42);

//     TGraphErrors *graphs[2];
//     TF1 *fits[2];

//     for (int i = 0; i < 2; i++) {
//         TFile *file = TFile::Open(fileName[i]);
//         if (!file || file->IsZombie()) continue;
//         TCanvas *inputCanvas = (TCanvas*)file->Get("c");
//         if (!inputCanvas) { file->Close(); continue; }

//         TGraphErrors *graph = nullptr;
//         TIter next(inputCanvas->GetListOfPrimitives());
//         TObject *obj;
//         while ((obj = next())) {
//             if (obj->InheritsFrom("TGraphErrors")) {
//                 graph = (TGraphErrors*)obj;
//                 break;
//             }
//         }
//         if (!graph) {
//             std::cerr << "Graph not found in file: " << fileName[i] << std::endl;
//             file->Close();
//             continue;
//         }

//         TF1 *fit = (TF1*)graph->GetListOfFunctions()->FindObject("totalFit");

//         // Style
//         graph->SetMarkerStyle(20 + i);
//         graph->SetMarkerSize(0.8);
//         graph->SetMarkerColor(markerColor[i]);
//         graph->SetLineColor(markerColor[i]);
//         if (fit) {
//             fit->SetLineColor(fitColor[i]);
//             fit->SetLineWidth(2);
//         }

//         // Draw
//         if (i == 0) {
//             graph->SetTitle(";;Yield [#muC^{-1}]");
//             graph->GetXaxis()->SetTitle("E_{p} [keV]");
//             graph->GetYaxis()->SetTitle("Yield [#muC^{-1}]");
//             graph->Draw("AP");
//             if (fit) fit->Draw("SAME");
//         } else {
//             graph->Draw("P SAME");
//             if (fit) fit->Draw("SAME");
//         }

//         legend->AddEntry(graph, plotName[i], "p");
//         if (fit) legend->AddEntry(fit, Form("Fit (%s)", plotName[i].Data()), "l");

//         // file->Close();
//     }

//     legend->Draw();
//     c1->SaveAs("Resonance_430keV/plot_430keV_combine_v1.pdf");
//     c1->Print("Resonance_430keV/plot_430keV_combine_v1.png", "pngdpi=600");
// }