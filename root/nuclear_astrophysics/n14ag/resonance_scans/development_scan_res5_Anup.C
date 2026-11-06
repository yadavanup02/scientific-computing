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

void scan_res5_Anup(int start_run, int end_run) { 
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

        TString input_file = TString::Format("/felsdata/n14ag_Felsenkeller_2025/Anup/Run%d/Peak1_Counts_Run%d.txt", run_number, run_number);
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
            double peakE, gross, net, netErr;
            if (ss >> detector >> peakE >> gross >> net >> netErr) {
                // For this example, we'll use EB18_A detector
                if (detector == "EB18_A") {
                    double yield_value = net / (real_time * current);
                    double uncertainty_value = netErr / (real_time * current);

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
    // Create the graph
    TGraphErrors *gr = new TGraphErrors(energy.size(), &energy[0], &yield[0], NULL, &uncertainty[0]);
    gr->SetMarkerStyle(20);
    gr->SetMarkerColor(kBlue);
    gr->SetTitle("EB18_A, PT 2775 keV #gamma;#it{E}_{#alpha} [keV];Yield [uC^{-1}]");
    gr->GetYaxis()->SetRangeUser(0., 1.2e-2);
    gr->GetXaxis()->SetRangeUser(1125, 1185);
    gr->GetYaxis()->SetLabelSize(0.035); 
    gr->GetYaxis()->SetLabelFont(42);    
    //gStyle->SetStripDecimals(kFALSE);  
    gStyle->SetPaintTextFormat(".1e");  
    //gr->GetYaxis()->SetTitleOffset(1.2);
    //gr->GetYaxis()->SetTitleSize(0.03);
    //gr->GetYaxis()->SetLabelSize(0.03);
    // General style points.
    gStyle->SetOptStat(kFALSE);
    //gStyle->SetLineScalePS(1);
    //gStyle->SetPaintTextFormat("1.2e");
    //gStyle->SetMaxDigits(2);
    gr->SetMarkerStyle(kOpenSquare);
    //gStyle->SetStatFontSize(0.0005); 
    gStyle->SetStatW(0.0);         
    gStyle->SetStatH(0.0);
    gr->Draw("AP");

    // Fit the graph with arctan function
    TF1* fit = new TF1("fit", "[0]*(atan((x-[4])/[1])-atan((x-[4]-[2])/[3]))+[5]", 1132, 1178);
    fit->SetParName(0, "wg");
    fit->SetParName(1, "Gamma1/2");
    fit->SetParName(2, "dE");
    fit->SetParName(3, "Gamma2/2");
    fit->SetParName(4, "Res energy");
    fit->SetParName(5, "Offset");

    fit->SetParameter(0, 0.014);   // Amplitude (wg)
    fit->SetParameter(1, 0.6);   // Half-width of the first arctan (Gamma1/2)
    fit->SetParameter(2, 40.0);  // Energy difference (DE)
    fit->SetParameter(3, 3.4);   // Half-width of the second arctan (Gamma2/2)
    fit->SetParameter(4, 1137.0); // Resonance energy (Res Energy)
    fit->SetParameter(5, 0.0);   // Vertical offset (Offset)
    gr->Fit("fit", "E");

    // Get fitted parameters and calculate FWHM
    double Gamma1_2 = fit->GetParameter(1);
    double dE = fit->GetParameter(2);
    double Gamma2_2 = fit->GetParameter(3);
    double FWHM = dE + (Gamma1_2 / 2.0) + (Gamma2_2 / 2.0);

    printf("FWHM of the fitted function: %.3f keV\n", FWHM);

    // Draw half-maximum line and arrows
    // double yMax = fit->GetMaximum(1140, 1170);
    // double yHalfMax = yMax / 2.0;

    // TLine *line = new TLine(1140, yHalfMax, 1170, yHalfMax);
    // line->SetLineColor(kOrange);
    // line->SetLineWidth(2);
    // line->Draw("same");

    // TArrow *arrow1 = new TArrow(1140, yHalfMax, 1140, yHalfMax, 0.02, "<|");
    // arrow1->SetLineColor(kOrange);
    // arrow1->Draw();

    // TArrow *arrow2 = new TArrow(1170.5, yHalfMax, 1170.5, yHalfMax, 0.02, "|>");
    // arrow2->SetLineColor(kOrange);
    // arrow2->Draw();

    // Add FWHM text
    // TLatex *text = new TLatex(1627, yHalfMax + 0.01, Form("FWHM = %.2f keV", FWHM));
    // text->SetTextSize(0.03);
    // text->SetTextColor(kBlack);
    // text->SetTextFont(42);
    // text->Draw();

    // Save the plot
    TString output_dir = "/felsdata/n14ag_Felsenkeller_2025/Anup/Resonance_1136keV/";
    gSystem->mkdir(output_dir, true);
    TString output_file = output_dir + "EB18_A_PT_extendedROI.pdf";
    c->SaveAs(output_file);
    std::cout << "Press ENTER to exit..." << std::endl;
    std::cin.ignore();
}
