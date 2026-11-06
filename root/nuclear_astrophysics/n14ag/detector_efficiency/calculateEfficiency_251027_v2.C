#include "TFile.h"
#include "TCanvas.h"
#include "TH1F.h"
#include "TF1.h"
#include "TLine.h"
#include "TGraph.h"
#include "TLegend.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <map>
#include <vector>
#include <cmath>
#include <sys/stat.h>
#include "TMatrixDSym.h"
#include "Math/Util.h"

std::vector<int> darkColors = {
    kBlue+1,   // Dark blue
    kRed+1,    // Dark red
    kGreen+2,  // Dark green
    kMagenta+2,
    kCyan+2,
    kOrange+7, // Deeper orange
    kViolet+1,
    kTeal+3,
    kPink+9,
    kAzure+3,
    kSpring+3,
    kGray+2,
    kBlack     
};

double calculate_uncertainty(TF1* fit, TFitResultPtr fitResult, double x) {
    if (!fitResult.Get()) {
        std::cerr << "Error: No fit result available!" << std::endl;
        return 0.0;
    }
    
    // Get covariance matrix
    TMatrixDSym covMatrix = fitResult->GetCovarianceMatrix();
    // Calculate derivatives
    TVectorD derivatives(fit->GetNpar());
    for (int i = 0; i < fit->GetNpar(); i++) {
        derivatives[i] = fit->GradientPar(i, &x);
    }
    
    return sqrt(covMatrix.Similarity(derivatives));
}

// Structure to hold metadata for each source
struct SourceInfo {
    double activity_kBq;
    double branching_ratio;
    double gamma_energy_keV;
    double t_live; // Live time in seconds
};

// - 20 degree angle for the source
std::vector<std::tuple<int, std::vector<std::string>, int>> run_sources = {
    {4337, {"133Ba"}, 1},
    {4330, {"137Cs"}, 1},
    {4320, {"60Co_1"}, 1},
    {4320, {"60Co_2"}, 2},    // 4321, -15 degree for the Co peak only 
    {4333, {"88Y_1"}, 1},
    {4333, {"88Y_2"}, 2}
};

