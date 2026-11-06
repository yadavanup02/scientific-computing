// #include "TFile.h"
// #include "TCanvas.h"
// #include "TGraphErrors.h"
// #include "TLegend.h"

// #include "TLatex.h"
// #include "TStyle.h"

// void plot_n14pg_scans() {
//     // Input files and resonance info
//     TString fileName[2] = {"Resonance_278keV/scan278keV_EB18_G_p1_v2.root", "Resonance_1058keV/scan1058keV_EB18_G_p1_v2.root"};
//     TString plotName[2] = {"#it{E}_{#alpha} = 278 keV", "#it{E}_{#alpha} = 1058 keV"};
//     double resoEn[2] = {278., 1058.};
//     TString cName[2] = {"c", "c"}; 
    
//     // Modern color palette
//     Color_t markerColor[2] = {kBlack, kBlack}; 
//     Color_t fitColor[2] = {kMagenta+2, kMagenta+2}; 

//     // Color_t markerColor[2] = {kBlack, kBlack}; 
//     // Color_t fitColor[2] = {kMagenta+2, kMagenta+2}; 

//     // Create main canvas
//     // TCanvas *c1 = new TCanvas("c1", "Resonance Scans", 1000, 400);
//     // c1->Divide(2,1, 0.01, 0.01); // Small margins between pads

//     TCanvas *c1 = new TCanvas("c1", "Resonance Scans", 400, 600);
//     c1->Divide(1,2, 0.01, 0.01);
//     // c1->SetTitle("Resonance Scans for #it{^{14}N(#alpha,#gamma)^{18}F}");
    
//     // Configure global style
//     // gStyle->SetOptTitle(1);
//     gStyle->SetLabelSize(0.05, "XY");
//     gStyle->SetTitleSize(0.055, "XY");
//     gStyle->SetTitleOffset(1.1, "X");
//     gStyle->SetTitleOffset(1.3, "Y");
    
//     for (int i=0; i<2; i++) {
//         // Open file and get canvas
//         TFile *file = TFile::Open(fileName[i]);
//         if (!file || file->IsZombie()) {
//             Error("plot_two_resonances", "Couldn't open %s", fileName[i].Data());
//             continue;
//         }
        
//         TCanvas *inputCanvas = (TCanvas*)file->Get(cName[i]);
//         if (!inputCanvas) {
//             Error("plot_two_resonances", "Canvas %s not found in %s", 
//                   cName[i].Data(), fileName[i].Data());
//             continue;
//         }
        
//         // Get graph and fit
//         TGraphErrors *graph = (TGraphErrors*)inputCanvas->FindObject("Graph");
//         TF1 *fit = (TF1*)graph->GetListOfFunctions()->FindObject("totalFit");
        
//         // Configure pad
//         c1->cd(i+1);
//         gPad->SetLeftMargin(0.1);
//         gPad->SetRightMargin(0.01);
//         gPad->SetTopMargin(0.009);
//         gPad->SetBottomMargin(0.075);
//         // gPad->SetLogy();
//         // gPad->SetGridx();
//         // gPad->SetGridy();
        
//         // Style the graph
//         graph->SetMarkerStyle(20); // Different markers
//         graph->SetMarkerSize(0.8);
//         graph->SetMarkerColor(markerColor[i]);
//         graph->SetLineColor(markerColor[i]);
        
//         // Set axis ranges
//         // graph->GetXaxis()->SetRangeUser(resoEn[i]-40., resoEn[i]+80.);
//         // graph->GetXaxis()->SetLimits(resoEn[i]-10., resoEn[i]+15.);
//         // graph->GetYaxis()->SetRangeUser(1.e-5, 0.3);
        
//         // Draw graph
//         graph->SetTitle(";;Yield [#muC^{-1}]");
//         graph->GetXaxis()->SetTitle("E_{p} [keV]");
//         if (i == 0) graph->GetYaxis()->SetTitle("Yield [#muC^{-1}]");
//         graph->Draw("AP");
        
//         // Style and draw fit
//         fit->SetLineColor(fitColor[i]);
//         fit->SetLineWidth(2);
//         fit->Draw("same");
        
