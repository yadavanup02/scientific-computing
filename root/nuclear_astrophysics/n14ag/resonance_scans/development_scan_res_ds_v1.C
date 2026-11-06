#include "TLine.h"
#include "TMarker.h"
#include "TGraph.h"
#include "TGraphErrors.h"
#include "TRandom.h"
#include "TStyle.h"
#include "TCanvas.h"
#include "TLegend.h"
#include "stdio.h"
#include "iostream"
#include "fstream"

void scan_res_ds_v1(TString filename="Yield_1618keV_resonance.txt") { 

    // Read the data from the file
    std::ifstream infile(filename.Data());
    std::vector<double> energy, yield1, yield2, yield3, unc1, unc2, unc3;
    double e, y1, y2, y3, u1, u2, u3;
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

    // Ensure there are at least 18 points
    if (energy.size() < 17) {
        printf("Error: Expected at least 18 data points, but got %lu.\n", energy.size());
        return;
    }

    // Create three different graphs for different color segments
    TGraphErrors *grRed = new TGraphErrors();
    TGraphErrors *grBlack = new TGraphErrors();
    TGraphErrors *grBlue = new TGraphErrors();

    // Track separate indices for each graph
    int idxRed = 0, idxBlack = 0, idxBlue = 0;

    for (size_t i = 0; i < energy.size(); ++i) {
        if (i < 5) {  // First 6 points -> Red
            grRed->SetPoint(idxRed, energy[i], yield1[i]);
            grRed->SetPointError(idxRed, 0, unc1[i]);
            idxRed++;
        }
        else if (i < 11) {  // Next 6 points -> Black
            grBlack->SetPoint(idxBlack, energy[i], yield1[i]);
            grBlack->SetPointError(idxBlack, 0, unc1[i]);
            idxBlack++;
        }
        else {  // Last 6 points -> Blue
            grBlue->SetPoint(idxBlue, energy[i], yield1[i]);
            grBlue->SetPointError(idxBlue, 0, unc1[i]);
            idxBlue++;
        }
    }

    // Set marker styles and colors
    grRed->SetMarkerStyle(20);
    grRed->SetMarkerColor(kRed);
    grRed->SetLineColor(kRed);

    grBlack->SetMarkerStyle(20);
    grBlack->SetMarkerColor(kBlack);
    grBlack->SetLineColor(kBlack);

    grBlue->SetMarkerStyle(20);
    grBlue->SetMarkerColor(kBlue);
    grBlue->SetLineColor(kBlue);

    // Create a canvas
    TCanvas *c1 = new TCanvas("c1", "Graph with Three Colors", 1200, 800);

    // Draw graphs
    grRed->Draw("AP");
    grBlack->Draw("P SAME");
    grBlue->Draw("P SAME");

    // Add a legend
    TLegend *legend = new TLegend(0.7, 0.75, 0.9, 0.9);
    legend->AddEntry(grRed, "First 6 points (Red)", "p");
    legend->AddEntry(grBlack, "Next 6 points (Black)", "p");
    legend->AddEntry(grBlue, "Last 6 points (Blue)", "p");
    legend->Draw();

    // Save the output
    c1->Update();
    c1->Print("Yield_1618keV_resonance.png");
}




    // // Create the first graph (columns 0, 1, 2)
    // TGraphErrors *gr1 = new TGraphErrors(energy.size(), &energy[0], &yield2[0], 0, &unc2[0]);
    // gr1->SetMarkerStyle(20);
    // //gr1->SetMarkerColor(kBlue);
    // gr1->SetTitle("1618keV_res_1081keV #gamma;Energy #it{[keV]};Yield #it{[a.u.]}");

    // // Create the first canvas and draw the first graph
    // TCanvas *c1 = new TCanvas("c1", "Peak 2539 keV", 1200, 800);
    // gr1->Draw("AP");

    // // Draw initial 6 points in blue
    // for (int i = 0; i < 6; ++i) {
    //     gr1->SetPoint(i, energy[i], yield2[i]);
    //     gr1->SetPointError(i, 0, unc2[i]);
    //     gr1->SetMarkerColor(kBlue);
    //     gr1->SetMarkerStyle(20);
    // }

    // // Draw middle 6 points in black
    // for (int i = 6; i < 12; ++i) {
    //     gr1->SetPoint(i, energy[i], yield2[i]);
    //     gr1->SetPointError(i, 0, unc2[i]);
    //     gr1->SetMarkerColor(kBlack);
    //     gr1->SetMarkerStyle(20);
    // }

    // // Draw last 6 points in red
    // for (int i = 12; i < energy.size(); ++i) {
    //     gr1->SetPoint(i, energy[i], yield1[i]);
    //     gr1->SetPointError(i, 0, unc1[i]);
    //     gr1->SetMarkerColor(kRed);
    //     gr1->SetMarkerStyle(20);
    // }

    // Fit the first graph with arctan function
    // TF1* f1 = new TF1("f1", "[0]*(atan((x-[4])/[1])-atan((x-[4]-[2])/[3]))+[5]", 1608, 1660);
    // f1->SetParName(0, "wg");
    // f1->SetParName(1, "Gamma1/2");
    // f1->SetParName(2, "dE");
    // f1->SetParName(3, "Gamma2/2");
    // f1->SetParName(4, "Res energy");
    // f1->SetParName(5, "Offset");

    // f1->SetParameter(0, 0.4);  // Amplitude (wg)
    // f1->SetParameter(1, 5.0);   // Half-width of the first arctan (Gamma1/2)
    // f1->SetParameter(2, 20.0);  // Energy difference (DE)
    // f1->SetParameter(3, 5.0);   // Half-width of the second arctan (Gamma2/2)
    // f1->SetParameter(4, 1618.0); // Resonance energy (Res Energy)
    // f1->SetParameter(5, 0.0);   // Vertical offset (Offset)


    // gr1->Fit("f1","W");
    // gr1->SetMinimum(-0.1);
    // gr1->SetMaximum(0.4);
    // gStyle->SetStatFontSize(0.02); 
    // gStyle->SetStatW(0.1);         
    // gStyle->SetStatH(0.1);
    // c1->Update();
    // c1->Print("1618keV_res_2539keV_ds.png");
//}