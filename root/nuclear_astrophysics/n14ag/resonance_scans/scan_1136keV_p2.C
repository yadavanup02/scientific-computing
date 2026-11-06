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

void scan_1136keV_p2(int start_run, int end_run) { 
    // Vectors to store data
    std::vector<double> energy, yield, uncertainty;

    // Map to store run number and corresponding beam energy, current, and real time
    std::map<int, std::tuple<double, double, double>> beam_data_map;

    // Read beam energy file
    std::ifstream energy_file("/felsdata/n14ag_Felsenkeller_2025/Anup/res1136Energy.txt");
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
    c->SetGrid();
    // gPad->SetLogy();
    // Create the graph
    TGraphErrors *gr = new TGraphErrors(energy.size(), &energy[0], &yield[0], NULL, &uncertainty[0]);
    gr->SetMarkerStyle(20);
    gr->SetMarkerColor(kBlue);
    gr->SetTitle(";#it{E}_{#alpha} [keV];Yield [uC^{-1}]");
    // gr->GetYaxis()->SetRangeUser(1e-5, 3e-1);
    gr->GetXaxis()->SetRangeUser(1125, 1185);
    gr->GetYaxis()->SetLabelSize(0.035); 
    gr->GetYaxis()->SetTitleOffset(1.5);
    gr->GetYaxis()->SetLabelFont(42);    
    // General style points.
    gStyle->SetOptStat(kFALSE);
    gr->SetMarkerStyle(kOpenSquare);
    //gStyle->SetStatFontSize(0.0005); 
    gStyle->SetStatW(0.0);         
    gStyle->SetStatH(0.0);
    gr->Draw("AP");

    // Fit the graph with arctan function
    TF1* fit = new TF1("totalFit", "[0]*(atan((x-[4])/[1]) - atan((x-[4]-[2])/[3])) + 0.000001", 1132.0, 1180);
    fit->SetParName(0, "wg");
    fit->SetParName(1, "Gamma1/2");
    fit->SetParName(2, "dE");
    fit->SetParName(3, "Gamma2/2");
    fit->SetParName(4, "Res energy");
    fit->SetParName(5, "Offset");

    fit->SetParameter(0, 0.028);   // Amplitude (wg)
    fit->SetParameter(1, 2.5);   // Half-width of the first arctan (Gamma1/2)
    fit->SetParameter(2, 37.68);  // Energy difference (DE)
    fit->SetParameter(3, 3.9);   // Half-width of the second arctan (Gamma2/2)
    fit->SetParameter(4, 1136.0); // Resonance energy (Res Energy)
    fit->SetParameter(5, 0.01);   // Vertical offset (Offset)
    gr->Fit("totalFit", "ER");
    fit->SetLineColor(kRed);
    fit->SetLineWidth(2);
    fit->Draw("same");



    // Draw half-maximum line and arrows
    double yMax1 = fit->GetMaximum(1135, 1155);
    double yMax2 = fit->GetMaximum(1150, 1175);

    double xmin = 1140;
    double xmax = 1164;
    double average = fit->Integral(xmin, xmax)/(xmax - xmin);
    std::cout << "\nAverage : " << average << std::endl;
    double E1 = fit->GetX(yMax1/2.0 , 1135, 1145);
    double E2 = fit->GetX(yMax1/2.0 , 1160 , 1175);

    std::cout << "\nE1: " << yMax1 << std::endl;
    std::cout << "\nE2: " << yMax2 << std::endl;
    double jet_thickness =  E2 - E1;
    std::cout << "\nJet thickness:: " << jet_thickness << std::endl;
    

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

    double E_res = fit->GetX(0.5*yMax1, 1135, 1160);
    double dE_res = fit->GetParameter(4);
    std::cout << "\nResonance energy: " << E_res << std::endl;
    std::cout << "\nResonance width: " << dE_res << std::endl;

    // Add FWHM text
    //TLatex *text = new TLatex(1650, yHalfMax + 0.01, Form("Jet thickness = %.2f keV", jet_thickness));
    TLatex *text = new TLatex((E1+E2)/2. - 7, yMax1 /2.0 - 0.005, Form("Jet thickness = %.2f keV", jet_thickness));
    text->SetTextSize(0.03);
    text->SetTextColor(kBlack);
    text->SetTextFont(42);
    text->Draw();
    printf("Jet thickness of the fitted function: %.3f keV\n", jet_thickness);

    // Save the plot
    TString output_dir = "/felsdata/n14ag_Felsenkeller_2025/Anup/Resonance_1136keV/";
    gSystem->mkdir(output_dir, true);
    TString output_file = output_dir + "scan_EB18_p2.pdf";
    TString output_root_file = output_dir + "scan_EB18_p2.root";
    c->SaveAs(output_file);
    c->SaveAs(output_root_file);
    c->Draw();
    c->Update();

    std::cout << "Press ENTER to exit..." << std::endl;
    std::cin.ignore();
}