//         // // Add resonance info
//         // TLatex *info = new TLatex(resoEn[i]+30, 0.15, plotName[i]);
//         // info->SetTextColor(markerColor[i]);
//         // info->SetTextSize(0.06);
//         // info->SetTextAlign(22);
//         // info->Draw();
        
//         // Add fit parameters
//         // TLatex *fitParams = new TLatex(resoEn[i]-15, 1.e-4, 
//         //     Form("E_{res} = %.1f keV\nFWHM = %.1f keV", 
//         //         fit->GetParameter(4), fit->GetParameter(2)));
//         // fitParams->SetTextColor(fitColor[i]);
//         // fitParams->SetTextSize(0.045);
//         // fitParams->Draw();

//         // resonance energy line with the arrorow signifying the resonance energy
//         if (i == 0) {
//             graph->GetXaxis()->SetLimits(resoEn[i]-10., resoEn[i]+15.);
//             graph->GetHistogram()->SetMinimum(0.0);
//             graph->GetHistogram()->SetMaximum(0.41);
//             graph->GetHistogram()->SetTitleOffset(1.2, "Y");
//             // Draw a vertical line with an arrow at the top
//             TArrow *arrowLine = new TArrow(resoEn[i], 0.0, resoEn[i], 0.05, 0.02, "|>");
//             arrowLine->SetLineColor(kGray+2);
//             arrowLine->SetLineWidth(1);
//             arrowLine->SetAngle(30); // Arrowhead opening angle
//             arrowLine->SetFillColor(kGray+2);
//             arrowLine->Draw();

//             // legend
//             TLegend *legend = new TLegend(0.8, 0.87, 0.98, 0.97);
//             legend->SetBorderSize(1);
//             legend->SetFillStyle(0); // Transparent background
//             legend->SetTextSize(0.04);
//             legend->SetTextFont(42); // Use a standard font
//             legend->AddEntry(graph, "278 keV", "p");
//             legend->AddEntry(fit, "Fit", "l");
//             legend->Draw();

//             TLatex *label = new TLatex(0.18, 0.9, "^{14}N(p, #gamma)^{15}O");
//             label->SetNDC();
//             label->SetTextSize(0.05);
//             label->SetTextFont(42);
//             label->Draw();
//         }
//         else if (i == 1) {
//             graph->GetXaxis()->SetLimits(resoEn[i]-12., resoEn[i]+20.);
//             graph->GetHistogram()->SetMinimum(0.0);
//             graph->GetHistogram()->SetMaximum(1.05);
//             graph->GetHistogram()->SetTitleOffset(1.0, "Y");
//             gPad->SetTopMargin(0.05);

//             // Draw a vertical line with an arrow at the top
//             TArrow *arrowLine = new TArrow(resoEn[i], 0.0, resoEn[i], 0.15, 0.02, "|>");
//             arrowLine->SetLineColor(kGray+2);
//             arrowLine->SetLineWidth(1);
//             arrowLine->SetAngle(30); // Arrowhead opening angle
//             arrowLine->SetFillColor(kGray+2);
//             arrowLine->Draw();

//             // legend
//             TLegend *legend = new TLegend(0.8, 0.87, 0.98, 0.97);
//             gPad->SetTopMargin(0.0);
//             legend->SetBorderSize(1);
//             legend->SetFillStyle(0); // Transparent background
//             legend->SetTextSize(0.04);
//             legend->SetTextFont(42); // Use a standard font
//             legend->AddEntry(graph, "1058 keV", "p");
//             legend->AddEntry(fit, "Fit", "l");
//             legend->Draw();
//             TLatex *label = new TLatex(0.18, 0.9, "^{14}N(p, #gamma)^{15}O");
//             label->SetNDC();
//             label->SetTextSize(0.05);
//             label->SetTextFont(42);
//             label->Draw();
//         }
//         // set legend
       
//         // TLine *resLine = new TLine(resoEn[i], 0.1, resoEn[i], 0.5);
//         // resLine->SetLineColor(kBlue);
//         // resLine->SetLineStyle(1); // Solid line
//         // resLine->SetLineWidth(1);
//         // resLine->Draw("same");
        
