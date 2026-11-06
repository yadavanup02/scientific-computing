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

void scan_278keV(int start_run, int end_run) { 
    // Vectors to store data
    std::vector<double> energy, yield, uncertainty;

    // Map to store run number and corresponding beam energy, current, and real time
    std::map<int, std::tuple<double, double, double>> beam_data_map;

    // Read beam energy file
    std::ifstream energy_file("/felsdata/n14ag_Felsenkeller_2025/Anup/res278keV.txt");
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

        // Run 4417 to 4448
        TString input_file = TString::Format("/felsdata/n14ag_Felsenkeller_2025/Anup/Run%d/Peak1_LcLu_Run%d.txt", run_number, run_number);
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
                if (detector == "EB18_G") {
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
    // Create the graph
    c->SetRightMargin(0.05);
    c->SetTopMargin(0.05);
    TGraphErrors *gr = new TGraphErrors(energy.size(), &energy[0], &yield[0], NULL, &uncertainty[0]);
    gr->SetMarkerStyle(kFullCircle);
    gr->SetMarkerSize(1.0);
    gr->SetMarkerColor(kBlack);
    //gr->SetTitle("EB18_G, PT 2523 keV #gamma;#it{E}_{#alpha} [keV];Yield [uC^{-1}]");
    gr->SetTitle(";#it{E}_{#alpha} [keV];Yield [uC^{-1}]");
    gr->GetYaxis()->SetRangeUser(0.0,0.42);
    // gr->GetXaxis()->SetRangeUser(268, 296);
    gr->GetXaxis()->SetLimits(268, 296);

    gPad->Clear();
    gr->Draw("AP");
    gr->GetYaxis()->SetLabelSize(0.035); 
    gr->GetYaxis()->SetTitleOffset(1.5);
    gr->GetYaxis()->SetLabelFont(42);    
    gStyle->SetOptStat(kFALSE);
    //gStyle->SetLineScalePS(1);
    // gr->SetMarkerStyle(kOpenSquare);
    //gStyle->SetStatFontSize(0.0005); 
    gStyle->SetStatW(0.0);         
    gStyle->SetStatH(0.0);
    // gr->GetXaxis()->SetRangeUser(268, 296);
    // gr->Draw("AP");

    // Fit the graph with arctan function
    // TF1* fit = new TF1("totalFit", "[0]*(atan((x-[4])/[1]) - atan((x-[4]-[2])/[3])) + [5]", 274, 292);
    TF1* fit = new TF1("totalFit", "[0]*(atan((x-[4])/[1]) - atan((x-[4]-[2])/[3])) + (x<[6])*0.00001 + (x>=[6])*[7]", 273.6, 292.0);

    fit->SetParName(6, "TransitionX");  // X where offset changes
    fit->SetParName(7, "Offset2");  // Offset above range

    // Set initial values
    fit->SetParameter(6, 274.0);  // Example transition point
    fit->SetParameter(7, 0.0);     // Example offset after transition

    
    //TF1* fit1 = new TF1("fit1", "[0]*(atan((x-[4])/[1])) + [5]", 1610, 1630);
    //TF1* fit2 = new TF1("fit2", "[0]*(-atan((x-[4])/[3])) + [5]", 1635, 1665);
    //TF1* fit = new TF1("fit", "[0]*(atan((x-[4])/[1]))", 1132, 1155);
    fit->SetParName(0, "wg");
    fit->SetParName(1, "Gamma1/2");
    fit->SetParName(2, "dE");
    fit->SetParName(3, "Gamma2/2");
    fit->SetParName(4, "Res energy");
    fit->SetParName(5, "Offset");

    fit->SetParameter(0, 0.04);   // Amplitude (wg)
    fit->SetParameter(1, 2.0);   // Half-width of the first arctan (Gamma1/2)
    fit->SetParameter(2, 8.9);  // Energy difference (DE)
    fit->SetParameter(3, 2.1);   // Half-width of the second arctan (Gamma2/2)
    fit->SetParameter(4, 276.0); // Resonance energy (Res Energy)
    fit->SetParameter(5, 0.05);   // Vertical offset (Offset)


    // gr->Fit("fit1", "ER");
    // gr->Fit("fit2", "ER+");
    fit->SetLineColor(kOrange+7);   
    fit->SetLineWidth(2); 
    gr->Fit("totalFit", "ER");
    gPad->Update();



    // Draw half-maximum line and arrows
    double yMax1 = fit->GetMaximum(275, 290);
    // double yMax2 = fit->GetMaximum(275, 290);
    // double E1 = fit->GetX(yMax1/2.0 , 1619, 1635);
    // double E2 = fit->GetX(yMax2/2.0 , 1635 , 1660);

    // double xmin = 275;
    // double xmax = 290;
    // double average = fit->Integral(xmin, xmax)/(xmax - xmin);
    // std::cout << "\nAverage : " << average << std::endl;
    double E1 = fit->GetX(yMax1/2.0 , 275, 283);
    double E2 = fit->GetX(yMax1/2.0 , 283, 290);

    std::cout << "\nE1: " << E1 << std::endl;
    std::cout << "\nE2: " << E2 << std::endl;
    double jet_thickness =  E2 - E1;
    std::cout << "\nJet thickness:: " << jet_thickness << std::endl;

    double E_b1 = fit->GetX(0.12*yMax1, 275, 290);
    double E_b2 = fit->GetX(0.88*yMax1, 275, 290);
    double dE_beam = E_b2 - E_b1;
    printf("\nBeam spread dE_beam : %.3f keV\n", dE_beam);

    // double E_str1 = fit->GetX(0.88*yMax1, 1160 , 1183);
    // double E_str2 = fit->GetX(0.12*yMax1, 1160 , 1183);
    // double dE_strag = E_str2 - E_str1;
    // printf("\nBeam straggling dE_strag : %.3f keV\n", dE_strag);

    // double dE_total = sqrt(- dE_beam*dE_beam + dE_strag*dE_strag);
    // printf("\nTotal energy spread dE_total : %.3f keV\n", dE_total);
    
    // //std::cout << "\nStraggling: " << E_strag << std::endl;

    double E_res = fit->GetX(0.50*yMax1, 274, 284);
    double dE_err = fit->GetParError(4);

    TLatex *resonance_text = new TLatex(0.15, 0.92, Form("Resonance energy = %.3f #pm %.3f keV", E_res, dE_err));
    resonance_text->SetNDC();
    resonance_text->SetTextSize(0.03);
    resonance_text->SetTextColor(kBlack);
    resonance_text->SetTextFont(42);
    resonance_text->Draw();

    TLatex *gamma1_text = new TLatex(0.15, 0.87, Form("#Gamma_{1} = %.3f #pm %.3f keV", fit->GetParameter(1), fit->GetParError(1)));
    gamma1_text->SetNDC();
    gamma1_text->SetTextSize(0.03);
    gamma1_text->SetTextColor(kBlack);
    gamma1_text->SetTextFont(42);
    gamma1_text->Draw();


    printf("\nResonance energy: %.3f keV\n", E_res);
    printf("\nResonance energy uncertainty: %.3f keV\n", dE_err);

    //TLine *line1 = new TLine(E1, yMax1/2.0, E2, yMax2/2.0);
    TLine *line1 = new TLine(E1, yMax1/2.0, E2, yMax1/2.0);
    line1->SetLineColor(kBlue);
    line1->SetLineWidth(2);
    line1->Draw("same");

    TArrow *arrow1 = new TArrow(E1, yMax1/2., E1 + 1, yMax1/2.0, 0.015, "<|");
    arrow1->SetLineColor(kBlue);
    arrow1->Draw();

    TArrow *arrow2 = new TArrow(E2 -1 , yMax1/2.0, E2, yMax1/2.0, 0.015, "|>");
    arrow2->SetLineColor(kBlue);
    arrow2->Draw();

    // Add FWHM text
    //TLatex *text = new TLatex(1650, yHalfMax + 0.01, Form("Jet thickness = %.2f keV", jet_thickness));
    TLatex *text = new TLatex((E1+E2)/2. - 3, yMax1/2.0 - 0.018, Form("Jet thickness = %.2f keV", jet_thickness));
    text->SetTextSize(0.03);
    text->SetTextColor(kBlack);
    text->SetTextFont(42);
    text->Draw();
    printf("Jet thickness of the fitted function: %.3f keV\n", jet_thickness);
    gPad->Modified();
    gPad->Update();

    // c->Update();

    // Save the plot
    TString output_dir = "/felsdata/n14ag_Felsenkeller_2025/Anup/Resonance_278keV/";
    gSystem->mkdir(output_dir, true);
    TString output_file = output_dir + "scan278keV_EB18_G_p1_v3.pdf";
    TString output_root_file = output_dir + "scan278keV_EB18_G_p1_v3.root";
    c->SaveAs(output_file);
    c->SaveAs(output_root_file);


    // TString root_output = output_dir + "scan1136keV_MB2_Add_p2.root";
    // TFile* root_file = new TFile(root_output, "RECREATE");
    // c->Write();
    c->Draw();
    // root_file->Close();

    std::cout << "Press ENTER to exit..." << std::endl;
    std::cin.ignore();
}
