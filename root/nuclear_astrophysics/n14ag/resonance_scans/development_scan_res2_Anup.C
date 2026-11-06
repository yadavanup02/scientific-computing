#include "TLine.h"
#include "TMarker.h"
#include "TGraph.h"
#include "TGraphErrors.h"
#include "TRandom.h"
#include "stdio.h"
#include "iostream"
#include "fstream"

void scan_res2_Anup(TString filename="resonance_1529keV.txt"){ 

    // Read the data from the file
    std::ifstream infile(filename.Data());
    std::vector<double> energy, yield1, unc1, yield2, unc2;
    double e, y1, u1, y2, u2;
    std::string header;
    std::getline(infile, header); // Skip the header line
    while (infile >> e >> y1 >> u1 >> y2 >> u2) {
        energy.push_back(e);
        yield1.push_back(y1);
        unc1.push_back(u1);
        yield2.push_back(y2);
        unc2.push_back(u2);
    }

    // Create the first graph (columns 0, 1, 2)
    TGraphErrors *gr1 = new TGraphErrors(energy.size(), &energy[0], &yield1[0], 0, &unc1[0]);
    gr1->SetMarkerStyle(20);
    gr1->SetMarkerColor(kBlue);
    gr1->SetTitle("1529keV_res_2125keV gamma;TV [kV];Yield [a.u.]");

    // Create the second graph (columns 0, 3, 4)
    TGraphErrors *gr2 = new TGraphErrors(energy.size(), &energy[0], &yield2[0], 0, &unc2[0]);
    gr2->SetMarkerStyle(21);
    gr2->SetMarkerColor(kRed);
    gr2->SetTitle("1529keV_res_4524keV gamma;TV [kV];Yield [a.u.]");

    // Create the first canvas and draw the first graph
    TCanvas *c1 = new TCanvas("c1", "Graph 1", 1200, 800);
    gr1->Draw("AP");

    // Fit the first graph with arctan function
    TF1* f1 = new TF1("f1", "[0]*(atan((x-[4])/[1])-atan((x-[4]-[2])/[3]))+[5]", 1560, 1625);
    f1->SetParName(0, "wg");
    f1->SetParName(1, "Gamma1/2");
    f1->SetParName(2, "dE");
    f1->SetParName(3, "Gamma2/2");
    f1->SetParName(4, "Res energy");
    f1->SetParName(5, "Offset");

    f1->SetParameter(0, 0.25);  // Amplitude (wg)
    f1->SetParameter(1, 5.0);   // Half-width of the first arctan (Gamma1/2)
    f1->SetParameter(2, 20.0);  // Energy difference (DE)
    f1->SetParameter(3, 5.0);   // Half-width of the second arctan (Gamma2/2)
    f1->SetParameter(4, 1583.0); // Resonance energy (Res Energy)
    f1->SetParameter(5, 0.0);   // Vertical offset (Offset)

    gr1->Fit("f1","w");
    c1->Update();
    c1->Print("1529keV_res_2125keVg.png");

    // Create the second canvas and draw the second graph
    TCanvas *c2 = new TCanvas("c2", "Graph 2", 1200, 800);
    gr2->Draw("AP");

    // Fit the second graph with arctan function
    TF1* f2 = new TF1("f2", "[0]*(atan((x-[4])/[1])-atan((x-[4]-[2])/[3]))+[5]", 1560, 1625);
    f2->SetParName(0, "wg");
    f2->SetParName(1, "Gamma1/2");
    f2->SetParName(2, "dE");
    f2->SetParName(3, "Gamma2/2");
    f2->SetParName(4, "Res energy");
    f2->SetParName(5, "Offset");

    f2->SetParameter(0, 0.25);  // Amplitude (wg)
    f2->SetParameter(1, 5.0);   // Half-width of the first arctan (Gamma1/2)
    f2->SetParameter(2, 20.0);  // Energy difference (DE)
    f2->SetParameter(3, 5.0);   // Half-width of the second arctan (Gamma2/2)
    f2->SetParameter(4, 1583.0); // Resonance energy (Res Energy)
    f2->SetParameter(5, 0.0);   // Vertical offset (Offset)

    gr2->Fit("f2","w");
    c2->Update();
    c2->Print("1529keV_res_4524keVg.png");
}