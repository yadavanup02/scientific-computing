#include "TFile.h"
#include "TCanvas.h"
#include "TGraphErrors.h"
#include "TLegend.h"

#include "TLatex.h"
#include "TStyle.h"

void plot_n14ag_scans_thesis_v1() {
    // Input files and resonance info
    TString fileName[3] = {"Resonance_573keV/scan_All_thesis.root", "Resonance_1136keV/scan_EB18_p2_thesis.root", "Resonance_1618keV/scan_EB18_p2_thesis.root"};
    TString plotName[3] = {"#it{E}_{#alpha} = 573 keV", "#it{E}_{#alpha} = 1136 keV", "#it{E}_{#alpha} = 1618 keV"};
    double resoEn[3] = {573., 1136., 1618.};
    TString cName[3] = {"c", "c", "c"};

    // Modern color palette
    // Color_t markerColor[3] = {kBlack, kBlack, kBlack}; 
    // Color_t fitColor[3] = {kMagenta+2, kMagenta+2, kMagenta+2}; 

    Color_t markerColor[3] = {kMagenta+2, kMagenta+2, kMagenta+2}; 
    Color_t fitColor[3] = {kGray+2, kGray+2, kGray+2}; 

    // Create main canvas
    // TCanvas *c1 = new TCanvas("c1", "Resonance Scans", 1000, 400);
    // c1->Divide(2,1, 0.01, 0.01); // Small margins between pads

    TCanvas *c1 = new TCanvas("c1", "Resonance Scans", 800, 1600);
    c1->Divide(1,3); //, 0.01, 0.01);
    // c1->SetTitle("Resonance Scans for #it{^{14}N(#alpha,#gamma)^{18}F}");
    
    // Configure global style
    // gStyle->SetOptTitle(1);
    gStyle->SetLabelSize(0.075, "XY");
    gStyle->SetTitleSize(0.08, "XY");
    gStyle->SetTitleOffset(1.1, "X");
    gStyle->SetTitleOffset(1.3, "Y");

    gStyle->SetStatW(0.0);
    gStyle->SetStatH(0.0);
    
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
        gPad->SetLeftMargin(0.11);
        gPad->SetRightMargin(0.005);
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
        graph->GetXaxis()->SetTitle("E_{#alpha} [keV]");
        graph->GetYaxis()->SetTitle("Yield [#muC^{-1}]");
        // graph->GetYaxis()->SetTitleSize(0.05);
        // Center axis titles
        graph->GetXaxis()->CenterTitle(true);
        graph->GetYaxis()->CenterTitle(true);
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
        // if (i != 0) {
        //     // Remove axis labels and ticks for Y axis (left) and X axis (bottom)
        //     graph->GetYaxis()->SetLabelSize(0);
        //     graph->GetYaxis()->SetTitle("");
        //     graph->GetYaxis()->SetNdivisions(0, kFALSE);
        //     // Do NOT reduce left margin, keep it same as first panel for alignment
        //     // gPad->SetLeftMargin(0.001); // <-- REMOVE or COMMENT this line

        //     // graph->GetXaxis()->SetLabelSize(0);
        //     // graph->GetXaxis()->SetTitle("");
        //     // graph->GetXaxis()->SetNdivisions(0, kFALSE);
        //     // gPad->SetBottomMargin(0.001);
        // }

        // resonance energy line with the arrorow signifying the resonance energy
        if (i == 0) {
            graph->GetXaxis()->SetLimits(resoEn[i]-20., resoEn[i]+70.);
            // graph->GetHistogram()->SetMinimum(1e-5);
            // graph->GetHistogram()->SetMaximum(4.0);
            graph->GetHistogram()->SetTitleOffset(1.5, "Y");
            // Only set axis labels and scales for the first plot
            gPad->SetTopMargin(0.005);

            // TLatex *label = new TLatex(0.18, 0.93, "^{14}N(#alpha, #gamma)^{18}F");
            // label->SetNDC();
            // label->SetTextSize(0.05);
            // label->SetTextFont(42);
            // label->Draw();

            // Draw a vertical line with an arrow at the top
            TArrow *arrowLine = new TArrow(resoEn[i], 1e-3, resoEn[i], 2.5e-5, 0.015, "");
            arrowLine->SetLineColor(kBlack);
            arrowLine->SetLineWidth(1);
            arrowLine->SetLineStyle(2);
            arrowLine->SetAngle(25); // Arrowhead opening angle
            arrowLine->SetFillColor(kBlack);
            arrowLine->Draw();

            TLatex *resLabel0 = new TLatex(0.47, 0.13, "E_{#it{lit}} = 573 #pm 3 keV");
            resLabel0->SetNDC();
            resLabel0->SetTextFont(42); // Use a standard font
            resLabel0->SetTextColor(kBlack);
            resLabel0->SetTextSize(0.040);
            resLabel0->Draw();

            // legend
            TLegend *legend = new TLegend(0.80, 0.87, 0.98, 0.97);
            legend->SetBorderSize(1);
            legend->SetFillStyle(0); // Transparent background
            legend->SetTextSize(0.04);
            legend->SetTextFont(42); // Use a standard font
            legend->AddEntry(graph, "Data", "p");
            legend->AddEntry(fit, "Fit", "l");
            legend->Draw();

            // Add resonance energy and width
            double yMax = fit->GetMaximum(570., 610.);
            double Eres = fit->GetX(0.50 * yMax, 570., 610.);
            double dEres = fit->GetParError(4); // Error on resonance energy
            double Gamma = fit->GetParameter(1); // gamma 1
            double dGamma = fit->GetParError(1); // Error on width
            // TLatex *resLabel1 = new TLatex(0.50, 0.20, Form("E_{r} = %.3f #pm %.3f keV", Eres, dEres));
            // resLabel1->SetNDC();
            // resLabel1->SetTextFont(42); // Use a standard font
            // resLabel1->SetTextColor(kBlack);
            // resLabel1->SetTextSize(0.040);
            // resLabel1->Draw();
            // TLatex *resLabel2 = new TLatex(0.50, 0.15, Form("#Gamma_{1} = %.3f #pm %.3f keV", Gamma, dGamma));
            // resLabel2->SetNDC();
            // resLabel2->SetTextFont(42); // Use a standard font
            // resLabel2->SetTextColor(kBlack);     
            // resLabel2->SetTextSize(0.040);
            // resLabel2->Draw();

             //// jet width
            double E1 = fit->GetX(0.50 * yMax, 565., 600.);
            double E2 = fit->GetX(0.50 * yMax, 610., 640.);

            double DE = E2 - E1;
            double dE = fit->GetParError(2); // Error in jet width
            TLatex *Label3 = new TLatex(0.40, 0.20, Form("#DeltaE = %.2f #pm %.2f keV", DE, dE));
            Label3->SetNDC();
            Label3->SetTextFont(42); // Use a standard font
            Label3->SetTextColor(kBlack);
            Label3->SetTextSize(0.040);
            Label3->Draw();

            TArrow *arrowLine2 = new TArrow(Eres, 1e-3, Eres, 1.8e-4, 0.010, "|>");
            arrowLine2->SetLineColor(kBlack);
            arrowLine2->SetLineWidth(1);
            arrowLine2->SetAngle(30); // Arrowhead opening angle
            arrowLine2->SetFillColor(kBlack);
            arrowLine2->Draw();
            // Add a label for the resonance energy
            TLatex *Label2 = new TLatex(0.20, 0.42, Form("E_{#it{#alpha}} = %.2f #pm %.2f keV", Eres, dEres));
            Label2->SetNDC();
            Label2->SetTextFont(42); // Use a standard font
            Label2->SetTextColor(kBlack);
            Label2->SetTextSize(0.040);
            Label2->Draw();
        }  
        else if (i == 1) {
            graph->GetXaxis()->SetLimits(resoEn[i]-20., resoEn[i]+60.);
            // graph->GetHistogram()->SetMinimum(1e-5);
            // graph->GetHistogram()->SetMaximum(4.0);
            graph->GetHistogram()->SetTitleOffset(1.5, "Y");

            gPad->SetTopMargin(0.0);

            // Draw a vertical line with an arrow at the top
            TArrow *arrowLine = new TArrow(resoEn[i], 4e-5, resoEn[i], 0.6e-2, 0.010, "|>");
            arrowLine->SetLineColor(kBlack);
            arrowLine->SetLineStyle(2);
            arrowLine->SetLineWidth(1);
            arrowLine->SetAngle(30); // Arrowhead opening angle
            arrowLine->SetFillColor(kBlack);
            arrowLine->Draw();
            // Add a label for the resonance energy
            // TLatex *resLabel = new TLatex(resoEn[i], 0.35, Form("E_{#it{lit}} = %.0f #pm 3 keV", resoEn[i]));
            // resLabel->SetTextColor(kGreen+2);
            // Use NDC coordinates for precise positioning: (x, y) in [0,1] relative to pad
            TLatex *resLabel0 = new TLatex(0.20, 0.14, "E_{lit} = 1136 #pm 3 keV");
            resLabel0->SetNDC();
            resLabel0->SetTextFont(42); // Use a standard font
            resLabel0->SetTextColor(kBlack);
            resLabel0->SetTextSize(0.040);
            resLabel0->Draw();

            // legend
            TLegend *legend = new TLegend(0.80, 0.87, 0.98, 0.97);
            gPad->SetTopMargin(0.01);
            legend->SetBorderSize(1);
            legend->SetFillStyle(0); // Transparent background
            legend->SetTextSize(0.04);
            legend->SetTextFont(42); // Use a standard font
            legend->AddEntry(graph, "Data", "p");
            legend->AddEntry(fit, "Fit", "l");
            legend->Draw();

            // Add resonance energy and width
            double yMax = fit->GetMaximum(1134., 1160.);
            double Eres = fit->GetX(0.50 * yMax, 1134., 1160.);
            double dEres = fit->GetParError(4); // Error on resonance energy
            double Gamma = fit->GetParameter(1); // gamma 1
            double dGamma = fit->GetParError(1); // Error on width

            //// jet width
            double E1 = fit->GetX(0.50 * yMax, 1134., 1150.);
            double E2 = fit->GetX(0.50 * yMax, 1155., 1175.);

            double DE = E2 - E1;
            double dE = fit->GetParError(2); // Error in jet width
            TLatex *Label3 = new TLatex(0.36, 0.60, Form("#DeltaE = %.2f #pm %.2f keV", DE, dE));
            Label3->SetNDC();
            Label3->SetTextFont(42); // Use a standard font
            Label3->SetTextColor(kBlack);
            Label3->SetTextSize(0.040);
            Label3->Draw();
        
            // TLatex *resLabel1 = new TLatex(0.50, 0.20, Form("E_{r} = %.3f #pm %.3f keV", Eres, dEres));
            // resLabel1->SetNDC();
            // resLabel1->SetTextFont(42); // Use a standard font
            // resLabel1->SetTextColor(kBlack);
            // resLabel1->SetTextSize(0.040);
            // resLabel1->Draw();
            // TLatex *resLabel2 = new TLatex(0.50, 0.15, Form("#Gamma_{1} = %.3f #pm %.3f keV", Gamma, dGamma));
            // resLabel2->SetNDC();
            // resLabel2->SetTextFont(42); // Use a standard font
            // resLabel2->SetTextColor(kBlack);     
            // resLabel2->SetTextSize(0.040);
            // resLabel2->Draw();

            TArrow *arrowLine2 = new TArrow(Eres, 0.2, Eres, 5.0e-2, 0.010, "|>");
            arrowLine2->SetLineColor(kBlack);
            arrowLine2->SetLineWidth(1);
            arrowLine2->SetAngle(30); // Arrowhead opening angle
            arrowLine2->SetFillColor(kBlack);
            arrowLine2->Draw();
            // Add a label for the resonance energy
            TLatex *Label2 = new TLatex(0.20, 0.79, Form("E_{#it{#alpha}} = %.2f #pm %.2f keV", Eres, dEres));
            Label2->SetNDC();
            Label2->SetTextFont(42); // Use a standard font
            Label2->SetTextColor(kBlack);
            Label2->SetTextSize(0.040);
            Label2->Draw();
        }
         else if (i == 2) {
            graph->GetXaxis()->SetLimits(resoEn[i]-20., resoEn[i]+60.);
            // graph->GetHistogram()->SetMinimum(1e-5);
            // graph->GetHistogram()->SetMaximum(4.0);
            graph->GetHistogram()->SetTitleOffset(1.5, "Y");
            gPad->SetTopMargin(0.0);
            // TLatex *label = new TLatex(0.18, 0.93, "^{14}N(#alpha, #gamma)^{18}F");
            // label->SetNDC();
            // label->SetTextSize(0.05);
            // label->SetTextFont(42);
            // label->Draw();
            // Draw a vertical line with an arrow at the top
            TArrow *arrowLine = new TArrow(resoEn[i], 5e-4, resoEn[i], 1e-1, 0.010, "|>");
            arrowLine->SetLineColor(kBlack);
            arrowLine->SetLineStyle(2);
            arrowLine->SetLineWidth(1);
            arrowLine->SetAngle(30); // Arrowhead opening angle
            arrowLine->SetFillColor(kBlack);
            arrowLine->Draw();

            TLatex *resLabel0 = new TLatex(0.20, 0.31, "E_{#it{lit}} = 1618 #pm 2 keV");
            resLabel0->SetNDC();
            resLabel0->SetTextFont(42); // Use a standard font
            resLabel0->SetTextColor(kBlack);
            resLabel0->SetTextSize(0.040);
            resLabel0->Draw();

            // legend
            TLegend *legend = new TLegend(0.80, 0.87, 0.98, 0.97);
            gPad->SetTopMargin(0.01);
            legend->SetBorderSize(1);
            legend->SetFillStyle(0); // Transparent background
            legend->SetTextSize(0.04);
            legend->SetTextFont(42); // Use a standard font
            legend->AddEntry(graph, "Data", "p");
            legend->AddEntry(fit, "Fit", "l");
            legend->Draw();

            // Add resonance energy and width
            double yMax = fit->GetMaximum(1616., 1638.);
            double Eres = fit->GetX(0.50 * yMax, 1616., 1638.);
            double dEres = fit->GetParError(4); // Error on resonance energy
            double Gamma = fit->GetParameter(1); // gamma 1
            double dGamma = fit->GetParError(1); // Error on width
            // TLatex *resLabel1 = new TLatex(0.50, 0.20, Form("E_{r} = %.3f #pm %.3f keV", Eres, dEres));
            // resLabel1->SetNDC();
            // resLabel1->SetTextFont(42); // Use a standard font
            // resLabel1->SetTextColor(kBlack);
            // resLabel1->SetTextSize(0.040);
            // resLabel1->Draw();
            // TLatex *resLabel2 = new TLatex(0.50, 0.15, Form("#Gamma_{1} = %.3f #pm %.3f keV", Gamma, dGamma));
            // resLabel2->SetNDC();
            // resLabel2->SetTextFont(42); // Use a standard font
            // resLabel2->SetTextColor(kBlack);     
            // resLabel2->SetTextSize(0.040);
            // resLabel2->Draw();

             //// jet width
            double E1 = fit->GetX(0.50 * yMax, 1614., 1635.);
            double E2 = fit->GetX(0.50 * yMax, 1635., 1665.);

            double DE = E2 - E1;
            double dE = fit->GetParError(2); // Error in jet width
            TLatex *Label3 = new TLatex(0.35, 0.65, Form("#DeltaE = %.2f #pm %.2f keV", DE, dE));
            Label3->SetNDC();
            Label3->SetTextFont(42); // Use a standard font
            Label3->SetTextColor(kBlack);
            Label3->SetTextSize(0.040);
            Label3->Draw();

            TArrow *arrowLine2 = new TArrow(Eres, 1.5, Eres, 5.0e-1, 0.010, "|>");
            arrowLine2->SetLineColor(kBlack);
            arrowLine2->SetLineWidth(1);
            arrowLine2->SetAngle(30); // Arrowhead opening angle
            arrowLine2->SetFillColor(kBlack);
            arrowLine2->Draw();
            // Add a label for the resonance energy
            TLatex *Label2 = new TLatex(0.20, 0.93, Form("E_{#it{#alpha}} = %.2f #pm %.2f keV", Eres, dEres));
            Label2->SetNDC();
            Label2->SetTextFont(42); // Use a standard font
            Label2->SetTextColor(kBlack);
            Label2->SetTextSize(0.040);
            Label2->Draw();
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
    c1->SaveAs("plot_n14ag_thesis_251118.pdf");
    // c1->SaveAs("n14ag_scan/plot_n14ag_v4.png", "pngdpi=600");
}
