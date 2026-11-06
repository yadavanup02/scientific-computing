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

void scan_res4_Anup(TString filename="Yield_1618keV_resonance.txt"){ 

    // Read the data from the file
    std::ifstream infile(filename.Data());
    std::vector<double> energy, yield1, yield2, yield3, unc1, unc2, unc3;
    double e, y1, y2, y3,  u1, u2, u3;
    std::string header;
    std::getline(infile, header); // Skip the header line
    while (infile >> e >> y1 >> y2 >> y3 >> u1 >> u2 >> u3) {
        energy.push_back(e);
        yield1.push_back(y1);
        yield2.push_back(y2);
        yield3.push_back(y3);
        unc1.push_back(u1);
        unc2.push_back(u2);
        unc3.push_back(u3);
    }
    
   // Create the second graph (columns 0, 3, 4)
    TGraphErrors *gr2 = new TGraphErrors(energy.size(), &energy[0], &yield2[0], 0, &unc2[0]);
    gr2->SetMarkerStyle(20);
    gr2->SetMarkerColor(kBlue);
    gr2->SetTitle(";Energy #it{[keV]};Yield #it{[a.u.]}");
    gStyle->SetOptStat(0);
    // Create the second canvas and draw the second graph
    TCanvas *c2 = new TCanvas("c2", "Peak 2539 keV", 800, 600);
    gr2->Draw("AP");

    // Fit the second graph with arctan function
    TF1* f2 = new TF1("f2", "[0]*(atan((x-[4])/[1])-atan((x-[4]-[2])/[3]))+[5]", 1608, 1658);
    f2->SetParName(0, "wg");
    f2->SetParName(1, "Gamma1/2");
    f2->SetParName(2, "dE");
    f2->SetParName(3, "Gamma2/2");
    f2->SetParName(4, "Res energy");
    f2->SetParName(5, "Offset");

    f2->SetParameter(0, 0.4);  // Amplitude (wg)
    f2->SetParameter(1, 1.2);   // Half-width of the first arctan (Gamma1/2)
    f2->SetParameter(2, 20.0);  // Energy difference (DE)
    f2->SetParameter(3, 2.6);   // Half-width of the second arctan (Gamma2/2)
    f2->SetParameter(4, 1618.0); // Resonance energy (Res Energy)
    f2->SetParameter(5, 0.0);   // Vertical offset (Offset)

    gr2->Fit("f2", "W");
    gr2->SetMinimum(-0.01);
    gr2->SetMaximum(0.4);

    // Get fitted parameters
    double Gamma1_2 = f2->GetParameter(1);  // Gamma1/2
    double dE = f2->GetParameter(2);        // dE
    double Gamma2_2 = f2->GetParameter(3);  // Gamma2/2

    // Compute the FWHM
    double FWHM = dE + (Gamma1_2 / 2.0) + (Gamma2_2 / 2.0);

    // Print the result
    printf("FWHM of the fitted function: %.3f keV\n", FWHM);

    // Calculate FWHM
    double xMin = 1608, xMax = 1658;
    double yMax = f2->GetMaximum(xMin, xMax);
    double yHalfMax = yMax / 2.0;



    // Draw a horizontal line at half-maximum
    TLine *line = new TLine(1621, yHalfMax, 1643, yHalfMax);
    line->SetLineColor(kOrange);
    line->SetLineStyle(1); // Solid line
    line->SetLineWidth(2);
    line->Draw("same");

    // Draw arrows at half-maximum on both sides
    TArrow *arrow1 = new TArrow(1621, yHalfMax, 1621, yHalfMax, 0.02, "<|");
    arrow1->SetLineColor(kOrange);
    arrow1->SetLineStyle(1); // Dashed line
    arrow1->Draw();

    TArrow *arrow2 = new TArrow(1643.5, yHalfMax, 1643.5, yHalfMax, 0.02, "|>");
    arrow2->SetLineColor(kOrange);
    arrow2->SetLineStyle(1); // Dashed line
    arrow2->Draw();

    // Add FWHM text
    TLatex *text = new TLatex(1627, yHalfMax + 0.01, Form("FWHM = %.2f keV", f2->GetParameter(2)));
    text->SetTextSize(0.03);
    text->SetTextColor(kBlack);
    text->SetTextFont(42); // Use a non-bold font
    text->Draw();

    // // Display FWHM value on the plot
    // TLatex latex;
    // latex.SetTextSize(0.04);
    // latex.SetTextColor(kRed);
    // latex.DrawLatex((x1 + x2) / 2, yHalfMax * 1.1, Form("FWHM = %.1f keV", FWHM));

    gStyle->SetStatFontSize(0.000); 
    gStyle->SetStatW(0.0);         
    gStyle->SetStatH(0.0);
    c2->Update();
    c2->Print("1618keV_res_2539keV_g.png", "png600");
    c2->Modified();


    
    

}