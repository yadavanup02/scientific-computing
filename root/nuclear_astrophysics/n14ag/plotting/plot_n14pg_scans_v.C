#include "TFile.h"
#include "TCanvas.h"
#include "TGraphErrors.h"
#include "TLegend.h"

#include "TLatex.h"
#include "TStyle.h"

void plot_n14pg_scans_v() {
    // Input files and resonance info
    TString fileName[3] = {"Resonance_278keV/scan278keV_EB18_G_p1_v3.root", "Resonance_430keV/scan430keV_EB18_G_p1_v2.root", "Resonance_1058keV/scan1058keV_EB18_G_p1_v2.root"};
    TString plotName[3] = {"#it{E}_{#alpha} = 278 keV", "#it{E}_{#alpha} = 430 keV", "#it{E}_{#alpha} = 1058 keV"};
    double resoEn[3] = {278., 429., 1058.};
    TString cName[3] = {"c", "c", "c"};

    // Modern color palette
    // Color_t markerColor[3] = {kBlack, kBlack, kBlack}; 
    // Color_t fitColor[3] = {kMagenta+2, kMagenta+2, kMagenta+2}; 

    Color_t markerColor[3] = {kMagenta+2, kMagenta+2, kMagenta+2}; 
    Color_t fitColor[3] = {kGray+2, kGray+2, kGray+2}; 

    // Create main canvas
    // TCanvas *c1 = new TCanvas("c1", "Resonance Scans", 1000, 400);
    // c1->Divide(2,1, 0.01, 0.01); // Small margins between pads

    TCanvas *c1 = new TCanvas("c1", "Resonance Scans", 800, 1200);
    c1->Divide(1,3, 0.01, 0.01);
    // c1->SetTitle("Resonance Scans for #it{^{14}N(#alpha,#gamma)^{18}F}");
    
    // Configure global style
    // gStyle->SetOptTitle(1);
    gStyle->SetLabelSize(0.095, "XY");
    gStyle->SetTitleSize(0.3, "XY");
    gStyle->SetTitleOffset(1.1, "X");
    gStyle->SetTitleOffset(1.3, "Y");
    // center the axis titles
    // gStyle->SetTitleAlign(22, "X");
    // gStyle->SetTitleAlign(22, "Y");

    
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
        gPad->SetBottomMargin(0.081);

        // gPad->SetLogy();
        // gPad->SetGridx();
        // gPad->SetGridy();
        
        // Style the graph
        graph->SetMarkerStyle(20); // Different markers
        graph->SetMarkerSize(1.0);
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
        graph->GetXaxis()->CenterTitle(true);
        graph->GetYaxis()->CenterTitle(true);
        graph->Draw("AP");
        graph->GetYaxis()->SetTickLength(0.015);
        
        // Style and draw fit
        fit->SetLineColor(fitColor[i]);
        fit->SetLineWidth(1);
        fit->Draw("same");

        double yMax1 = fit->GetMaximum(274, 284);
        double yMax2 = fit->GetMaximum(427, 435);
        double yMax3 = fit->GetMaximum(1050, 1060);

        double Eres1 = fit->GetX(0.50*yMax1, 274, 284);
        double Eres2 = fit->GetX(0.50*yMax2, 427, 435);
        double Eres3 = fit->GetX(0.50*yMax3, 1050, 1060);

        double dEres = fit->GetParError(4); // Error on resonance energy

        double Gamma1 = fit->GetParameter(1); // gamma 1
        double dGamma1 = fit->GetParError(1); // Error on width

        // TLatex *resLabel1 = new TLatex(0.18, 0.96, Form("E_{r} = %.3f #pm %.3f keV", Eres, dEres));
        // resLabel1->SetNDC();
        // resLabel1->SetTextFont(42); // Use a standard font
        // resLabel1->SetTextColor(kBlack);    
        // resLabel1->SetTextSize(0.040);
        // resLabel1->Draw();

        TLatex *resLabel2 = new TLatex(0.15, 0.88, Form("#Gamma_{1} = %.2f #pm %.2f keV", Gamma1, dGamma1));
        resLabel2->SetNDC();
        resLabel2->SetTextFont(42); // Use a standard font
        resLabel2->SetTextColor(kBlack);    
        resLabel2->SetTextSize(0.040);
        resLabel2->Draw();

        // resonance energy line with the arrorow signifying the resonance energy
        if (i == 0) {
            // FWHM calculation 
            double E1 = fit->GetX(yMax1/2.0 , 275, 282);
            double E2 = fit->GetX(yMax1/2.0 , 283, 290);

            // std::cout << "\nE1: " << E1 << std::endl;
            // std::cout << "\nE2: " << E2 << std::endl;
            double DE =  E2 - E1;

            double dE = fit->GetParError(2); // Error in jet width
            TLatex *Label3 = new TLatex(0.45, 0.12, Form("#DeltaE = %.1f #pm %.1f keV", DE, dE));
            Label3->SetNDC();
            Label3->SetTextFont(42); // Use a standard font
            Label3->SetTextColor(kBlack);
            Label3->SetTextSize(0.040);
            Label3->Draw();

            // std::cout << "\nJet thickness:: " << jet_thickness << std::endl;

            graph->GetYaxis()->SetTickLength(0.015);
            graph->GetXaxis()->SetLimits(resoEn[i]-10., resoEn[i]+15.);
            graph->GetHistogram()->SetMinimum(0.0);
            graph->GetHistogram()->SetMaximum(0.41);
            graph->GetHistogram()->SetTitleOffset(1.5, "Y");
            graph->GetHistogram()->SetTitleSize(0.04, "Y");
            graph->GetHistogram()->SetLabelSize(0.04, "Y");
            //  X axis title size
            graph->GetXaxis()->SetTitleSize(0.04);
            graph->GetXaxis()->SetLabelSize(0.04);
            gPad->SetTopMargin(0.005);

            TLatex *label = new TLatex(0.75, 0.95, "^{14}N(p, #gamma)^{15}O");
            label->SetNDC();
            label->SetTextSize(0.045);
            label->SetTextFont(42);
            label->Draw();
            

            // legend
            TLegend *legend = new TLegend(0.80, 0.83, 0.98, 0.92);
            legend->SetBorderSize(1);
            legend->SetFillStyle(0); // Transparent background
            legend->SetTextSize(0.04);
            legend->SetTextFont(42); // Use a standard font
            legend->AddEntry(graph, "Data", "p");
            legend->AddEntry(fit, "Fit", "l");
            legend->Draw();

            // TLatex *resLabel1 = new TLatex(0.18, 0.96, Form("E_{r} = %.3f #pm %.3f keV", Eres1, dEres));
            // resLabel1->SetNDC();
            // resLabel1->SetTextFont(42); // Use a standard font
            // resLabel1->SetTextColor(kBlack);    
            // resLabel1->SetTextSize(0.040);
            // resLabel1->Draw();

            // Draw a vertical line with an arrow at the top
            TArrow *Line1 = new TArrow(resoEn[i], 0.05, resoEn[i], 0.165, 0.010, "|>");
            Line1->SetLineColor(kBlack);
            Line1->SetLineStyle(2);
            Line1->SetLineWidth(1);
            Line1->SetAngle(25); // Arrowhead opening angle
            Line1->SetFillColor(kBlack);
            Line1->Draw();

            TLatex *resLabel0 = new TLatex(0.43, 0.16, "E_{lit} = 278.1 #pm 0.4 keV");
            resLabel0->SetNDC();
            resLabel0->SetTextFont(42); // Use a standard font
            resLabel0->SetTextColor(kBlack);
            resLabel0->SetTextSize(0.040);
            resLabel0->Draw();

            TArrow *arrowLine2 = new TArrow(Eres1, 0.33, Eres1, 0.19, 0.010, "|>");
            arrowLine2->SetLineColor(kBlack);
            arrowLine2->SetLineWidth(1);
            arrowLine2->SetAngle(30); // Arrowhead opening angle
            arrowLine2->SetFillColor(kBlack);
            arrowLine2->Draw();
            // Add a label for the resonance energy
            TLatex *Label2 = new TLatex(0.15, 0.93, Form("E_{p} = %.1f #pm %.1f keV", Eres1, dEres));
            Label2->SetNDC();
            Label2->SetTextFont(42); // Use a standard font
            Label2->SetTextColor(kBlack);
            Label2->SetTextSize(0.040);
            Label2->Draw();

            // TLatex *resLabel = new TLatex(0.18, 0.87, Form("E_{r} = %.1f #pm %.1f keV\n#Gamma = %.1f #pm %.1f keV", Eres, dEres, Gamma1, dGamma1));
            // resLabel->SetNDC();
            // resLabel->SetTextColor(kBlack);    
            // resLabel->SetTextSize(0.035);
            // resLabel->Draw();
        }  
        else if (i == 1) {

            // FWHM calculation 
            double E1 = fit->GetX(yMax2/2.0 , 427, 434);
            double E2 = fit->GetX(yMax2/2.0 , 435, 441);

            // std::cout << "\nE1: " << E1 << std::endl;
            // std::cout << "\nE2: " << E2 << std::endl;
            double DE =  E2 - E1;

            double dE = fit->GetParError(2); // Error in jet width
            TLatex *Label3 = new TLatex(0.46, 0.12, Form("#DeltaE = %.2f #pm %.2f keV", DE, dE));
            Label3->SetNDC();
            Label3->SetTextFont(42); // Use a standard font
            Label3->SetTextColor(kBlack);
            Label3->SetTextSize(0.040);
            Label3->Draw();
            graph->GetYaxis()->SetTitleSize(0.045);
            graph->GetYaxis()->SetLabelSize(0.040);
            graph->GetXaxis()->SetTitleSize(0.045);
            graph->GetXaxis()->SetLabelSize(0.040);
            gPad->SetTopMargin(0.0);
            gPad->SetBottomMargin(0.11);
            gPad->SetLeftMargin(0.08);
            graph->GetYaxis()->SetTickLength(0.015);

            graph->GetXaxis()->SetLimits(resoEn[i]-10., resoEn[i]+15.);
            graph->GetHistogram()->SetMinimum(0.0);
            graph->GetHistogram()->SetMaximum(2.0);
            graph->GetHistogram()->SetTitleOffset(1.3, "Y");
            graph->GetHistogram()->SetTitleSize(0.04, "Y");
            graph->GetHistogram()->SetLabelSize(0.04, "Y");
            TLatex *label = new TLatex(0.73, 0.95, "^{15}N(p, #alpha#gamma)^{12}C");
            label->SetNDC();
            label->SetTextSize(0.045);
            label->SetTextFont(42);
            label->Draw();
            // Draw a vertical line with an arrow at the top
            // TArrow *arrowLine = new TArrow(resoEn[i], 0.0, resoEn[i], 0.2, 0.015, "|>");
            // arrowLine->SetLineColor(kBlack);
            // arrowLine->SetLineWidth(1);
            // arrowLine->SetAngle(30); // Arrowhead opening angle
            // arrowLine->SetFillColor(kBlack);
            // arrowLine->Draw();

            // legend
            TLegend *legend = new TLegend(0.80, 0.83, 0.98, 0.92);
            gPad->SetTopMargin(0.01);
            legend->SetBorderSize(1);
            legend->SetFillStyle(0); // Transparent background
            legend->SetTextSize(0.04);
            legend->SetTextFont(42); // Use a standard font
            legend->AddEntry(graph, "Data", "p");
            legend->AddEntry(fit, "Fit", "l");
            legend->Draw();
            // TLatex *resLabel1 = new TLatex(0.18, 0.96, Form("E_{r} = %.3f #pm %.3f keV", Eres2, dEres));
            // resLabel1->SetNDC();
            // resLabel1->SetTextFont(42); // Use a standard font
            // resLabel1->SetTextColor(kBlack);    
            // resLabel1->SetTextSize(0.040);
            // resLabel1->Draw();

            // Draw a vertical line with an arrow at the top
            TArrow *Line1 = new TArrow(resoEn[i], 0.25, resoEn[i], .4, 0.010, "|>");
            Line1->SetLineColor(kBlack);
            Line1->SetLineStyle(2);
            Line1->SetLineWidth(1);
            Line1->SetAngle(25); // Arrowhead opening angle
            Line1->SetFillColor(kBlack);
            Line1->Draw();

            TLatex *resLabel0 = new TLatex(0.46, 0.15, "E_{lit} = 429 #pm 1 keV");
            resLabel0->SetNDC();
            resLabel0->SetTextFont(42); // Use a standard font
            resLabel0->SetTextColor(kBlack);
            resLabel0->SetTextSize(0.040);
            resLabel0->Draw();

            TArrow *arrowLine2 = new TArrow(Eres2, 1.7, Eres2, 1.0, 0.010, "|>");
            arrowLine2->SetLineColor(kBlack);
            arrowLine2->SetLineWidth(1);
            arrowLine2->SetAngle(30); // Arrowhead opening angle
            arrowLine2->SetFillColor(kBlack);
            arrowLine2->Draw();
            // Add a label for the resonance energy
            TLatex *Label2 = new TLatex(0.15, 0.93, Form("E_{p} = %.1f #pm %.1f keV", Eres2, dEres));
            Label2->SetNDC();
            Label2->SetTextFont(42); // Use a standard font
            Label2->SetTextColor(kBlack);
            Label2->SetTextSize(0.040);
            Label2->Draw();
        }
         else if (i == 2) {

            // FWHM calculation 
            double E1 = fit->GetX(yMax3/2.0 , 1050, 1060);
            double E2 = fit->GetX(yMax3/2.0 , 1060, 1070);

            // std::cout << "\nE1: " << E1 << std::endl;
            // std::cout << "\nE2: " << E2 << std::endl;
            double DE =  E2 - E1;

            double dE = fit->GetParError(2); // Error in jet width
            TLatex *Label3 = new TLatex(0.39, 0.12, Form("#DeltaE = %.1f #pm %.1f keV", DE, dE));
            Label3->SetNDC();
            Label3->SetTextFont(42); // Use a standard font
            Label3->SetTextColor(kBlack);
            Label3->SetTextSize(0.040);
            Label3->Draw(); 

            graph->GetXaxis()->SetLimits(resoEn[i]-12., resoEn[i]+15.);
            graph->GetHistogram()->SetMinimum(0.0);
            graph->GetHistogram()->SetMaximum(1.05);
            graph->GetHistogram()->SetTitleOffset(1.3, "Y");
            graph->GetHistogram()->SetTitleSize(0.04, "Y");
            graph->GetHistogram()->SetLabelSize(0.04, "Y");
            //  X axis title size
            graph->GetXaxis()->SetTitleSize(0.04);
            graph->GetXaxis()->SetLabelSize(0.04);
            gPad->SetTopMargin(0.0);
            TLatex *label = new TLatex(0.75, 0.95, "^{14}N(p, #gamma)^{15}O");
            label->SetNDC();
            label->SetTextSize(0.045);
            label->SetTextFont(42);
            label->Draw();
    
            // legend
            TLegend *legend = new TLegend(0.80, 0.83, 0.98, 0.92);
            gPad->SetTopMargin(0.01);
            legend->SetBorderSize(1);
            legend->SetFillStyle(0); // Transparent background
            legend->SetTextSize(0.04);
            legend->SetTextFont(42); // Use a standard font
            legend->AddEntry(graph, "Data", "p");
            legend->AddEntry(fit, "Fit", "l");
            legend->Draw();

            // Draw a vertical line with an arrow at the top
            TArrow *Line1 = new TArrow(resoEn[i], 0.15, resoEn[i], 0.84, 0.010, "|>");
            Line1->SetLineColor(kBlack);
            Line1->SetLineStyle(2);
            Line1->SetLineWidth(1);
            Line1->SetAngle(25); // Arrowhead opening angle
            Line1->SetFillColor(kBlack);
            Line1->Draw();

            TLatex *resLabel0 = new TLatex(0.35, 0.16, "E_{lit} = 1058.0 #pm 0.5 keV");
            resLabel0->SetNDC();
            resLabel0->SetTextFont(42); // Use a standard font
            resLabel0->SetTextColor(kBlack);
            resLabel0->SetTextSize(0.040);
            resLabel0->Draw();

            TArrow *arrowLine2 = new TArrow(Eres3, 0.90, Eres3, 0.5, 0.010, "|>");
            arrowLine2->SetLineColor(kBlack);
            arrowLine2->SetLineWidth(1);
            arrowLine2->SetAngle(30); // Arrowhead opening angle
            arrowLine2->SetFillColor(kBlack);
            arrowLine2->Draw();
            // Add a label for the resonance energy
            TLatex *Label2 = new TLatex(0.15, 0.93, Form("E_{p} = %.1f #pm %.1f keV", Eres3, dEres));
            Label2->SetNDC();
            Label2->SetTextFont(42); // Use a standard font
            Label2->SetTextColor(kBlack);
            Label2->SetTextSize(0.040);
            Label2->Draw();
        }
        
        file->Close();
    }

    c1->cd(); // Return to main canvas
    // Save output
    c1->SaveAs("Resonance_278keV/plot_n14pg_all_thesis_v2.pdf");
    // c1->SaveAs("Resonance_278keV/plot_n14pg_all_v4.png");
}