//         file->Close();
//     }
    
//     // // Add common title
//     // c1->cd();
//     // // Create an overlay pad for the title
//     // TPad *titlePad = new TPad("titlePad", "titlePad", 0, 0.95, 1, 1);
//     // titlePad->SetFillStyle(4000); // Transparent
//     // titlePad->SetFrameFillStyle(0);
//     // titlePad->Draw();
//     // titlePad->cd();

//     // TLatex *title = new TLatex(0.5, 0.5, "^{14}N(p,#gamma)^{15}O");
//     // title->SetNDC();
//     // title->SetTextAlign(22);
//     // title->SetTextSize(0.45); // Relative to pad height
//     // title->SetTextFont(42); // Use a standard font
//     // title->SetTextColor(kBlack);
//     // title->Draw();

//     c1->cd(); // Return to main canvas
//     // Save output
//     c1->SaveAs("Resonance_278keV/plot_n14pg_scans_v2.pdf");
//     c1->SaveAs("Resonance_278keV/plot_n14pg_scans_v2.png");
// }
#include "TFile.h"
#include "TCanvas.h"
#include "TGraphErrors.h"
#include "TLegend.h"

#include "TLatex.h"
#include "TStyle.h"

void plot_n14pg_scans() {
    // Input files and resonance info
    TString fileName[3] = {"Resonance_278keV/scan278keV_EB18_G_p1_v2.root", "Resonance_430keV/scan430keV_EB18_G_p1_v2.root", "Resonance_1058keV/scan1058keV_EB18_G_p1_v2.root"};
    TString plotName[3] = {"#it{E}_{#alpha} = 278 keV", "#it{E}_{#alpha} = 430 keV", "#it{E}_{#alpha} = 1058 keV"};
    double resoEn[3] = {278., 430., 1058.};
    TString cName[3] = {"c", "c", "c"};

    // Modern color palette
    Color_t markerColor[3] = {kBlack, kBlack, kBlack}; 
    Color_t fitColor[3] = {kMagenta+2, kMagenta+2, kMagenta+2}; 

    // Color_t markerColor[2] = {kBlack, kBlack}; 
    // Color_t fitColor[2] = {kMagenta+2, kMagenta+2}; 

    // Create main canvas
    // TCanvas *c1 = new TCanvas("c1", "Resonance Scans", 1000, 400);
    // c1->Divide(2,1, 0.01, 0.01); // Small margins between pads

    TCanvas *c1 = new TCanvas("c1", "Resonance Scans", 400, 900);
    c1->Divide(1,3, 0.01, 0.01);
    // c1->SetTitle("Resonance Scans for #it{^{14}N(#alpha,#gamma)^{18}F}");
    
    // Configure global style
    // gStyle->SetOptTitle(1);
    gStyle->SetLabelSize(0.075, "XY");
    gStyle->SetTitleSize(0.08, "XY");
    gStyle->SetTitleOffset(1.1, "X");
    gStyle->SetTitleOffset(1.3, "Y");
    
    for (int i=0; i<3; i++) {
        // Open file and get canvas
        TFile *file = TFile::Open(fileName[i]);
        if (!file || file->IsZombie()) {
            Error("plot_two_resonances", "Couldn't open %s", fileName[i].Data());
            continue;
        }
        
        TCanvas *inputCanvas = (TCanvas*)file->Get(cName[i]);
        if (!inputCanvas) {
            Error("plot_two_resonances", "Canvas %s not found in %s", 
                  cName[i].Data(), fileName[i].Data());
            continue;
        }
        
        // Get graph and fit
        // TGraphErrors *graph = (TGraphErrors*)inputCanvas->FindObject("Graph");
        // if (!graph) {
        //     Error("plot_two_resonances", "Graph 'Graph' not found in canvas");
        //     file->Close();
        //     continue;
        // }
        TGraphErrors *graph = nullptr;
        TIter next(inputCanvas->GetListOfPrimitives());
        TObject *obj;
        while ((obj = next())) {
            if (obj->InheritsFrom("TGraphErrors")) {
                graph = (TGraphErrors*)obj;
                std::cout << "Found graph: " << graph->GetName() << std::endl;
                break;
            }
        }
        
        if (!graph) {
            Error("plot_n14pg_scans", "No graph found in %s", fileName[i].Data());
            file->Close();
            continue;
        }

        TF1 *fit = (TF1*)graph->GetListOfFunctions()->FindObject("totalFit");
        
        // Configure pad
        c1->cd(i+1);
        gPad->SetLeftMargin(0.1);
        gPad->SetRightMargin(0.013);
        gPad->SetTopMargin(0.07);
        gPad->SetBottomMargin(0.075);
        // gPad->SetLogy();
        // gPad->SetGridx();
        // gPad->SetGridy();
        
        // Style the graph
        graph->SetMarkerStyle(20); // Different markers
        graph->SetMarkerSize(0.8);
        graph->SetMarkerColor(markerColor[i]);
        graph->SetLineColor(markerColor[i]);
        
        // Set axis ranges
        // graph->GetXaxis()->SetRangeUser(resoEn[i]-40., resoEn[i]+80.);
        // graph->GetXaxis()->SetLimits(resoEn[i]-10., resoEn[i]+15.);
        // graph->GetYaxis()->SetRangeUser(1.e-5, 0.3);
        
        // Draw graph
        graph->SetTitle(";;Yield [#muC^{-1}]");
        graph->GetXaxis()->SetTitle("E_{p} [keV]");
        graph->GetYaxis()->SetTitle("Yield [#muC^{-1}]");
        // if (i == 0) graph->GetYaxis()->SetTitle("Yield [ #muC^{-1}]");
        graph->Draw("AP");
        
        // Style and draw fit
        fit->SetLineColor(fitColor[i]);
        fit->SetLineWidth(2);
        fit->Draw("same");
        
        // // Add resonance info
        // TLatex *info = new TLatex(resoEn[i]+30, 0.15, plotName[i]);
        // info->SetTextColor(markerColor[i]);
        // info->SetTextSize(0.06);
        // info->SetTextAlign(22);
        // info->Draw();
        
        // Add fit parameters
        // TLatex *fitParams = new TLatex(resoEn[i]-15, 1.e-4, 
        //     Form("E_{res} = %.1f keV\nFWHM = %.1f keV", 
        //         fit->GetParameter(4), fit->GetParameter(2)));
        // fitParams->SetTextColor(fitColor[i]);
        // fitParams->SetTextSize(0.045);
        // fitParams->Draw();

        // resonance energy line with the arrorow signifying the resonance energy
        if (i == 0) {
            graph->GetXaxis()->SetLimits(resoEn[i]-10., resoEn[i]+15.);
            graph->GetHistogram()->SetMinimum(0.0);
            graph->GetHistogram()->SetMaximum(0.41);
            graph->GetHistogram()->SetTitleOffset(1.2, "Y");
            gPad->SetTopMargin(0.005);

            TLatex *label = new TLatex(0.18, 0.9, "^{14}N(p, #gamma)^{15}O");
            label->SetNDC();
            label->SetTextSize(0.05);
            label->SetTextFont(42);
            label->Draw();

            // Draw a vertical line with an arrow at the top
            TArrow *arrowLine = new TArrow(resoEn[i], 0.0, resoEn[i], 0.05, 0.015, "|>");
            arrowLine->SetLineColor(kGray+2);
            arrowLine->SetLineWidth(1);
            arrowLine->SetAngle(30); // Arrowhead opening angle
            arrowLine->SetFillColor(kGray+2);
            arrowLine->Draw();

            // legend
            TLegend *legend = new TLegend(0.8, 0.87, 0.98, 0.97);
            legend->SetBorderSize(1);
            legend->SetFillStyle(0); // Transparent background
            legend->SetTextSize(0.04);
            legend->SetTextFont(42); // Use a standard font
            legend->AddEntry(graph, "278 keV", "p");
            legend->AddEntry(fit, "Fit", "l");
            legend->Draw();
        }  
        else if (i == 1) {
            graph->GetXaxis()->SetLimits(resoEn[i]-10., resoEn[i]+15.);
            graph->GetHistogram()->SetMinimum(0.0);
            graph->GetHistogram()->SetMaximum(2.0);
            graph->GetHistogram()->SetTitleOffset(1.2, "Y");

            gPad->SetTopMargin(0.0);
            TLatex *label = new TLatex(0.18, 0.9, "^{15}N(p, #alpha#gamma)^{12}C");
            label->SetNDC();
            label->SetTextSize(0.05);
            label->SetTextFont(42);
            label->Draw();
            // Draw a vertical line with an arrow at the top
            TArrow *arrowLine = new TArrow(resoEn[i], 0.0, resoEn[i], 0.2, 0.015, "|>");
            arrowLine->SetLineColor(kGray+2);
            arrowLine->SetLineWidth(1);
            arrowLine->SetAngle(30); // Arrowhead opening angle
            arrowLine->SetFillColor(kGray+2);
            arrowLine->Draw();

            // legend
            TLegend *legend = new TLegend(0.8, 0.87, 0.98, 0.97);
            gPad->SetTopMargin(0.01);
            legend->SetBorderSize(1);
            legend->SetFillStyle(0); // Transparent background
            legend->SetTextSize(0.04);
            legend->SetTextFont(42); // Use a standard font
            legend->AddEntry(graph, "430 keV", "p");
            legend->AddEntry(fit, "Fit", "l");
            legend->Draw();
        }
         else if (i == 2) {
            graph->GetXaxis()->SetLimits(resoEn[i]-12., resoEn[i]+15.);
            graph->GetHistogram()->SetMinimum(0.0);
            graph->GetHistogram()->SetMaximum(1.05);
            graph->GetHistogram()->SetTitleOffset(1.2, "Y");
            gPad->SetTopMargin(0.0);
            TLatex *label = new TLatex(0.18, 0.9, "^{14}N(p, #gamma)^{15}O");
            label->SetNDC();
            label->SetTextSize(0.05);
            label->SetTextFont(42);
            label->Draw();
            // Draw a vertical line with an arrow at the top
            TArrow *arrowLine = new TArrow(resoEn[i], 0.0, resoEn[i], 0.15, 0.015, "|>");
            arrowLine->SetLineColor(kGray+2);
            arrowLine->SetLineWidth(1);
            arrowLine->SetAngle(30); // Arrowhead opening angle
            arrowLine->SetFillColor(kGray+2);
            arrowLine->Draw();

            // legend
            TLegend *legend = new TLegend(0.8, 0.87, 0.98, 0.97);
            gPad->SetTopMargin(0.01);
            legend->SetBorderSize(1);
            legend->SetFillStyle(0); // Transparent background
            legend->SetTextSize(0.04);
            legend->SetTextFont(42); // Use a standard font
            legend->AddEntry(graph, "1058 keV", "p");
            legend->AddEntry(fit, "Fit", "l");
            legend->Draw();
        }


        // set legend
       
        // TLine *resLine = new TLine(resoEn[i], 0.1, resoEn[i], 0.5);
        // resLine->SetLineColor(kBlue);
        // resLine->SetLineStyle(1); // Solid line
        // resLine->SetLineWidth(1);
        // resLine->Draw("same");
        
        file->Close();
    }
    
    // // Add common title
    // c1->cd();
    // // Create an overlay pad for the title
    // TPad *titlePad = new TPad("titlePad", "titlePad", 0, 0.95, 1, 1);
    // titlePad->SetFillStyle(4000); // Transparent
    // titlePad->SetFrameFillStyle(0);
    // titlePad->Draw();
    // titlePad->cd();

    // TLatex *title = new TLatex(0.5, 0.5, "^{14}N(p,#gamma)^{15}O");
    // title->SetNDC();
    // title->SetTextAlign(22);
    // title->SetTextSize(0.45); // Relative to pad height
    // title->SetTextFont(42); // Use a standard font
    // title->SetTextColor(kBlack);
    // title->Draw();

    c1->cd(); // Return to main canvas
    // Save output
    c1->SaveAs("Resonance_278keV/plot_n14pg_all_v2.pdf");
    c1->SaveAs("Resonance_278keV/plot_n14pg_all_v2.png");
}
