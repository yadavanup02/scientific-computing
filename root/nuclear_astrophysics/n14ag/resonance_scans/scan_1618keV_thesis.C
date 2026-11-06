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
#include "TMath.h"
#include "stdio.h"
#include "iostream"
#include "fstream"
#include <vector>
#include <sstream>

void scan_1618keV_thesis(int start_run, int end_run) { 
    // Vectors to store data
    // range (4367, 4314) for 1618 keV
    std::vector<double> energy, yield, uncertainty;

    // Map to store run number and corresponding beam energy, current, and real time
    std::map<int, std::tuple<double, double, double>> beam_data_map;

    // Read beam energy file
    std::ifstream energy_file("/felsdata/n14ag_Felsenkeller_2025/Anup/res1618Energy.txt");
    if (!energy_file.is_open()) {
        std::cerr << "Error opening beam energy file." << std::endl;
        return;
    }

    std::string line;
    while (std::getline(energy_file, line)) {
        if (line.find("Run Number") != std::string::npos || line.find("Beam Energy") != std::string::npos) {
            continue; // Skip header
        }

        std::stringstream ss(line);
        int run_number;
        double beam_energy, current, real_time;
        if (ss >> run_number >> beam_energy >> current >> real_time) {
            beam_data_map[run_number] = std::make_tuple(beam_energy, current, real_time);
        }
    }
    energy_file.close();

    // Loop over run numbers
    for (int run_number = start_run; run_number <= end_run; run_number++) {
        // Get beam data for this run
        if (beam_data_map.find(run_number) == beam_data_map.end()) {
            std::cerr << "Warning: Run number " << run_number << " not found in beam energy file. Skipping." << std::endl;
            continue;
        }
        double beam_energy = std::get<0>(beam_data_map[run_number]);
        double current = std::get<1>(beam_data_map[run_number]);
        double real_time = std::get<2>(beam_data_map[run_number]);

        TString input_file = TString::Format("/felsdata/n14ag_Felsenkeller_2025/Anup/Run%d/AddbackPeak2_Counts_Run%d.txt", run_number, run_number);
        std::ifstream infile(input_file.Data());

        if (!infile.is_open()) {
            std::cerr << "Error opening file: " << input_file.Data() << std::endl;
            continue;
        }

        while (std::getline(infile, line)) {
            // Skip header lines
            if (line.find("Detector") != std::string::npos || 
                line.find("Run Number") != std::string::npos || 
                line.find("---") != std::string::npos) {
                continue;
            }

            std::stringstream ss(line);
            std::string detector;
            double peakE, gross, net, netErr, L_u, L_c;
            if (ss >> detector >> peakE >> gross >> net >> netErr) {
                if (detector == "EB18") {
                    double yield_value = 0;
                    double uncertainty_value = 0;

                    // double B = gross - net;
                    // double C_l = 1.0 * sqrt(B)


                    if (net < 0) {
                        yield_value = 0;
                        uncertainty_value = L_u/(real_time * current);
                    } else {
                        yield_value = net / (real_time * current);
                        uncertainty_value = netErr / (real_time * current);
                    }

                    energy.push_back(beam_energy);
                    yield.push_back(yield_value);
                    uncertainty.push_back(uncertainty_value);
                }
            }
        }
        infile.close();
    }
    
    printf("Number of data points: %zu\n", energy.size());
    // Create canvas and draw the graph
    TCanvas *c = new TCanvas("c", "Resonance Scan", 800, 600);
    // c->SetGrid();
    // gPad->SetLogy();
    // Create the graph
    TGraphErrors *gr = new TGraphErrors(energy.size(), &energy[0], &yield[0], NULL, &uncertainty[0]);
    // gPad->SetLeftMargin(0.15); // Increase left margin for Y-axis title
    // gPad->SetBottomMargin(0.15); // Increase bottom margin for X-axis title
    gPad->SetRightMargin(0.02); // Right margin
    gPad->SetTopMargin(0.3); // Top margin
    gStyle->SetTickLength(0.02, "X");
    gStyle->SetTickLength(0.02, "Y");
    gr->SetMarkerStyle(20);

    gr->SetMarkerSize(1.0);
    gr->SetMarkerColor(kMagenta+2);
    gr->SetLineColor(kMagenta+2); 
    //gr->SetTitle("EB18_G, PT 2523 keV #gamma;#it{E}_{#alpha} [keV];Yield [uC^{-1}]");
    gr->SetTitle(";#it{E}_{#alpha} [keV];Yield [uC^{-1}]");
    gr->GetYaxis()->SetRangeUser(-0.01, 0.9);
    gr->GetXaxis()->SetRangeUser(1605, 1670);
    gr->GetYaxis()->SetLabelSize(0.035); 
    gr->GetYaxis()->SetLabelFont(42);    
    //gStyle->SetStripDecimals(kFALSE);
    gr->SetTitle(";;Yield [#muC^{-1}]");
    gr->GetXaxis()->SetTitle("#it{E_{#alpha}} [keV]");
    gr->GetYaxis()->SetTitle("Yield [#muC^{-1}]");
    gr->GetXaxis()->SetTitleOffset(1.2);

    // Center axis titles
    gr->GetXaxis()->CenterTitle(true);
    gr->GetYaxis()->CenterTitle(true);
    gStyle->SetPaintTextFormat(".1e");
    gStyle->SetOptStat(kFALSE);
    // gr->SetMarkerStyle(kOpenSquare);
    //gStyle->SetStatFontSize(0.0005); 
    gStyle->SetStatW(0.0);         
    gStyle->SetStatH(0.0);
    gr->Draw("AP");

    // Fit the graph with arctan function
    // TF1* fit = new TF1("totalFit", "[0]*(atan((x-[4])/[1]) - atan((x-[4]-[2])/[3]))+ [5]", 1614, 1662.0);
    // fit->SetMinimum(0.005);
  
    TF1* fit = new TF1("totalFit", "[0]*(atan((x-[4])/[1]) - atan((x-[4]-[2])/[3])) + (x<[6])*0.00005 + (x>=[6])*[7]", 1614.0, 1662.0);

    // Add two new parameters:
    fit->SetParName(5, "Offset1");  // Offset below range
    fit->SetParName(6, "TransitionX");  // X where offset changes
    fit->SetParName(7, "Offset2");  // Offset above range

    // Set initial values
    fit->SetParameter(6, 1620.0);  // Example transition point
    fit->SetParameter(7, 0.0);     // Example offset after transition
    // Modify your existing fit to constrain it to be non-negative
    // TF1* fit = new TF1("totalFit", "TMath::Max(0, [0]*(atan((x-[4])/[1]) - atan((x-[4]-[2])/[3]))+ [5])", 1610., 1670.0);
    // Use an exponential function to ensure positivity
    // TF1* fit = new TF1("totalFit", "exp([0])*(atan((x-[4])/[1]) - atan((x-[4]-[2])/[3])) + [5]", 1610., 1670.0);

    fit->SetParName(0, "wg");
    fit->SetParName(1, "Gamma1/2");
    fit->SetParName(2, "dE");
    fit->SetParName(3, "Gamma2/2");
    fit->SetParName(4, "Res energy");
    fit->SetParName(5, "Offset");
    // Set initial parameter values
    fit->SetParameter(0, 0.285);   // Amplitude (wg)
    fit->SetParameter(1, 0.4);   // Half-width of the first arctan (Gamma1/2)
    fit->SetParameter(2, 31.9);  // Energy difference (DE)
    fit->SetParameter(3, 4.6);   // Half-width of the second arctan (Gamma2/2)
    fit->SetParameter(4, 1618.5); // Resonance energy (Res Energy)
    fit->SetParameter(5, 0.001);   // Vertical offset (Offset)

    fit->SetLineColor(kGray+2);
    fit->SetLineWidth(2);

    // gr->Fit("totalFit", "ERW", "", 1615, 1662);
    gr->Fit("totalFit", "ERW");



    // Draw half-maximum line and arrows
    double yMax1 = fit->GetMaximum(1616, 1635);
    double E1 = fit->GetX(yMax1/2.0 , 1617, 1635);
    double E2 = fit->GetX(yMax1/2.0 , 1635 , 1665);

    std::cout << "\nE1: " << E1 << std::endl;
    std::cout << "\nE2: " << E2 << std::endl;
    double jet_thickness =  E2 - E1;
    std::cout << "\nJet thickness:: " << jet_thickness << std::endl;

    double E_b1 = fit->GetX(0.12*yMax1, 1617, 1635);
    double E_b2 = fit->GetX(0.88*yMax1, 1617, 1635);
    double dE_beam = E_b2 - E_b1;
    printf("\nBeam spread dE_beam : %.3f keV\n", dE_beam);

    double E_str1 = fit->GetX(0.88*yMax1, 1635 , 1658);
    double E_str2 = fit->GetX(0.12*yMax1, 1635 , 1658);
    double dE_strag = E_str2 - E_str1;
    printf("\nBeam straggling dE_strag : %.3f keV\n", dE_strag);

    double dE_total = sqrt(- dE_beam*dE_beam + dE_strag*dE_strag);
    printf("\nTotal energy spread dE_total : %.3f keV\n", dE_total);
    
    //std::cout << "\nStraggling: " << E_strag << std::endl;

    double E_res = fit->GetX(0.50*yMax1, 1617, 1635);
    printf("\nResonance energy: %.3f keV\n", E_res);
    

    //TLine *line1 = new TLine(E1, yMax1/2.0, E2, yMax2/2.0);
    TLine *line1 = new TLine(E1, yMax1/2.0, E2, yMax1/2.0);
    line1->SetLineColor(kBlack);
    line1->SetLineWidth(1);
    line1->Draw("same");

    TArrow *arrow1 = new TArrow(E1, yMax1/2., E1 + 1, yMax1/2.0, 0.015, "<|");
    //TArrow *arrow1 = new TArrow(E1, yMax1/2., E1 + 1, yMax1/2.0, 0.015, "<|");

    arrow1->SetLineColor(kBlack);
    arrow1->Draw();

    TArrow *arrow2 = new TArrow(E2 -1 , yMax1/2.0, E2, yMax1/2.0, 0.015, "|>");
    //TArrow *arrow2 = new TArrow(E2 -1 , yMax2/2.0, E2, yMax2/2.0, 0.015, "|>");

    arrow2->SetLineColor(kBlack);
    arrow2->Draw();

    // Add FWHM text
    //TLatex *text = new TLatex(1650, yHalfMax + 0.01, Form("Jet thickness = %.2f keV", jet_thickness));
    // TLatex *text = new TLatex((E1+E2)/2.0 - 7, yMax1/2.0 - 0.05, Form("#Delta E = %.1f keV", jet_thickness));
    // text->SetTextSize(0.03);
    // text->SetTextColor(kBlack);
    // text->SetTextFont(42);
    // text->Draw();
    printf("Jet thickness of the fitted function: %.3f keV\n", jet_thickness);

    double resoEn = 1618.0;
    /// jet width
    double DE = E2 - E1;
    double dE = fit->GetParError(2); // Error in jet width
    TLatex *Label3 = new TLatex(0.37, 0.43, Form("#DeltaE = %.1f #pm %.1f keV", DE, dE));
    Label3->SetNDC();
    Label3->SetTextFont(42); // Use a standard font
    Label3->SetTextColor(kBlack);
    Label3->SetTextSize(0.035);
    Label3->Draw();

    TArrow *arrowLine2 = new TArrow(E_res, 0.82, E_res, 0.4, 0.010, "|>");
    arrowLine2->SetLineColor(kBlack);
    arrowLine2->SetLineWidth(1.7);
    arrowLine2->SetAngle(30); // Arrowhead opening angle
    arrowLine2->SetFillColor(kBlack);
    arrowLine2->Draw();
    // Add a label for the resonance energy
    TLatex *Label2 = new TLatex(0.18, 0.92, Form("E_{#it{#alpha}} = %.1f #pm %.1f keV", E_res, dE));
    Label2->SetNDC();
    Label2->SetTextFont(42); // Use a standard font
    Label2->SetTextColor(kBlack);
    Label2->SetTextSize(0.035);
    Label2->Draw();

    // graph->GetXaxis()->SetLimits(1618-20., 1680+60.);
    // graph->GetHistogram()->SetMinimum(1e-5);
    // graph->GetHistogram()->SetMaximum(4.0);
    // graph->GetHistogram()->SetTitleOffset(1.5, "Y");
    // gPad->SetTopMargin(0.0);
    // TLatex *label = new TLatex(0.18, 0.93, "^{14}N(#alpha, #gamma)^{18}F");
    // label->SetNDC();
    // label->SetTextSize(0.05);
    // label->SetTextFont(42);
    // label->Draw();
    // Draw a vertical line with an arrow at the top
    TArrow *arrowLine = new TArrow(resoEn, 0.05, resoEn, 0.27, 0.010, "|>");
    arrowLine->SetLineColor(kBlack);
    arrowLine->SetLineStyle(2);
    arrowLine->SetLineWidth(1.7);
    arrowLine->SetAngle(30); // Arrowhead opening angle
    arrowLine->SetFillColor(kBlack);
    arrowLine->Draw();

    TLatex *resLabel0 = new TLatex(0.28, 0.15, "E_{#it{lit}} = 1618 #pm 2 keV");
    resLabel0->SetNDC();
    resLabel0->SetTextFont(42); // Use a standard font
    resLabel0->SetTextColor(kBlack);
    resLabel0->SetTextSize(0.035);
    resLabel0->Draw();

    // legend
    TLegend *legend = new TLegend(0.87, 0.86, 0.97, 0.96);
    gPad->SetTopMargin(0.03);
    legend->SetBorderSize(1);
    legend->SetFillStyle(0); // Transparent background
    legend->SetTextSize(0.04);
    legend->SetTextFont(42); // Use a standard font
    legend->AddEntry(gr, "Data", "p");
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

    
    // Save the plot
    TString output_dir = "/felsdata/n14ag_Felsenkeller_2025/Anup/Resonance_1618keV/";
    gSystem->mkdir(output_dir, true);
    TString output_file = output_dir + "scan_EB18_p2_thesis.pdf";
    TString output_root_file = output_dir + "scan_EB18_p2_thesis.root";
    c->SaveAs(output_root_file);

    c->SaveAs(output_file);



    std::cout << "Press ENTER to exit..." << std::endl;
    std::cin.ignore();
}