void calculateEfficiency_251027_v2() {
      // -20 degree angle for the source 
    std::map<std::string, SourceInfo> source_meta = {
        {"133Ba",  {9.519, 0.6205, 356.0, 1393.0}},
        {"137Cs",  {7.087, 0.851, 661.7, 4966.0}},
        {"88Y_1", {3.353, 0.932, 898.0, 4310.0}},
        {"60Co_1",   {18.223, 0.9985, 1173.228, 54052.0}},
        {"60Co_2",   {18.223, 0.9998, 1332.49, 54052.0}},
        {"88Y_2", {3.353, 0.992, 1836.0, 4310.0}}
        
    };
    // +20 degree angle for the source 
    // std::map<std::string, SourceInfo> source_meta = {
    //     {"137CS", {7.09, 0.85, 661.7, 4167.0}},
    //     {"60Co",  {18.22, 1.00, 1332.49, 952.0}},
    //     {"88Y_93", {3.35, 0.93, 898.0, 4177.0}},
    //     {"88Y_99", {3.35, 0.99, 1836.0, 4177.0}},
    //     {"133Ba", {9.52, 0.6205, 356.0, 6971.0}}
    // };

    std::map<std::string, std::vector<double>> detector_energies;
    std::map<std::string, std::vector<double>> detector_efficiencies;
    std::map<std::string, std::vector<double>> detector_errors;

    for (const auto& run_entry : run_sources) {
        int run_number = std::get<0>(run_entry);
        std::vector<std::string> sources = std::get<1>(run_entry);
        int peak_number = std::get<2>(run_entry);
    
        std::string input_txt = "/felsdata/n14ag_Felsenkeller_2025/Anup/Efficiency/Run" + std::to_string(run_number) + "/Peak"+std::to_string(peak_number)+"_Counts_Run" + std::to_string(run_number) + ".txt";
        std::ifstream inFile(input_txt);
        if (!inFile.is_open()) {
            std::cerr << "Error opening file: " << input_txt << std::endl;
            continue;
        }

        std::string line;
        while (getline(inFile, line)) {
            if (line.find("Detector") != std::string::npos || line.find("Run Number") != std::string::npos || line.find("---") != std::string::npos) continue;

            std::stringstream ss(line);
            std::string detector;
            double gross, net, peakE, netErr, L_u, L_c;
            ss >> detector >> peakE >> gross >> net >> netErr >> L_u >> L_c;

            for (const auto& s : sources) {
                const auto& info = source_meta[s];

                bool in_range = false;
                if (s == "137Cs" && peakE > 0.600 && peakE < 0.700) in_range = true;
                if (s == "60Co_1" && peakE > 1.170 && peakE < 1.400) in_range = true;
                if (s == "60Co_2" && peakE > 1.300 && peakE < 1.395) in_range = true;
                if (s == "88Y_1" && peakE > 0.830 && peakE < 0.950) in_range = true;
                if (s == "88Y_2" && peakE > 1.800 && peakE < 1.900) in_range = true;
                if (s == "133Ba" && peakE > 0.3 && peakE < 0.39) in_range = true;

                if (!in_range) continue;

                double eff = net / (info.activity_kBq * 1000. * info.branching_ratio * info.t_live);
                double err = netErr / (info.activity_kBq * 1000. * info.branching_ratio * info.t_live);

                detector_energies[detector].push_back(info.gamma_energy_keV);
                detector_efficiencies[detector].push_back(eff);
                detector_errors[detector].push_back(err);

                std::cout << "Run: " << run_number << " | Detector: " << detector << " | Source: " << s
                          << " | E_gamma: " << info.gamma_energy_keV
                          << " keV | Efficiency: " << eff
                          << " | Error: " << err << std::endl;
            }
        }
        inFile.close();
    }
    std::string output_dir = "/felsdata/n14ag_Felsenkeller_2025/Anup/Efficiency/";
    std::string output_file = output_dir + "minus20degree_test.txt";
    std::ofstream outFile(output_file);
    if (!outFile.is_open()) {
        std::cerr << "Error opening output file: " << output_file << std::endl;
        return;
    }
    outFile << "Detector\tE_gamma (keV)\tEfficiency\tError" << std::endl;

    for (const auto& entry : detector_energies) {
        const std::string& detector = entry.first;
        const std::vector<double>& energies = entry.second;
        const std::vector<double>& efficiencies = detector_efficiencies[detector];
        const std::vector<double>& errors = detector_errors[detector];

        for (size_t i = 0; i < energies.size(); ++i) {
            outFile << detector << "\t" << energies[i] << "\t" << efficiencies[i] << "\t" << errors[i] << std::endl;
        }
    }
    

    TCanvas* c = new TCanvas("c_eff_all", "Efficiency Curves - All Detectors", 1000, 800);
    TLegend* legend = new TLegend(0.88, 0.45, 0.99, 0.99);
    int colorIndex = 0;

    // === 1) RIGHT AFTER you create the canvas `c` ===
    TPad* pad1 = new TPad("pad1","efficiency", 0.0, 0.28, 1.0, 1.0);
    TPad* pad2 = new TPad("pad2","residuals",  0.0, 0.00, 1.0, 0.28);

    // Margins (tight shared x-axis look)
    pad1->SetBottomMargin(0.018);
    pad1->SetTopMargin(0.01);
    pad1->SetLeftMargin(0.08);
    pad1->SetRightMargin(0.01);

    pad2->SetTopMargin(0.020);
    pad2->SetBottomMargin(0.28);   // more space for x labels
    pad2->SetLeftMargin(0.08);
    pad2->SetRightMargin(0.01);

    // Log scales: same log-x as main, residuals linear y
    pad1->SetLogx(1); pad1->SetLogy(1);
    pad2->SetLogx(1); pad2->SetLogy(0);

    pad1->Draw();
    pad2->Draw();

    // you previously had: c->SetLogx(); c->SetLogy();
    // -> remove or comment those, we set logs on pads now.

    // We'll draw main stuff on pad1 first:
    pad1->cd();


    // helper to know when to initialize residual axes
    bool firstResidual = true;
    double resYmin = -3.0, resYmax = 3.0; // adjust if you prefer fractional residuals


    std::string output_file1 = output_dir + "minus20degree_2774keV_thesis.txt";
    std::ofstream outFile1(output_file1);
    if (!outFile1.is_open()) {
        std::cerr << "Error opening output file: " << output_file1 << std::endl;
        return;
    }
    outFile1 << "Detector\tE_value (keV)\tEfficiency\tError" << std::endl;
    
    for (const auto& entry : detector_energies) {
        const std::string& detector = entry.first;
        const std::vector<double>& energies = entry.second;
        const std::vector<double>& efficiencies = detector_efficiencies[detector];
        const std::vector<double>& errors = detector_errors[detector];

        int color = darkColors[colorIndex % darkColors.size()];
        colorIndex++;

        TGraphErrors* g = new TGraphErrors(energies.size(), &energies[0], &efficiencies[0], 0, &errors[0]);
        g->SetMarkerStyle(20);
        g->SetMarkerColor(color);
        g->SetLineWidth(3);
        g->SetLineColor(color);
        g->Draw(colorIndex == 1 ? "AP" : "P SAME");
       // g->SetTitle("Absolute Efficiency Curve" );
        g->GetXaxis()->SetTitle("Energy E_{#gamma} [keV]");
        g->GetYaxis()->SetTitle("#eta_{#gamma}");
        g->GetYaxis()->SetTitleOffset(0.50);
        g->SetTitle("");
        g->GetXaxis()->CenterTitle();
        g->GetYaxis()->CenterTitle();
       

        g->GetXaxis()->SetTitleSize(0.06);
        g->GetYaxis()->SetTitleSize(0.07);
        g->GetXaxis()->SetLabelSize(0.06);
        g->GetYaxis()->SetLabelSize(0.04);

        // Set x and y range
        g->GetXaxis()->SetLimits(300, 2500);
        g->SetMinimum(1e-3);                
        g->SetMaximum(0.02); 
        // gPad->SetRightMargin(0.03);
        // gPad->SetTopMargin(0.03);
        // gPad->SetBottomMargin(0.10);
        // gPad->SetLeftMargin(0.10);  
        gStyle->SetStatW(0.0);         
        gStyle->SetStatH(0.0);            
        g->GetXaxis()->SetMoreLogLabels(kTRUE); // Add more labels on log scale
        g->GetXaxis()->SetNoExponent(kTRUE); 
        // pad1->SetGrid();
        c->SetGrid();
        
        //double a = 1384.0;
        // Log-polynomial fit: log(eff) = a0 + a1*log(E) + a2*log(E)^2 + a3*log(E)^3
        // TF1* fit = new TF1(("fit_" + detector).c_str(), "exp([0] + [1]*log(x) + [2]*pow(log(x),2) + [3]*pow(log(x),3))", 330, 2000);
        TF1* fit = new TF1(("fit_" + detector).c_str(), "exp([0] + [1]*log(x) + [2]*pow(log(x),2))", 330, 2400);

        fit->SetLineColor(color);
        //g->Fit(fit, "QN");
        TFitResultPtr fitResult = g->Fit(fit, "S"); // Q, Added 'S' to store the result
        fit->Draw("SAME");
        legend->SetTextSize(0.030);
        legend->AddEntry(g, detector.c_str(), "p");

        // Hide x labels on the top pad (they'll appear in residuals panel)
        if (colorIndex == 1) {
            g->GetXaxis()->SetLabelSize(0);
            g->GetXaxis()->SetTitleSize(0);
        }

        // ---- Build residuals: pull = (data - fit)/error
        std::vector<double> x_res(energies.size()), y_res(energies.size()), ex_res(energies.size(), 0.0), ey_res(energies.size(), 0.0);

        // If you prefer fractional residuals, set usePull=false
        bool usePull = true;

        for (size_t i = 0; i < energies.size(); ++i) {
            double xi   = energies[i];
            double yi   = efficiencies[i];
            double eyi  = errors[i];
            double fval = fit->Eval(xi);

            x_res[i] = xi;

            // Compute relative residual in percent
            y_res[i] = (fval != 0.0) ? ((yi - fval) / fval) * 100.0 : 0.0;

        }

        // Draw residuals on the bottom pad
        pad2->cd();
        TGraphErrors* gRes = new TGraphErrors((int)x_res.size(), x_res.data(), y_res.data(),
                                            ex_res.data(), ey_res.data());
        gRes->SetMarkerStyle(20);
        gRes->SetMarkerSize(0.9);
        gRes->SetMarkerColor(color);
        gRes->SetLineColor(color);

        // First graph defines the residual axes
        if (firstResidual) {
            gRes->Draw("AP");
            gRes->SetTitle("");
            gRes->GetXaxis()->SetTitle("Energy E_{#gamma} [keV]");
            // gRes->GetYaxis()->SetTitle(usePull ? "Residual (data-fit)/#sigma" : "Residual (data-fit)/fit");
            gRes->GetYaxis()->SetTitle("Residual [%]");
            gRes->GetYaxis()->SetTickLength(0.015);
            gRes->GetXaxis()->SetTickLength(0.071);
            gRes->GetXaxis()->CenterTitle();
            gRes->GetYaxis()->CenterTitle();

            gRes->GetXaxis()->SetMoreLogLabels(kTRUE);
            gRes->GetXaxis()->SetNoExponent(kTRUE);

            gRes->GetXaxis()->SetTitleSize(0.11);
            gRes->GetXaxis()->SetLabelSize(0.11);
            gRes->GetYaxis()->SetTitleSize(0.11);
            gRes->GetYaxis()->SetLabelSize(0.10);
            gRes->GetYaxis()->SetTitleOffset(0.35);
            gRes->GetXaxis()->SetTitleOffset(1.1);

            gRes->GetXaxis()->SetLimits(300, 2500);
            // resYmin = -5;   // or -5 depending on fit quality
            // resYmax =  5;
            // gRes->GetYaxis()->SetRangeUser(resYmin, resYmax);
            gRes->GetYaxis()->SetRangeUser(-6, 6);
            gRes->GetYaxis()->SetNdivisions(8);
            // Force the axis to update
            gRes->GetYaxis()->SetLimits(-6, 6);

            // Zero line
            TLine* zero = new TLine(300, 0.0, 2500, 0.0);
            zero->SetLineStyle(1);
            zero->SetLineWidth(1);
            zero->SetLineColor(kGray+2);
            zero->Draw("SAME");

            // --- Compute and draw average residual uncertainty band ---
            double sumUnc = 0;
            for (double e : ey_res) sumUnc += e;
            double avgUnc = sumUnc / ey_res.size();

            // Define band limits
            double xMin = 300;
            double xMax = 2500;

            // Draw filled uncertainty band around zero
            TBox* band = new TBox(xMin, -avgUnc, xMax, avgUnc);
            band->SetFillColorAlpha(kGray, 0.3);  // light gray transparent band
            band->SetLineColor(kGray+2);
            band->Draw("same");

            firstResidual = false;
        } else {
            gRes->Draw("P SAME");
        }

        // Return to top pad for the next detector's curve
        pad1->cd();
        // //------------------------------------------------------------
        // // Build residuals: (data - fit)/fit in percent
        // //------------------------------------------------------------
        // std::vector<double> x_res(energies.size()), y_res(energies.size());
        // std::vector<double> ex_res(energies.size(), 0.0);
        // std::vector<double> ey_res(energies.size(), 0.0);

        // for (size_t i = 0; i < energies.size(); ++i) {
        //     double xi   = energies[i];
        //     double yi   = efficiencies[i];
        //     double eyi  = errors[i];
        //     double fval = fit->Eval(xi);

        //     x_res[i] = xi;
        //     y_res[i] = (fval != 0.0 ? ((yi - fval) / fval) * 100.0 : 0.0);

        //     // uncertainty (in %) for the residual band
        //     ey_res[i] = (yi != 0.0 ? (eyi / yi) * 100.0 : 0.0);
        // }


        // //------------------------------------------------------------
        // // Draw residuals on bottom pad
        // //------------------------------------------------------------
        // pad2->cd();

        // TGraphErrors* gRes = new TGraphErrors(
        //     (int)x_res.size(),
        //     x_res.data(),
        //     y_res.data(),
        //     ex_res.data(),
        //     ey_res.data()
        // );

        // gRes->SetMarkerStyle(20);
        // gRes->SetMarkerSize(0.9);
        // gRes->SetMarkerColor(color);
        // gRes->SetLineColor(color);


        // //------------------------------------------------------------
        // // First detector defines axes
        // //------------------------------------------------------------
        // if (firstResidual) {

        //     // Draw axes
        //     gRes->Draw("AP");
        //     gRes->SetTitle("");

        //     gRes->GetXaxis()->SetTitle("Energy E_{#gamma} [keV]");
        //     gRes->GetYaxis()->SetTitle("Residual [%]");

        //     gRes->GetXaxis()->SetLimits(300, 2500);
        //     gRes->GetYaxis()->SetRangeUser(-6, 6);
        //     gRes->GetYaxis()->SetNdivisions(8);

        //     // --- Zero line ---
        //     TLine* zero = new TLine(300, 0.0, 2500, 0.0);
        //     zero->SetLineStyle(1);
        //     zero->SetLineWidth(1);
        //     zero->SetLineColor(kGray+2);
        //     zero->Draw("same");

        //     //--------------------------------------------------------
        //     // Compute & draw the average uncertainty band
        //     //--------------------------------------------------------
        //     double sumUnc = 0.0;
        //     for (double u : ey_res) sumUnc += u;
        //     double avgUnc = (ey_res.empty() ? 0.0 : sumUnc / ey_res.size());

        //     TBox* band = new TBox(300, -avgUnc, 2500, avgUnc);
        //     band->SetFillColorAlpha(kGray, 0.3); // light transparent band
        //     band->SetLineColor(kGray+2);
        //     band->Draw("same");

        //     gPad->Modified();
        //     gPad->Update();

        //     firstResidual = false;
        // }
        // else {
        //     // Next detectors just add points
        //     gRes->Draw("P SAME");
        // }


        // //------------------------------------------------------------
        // // Return to top pad for next detector curve
        // //------------------------------------------------------------
        // pad1->cd();


        

        // // efficiency and uncertainty at specific energy points
        // std::vector<double> test_energies = {2774.0};
        // // std::cout << "\nEfficiency values and uncertainties for detector " << detector << ":\n";
        // for (double energy : test_energies) {
        //     double eff = fit->Eval(energy);
        //     double unc = calculate_uncertainty(fit, fitResult, energy);
        //     //double unc = calculate_uncertainty(fit, energy);
        //     std::cout << detector << "|" << energy << " keV | Efficiency = " << eff 
        //             << " ± " << unc << " (" << (unc/eff)*100 << "%)\n";
            
        //     outFile1 << detector << "\t" << energy << "\t" << eff << "\t" << unc << std::endl;
        // }

        // Increment color, skipping white (color index 10)
        color++;
        if (color == 10) color++;
    }
    // === 3) AFTER the loop finishes ===
    pad1->cd();
    legend->Draw();
    c->Update();
    c->SaveAs((output_dir + "/eff-thesis-251105_v4.pdf").c_str());

    // legend->Draw();
    // //c->SetGrid();
    // // c->SetLogx();
    // // c->SetLogy();
    // c->SaveAs((output_dir + "/eff-thesis-251027.pdf").c_str());
}
