#include "TLine.h"
#include "TMarker.h"
#include "TGraph.h"
#include "TGraphErrors.h"
#include "TRandom.h"
#include "TStyle.h"
#include "stdio.h"
#include "iostream"
#include "fstream"

void scan_res3_Anup(TString filename="Yield_1618keV_resonance.txt"){ 

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

    // Create the first graph (columns 0, 1, 2)
    TGraphErrors *gr1 = new TGraphErrors(energy.size(), &energy[0], &yield1[0], 0, &unc1[0]);
    gr1->SetMarkerStyle(20);
    gr1->SetMarkerColor(kBlue);
    gr1->SetTitle("1618keV_res_1081keV #gamma;Energy #it{[keV]};Yield #it{[a.u.]}");

    // Create the second graph (columns 0, 3, 4)
    TGraphErrors *gr2 = new TGraphErrors(energy.size(), &energy[0], &yield2[0], 0, &unc2[0]);
    gr2->SetMarkerStyle(20);
    gr2->SetMarkerColor(kBlue);
    gr2->SetTitle(";Energy #it{[keV]};Yield #it{[a.u.]}");

    //Create the third graph (columns 0, 5, 6)
    TGraphErrors *gr3 = new TGraphErrors(energy.size(), &energy[0], &yield3[0], 0, &unc3[0]);
    gr3->SetMarkerStyle(22);
    gr3->SetMarkerColor(kGreen);
    gr3->SetTitle(";Energy #it{[keV]};Yield #it{[a.u.]}");    

    // Create the first canvas and draw the first graph
    TCanvas *c1 = new TCanvas("c1", "Peak 1081 keV", 1200, 800);
    gr1->Draw("AP");

    // Fit the first graph with arctan function
    TF1* f1 = new TF1("f1", "[0]*(atan((x-[4])/[1])-atan((x-[4]-[2])/[3]))+[5]", 1608, 1660);
    f1->SetParName(0, "wg");
    f1->SetParName(1, "Gamma1/2");
    f1->SetParName(2, "dE");
    f1->SetParName(3, "Gamma2/2");
    f1->SetParName(4, "Res energy");
    f1->SetParName(5, "Offset");

    f1->SetParameter(0, 1.0);  // Amplitude (wg)
    f1->SetParameter(1, 1.2);   // Half-width of the first arctan (Gamma1/2)
    f1->SetParameter(2, 20.0);  // Energy difference (DE)
    f1->SetParameter(3, 2.4);   // Half-width of the second arctan (Gamma2/2)
    f1->SetParameter(4, 1618.0); // Resonance energy (Res Energy)
    f1->SetParameter(5, 0.0);   // Vertical offset (Offset)

    gr1->Fit("f1","W");
    gr1->SetMinimum(-0.1);
    gr1->SetMaximum(1.5);
    gStyle->SetOptStat(0);

    gStyle->SetStatFontSize(0.02); 
    gStyle->SetStatW(0.1);         
    gStyle->SetStatH(0.1);
    c1->Update();
    c1->Print("1618keV_res_1018keV_g.png");

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

    gr2->Fit("f2","W");
    gr2->SetMinimum(-0.01);
    gr2->SetMaximum(0.4);
    //gStyle->SetOptStat(0);
    gStyle->SetStatFontSize(0.04); 
    gStyle->SetStatW(0.1);         
    gStyle->SetStatH(0.1);
    c2->Update();
    c2->Print("1618keV_res_2539keV_g.png");

    // Create the third canvas and draw the third graph
    TCanvas *c3 = new TCanvas("c3", "Peak 4592 keV", 1200, 800);
    gr3->Draw("AP");
    // Fit the third graph with arctan function
    TF1* f3 = new TF1("f3", "[0]*(atan((x-[4])/[1])-atan((x-[4]-[2])/[3]))+[5]", 1608, 1660);
    f3->SetParName(0, "wg");
    f3->SetParName(1, "Gamma1/2");
    f3->SetParName(2, "dE");
    f3->SetParName(3, "Gamma2/2");
    f3->SetParName(4, "Res energy");
    f3->SetParName(5, "Offset");

    f3->SetParameter(0, 0.35);  
    f3->SetParameter(1, 1.4);   
    f3->SetParameter(2, 20.0);  
    f3->SetParameter(3, 3.5);  
    f3->SetParameter(4, 1618.0); 
    f3->SetParameter(5, 0.0);

    gr3->Fit("f3","W");
    gr3->SetMinimum(-0.02);
    gr3->SetMaximum(0.3);
    TStyle *gStyle = new TStyle("gStyle", "Style for P-TDR");
    gStyle->SetStatFontSize(0.01);
    gStyle->SetStatFontSize(0.01);
    c3->Update();
    c3->Print("1618keV_res_4592keV_g.png");

    // Get fitted parameters
    double Gamma1_2 = f2->GetParameter(1);  // Gamma1/2
    double dE = f2->GetParameter(2);        // dE
    double Gamma2_2 = f2->GetParameter(3);  // Gamma2/2

    // Compute the FWHM
    double FWHM = dE + (Gamma1_2 / 2.0) + (Gamma2_2 / 2.0);

    // Print the result
    printf("FWHM of the fitted function: %.3f keV\n", FWHM);

}