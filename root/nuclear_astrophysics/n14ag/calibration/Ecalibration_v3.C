#include "TFile.h"
#include "TDirectory.h"
#include "TH1F.h"
#include "TCanvas.h"
#include "TStyle.h"
#include "TF1.h"
#include "TGraph.h"
#include "TLegend.h"
#include "TTree.h"
#include "TSystem.h"
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <sys/stat.h>
#include <sys/types.h>

void Ecalibration_v3() {
    int run_number = 4444;
    std::string base_dir = "/felsdata/n14ag_Felsenkeller_2025/Rootfiles_high";
    std::string rootFilename = base_dir + "/Run" + std::to_string(run_number) + "_high.root";
    std::string output_dir = base_dir + "/Anup/Run" + std::to_string(run_number);
    std::string output_rootfile = output_dir + "/calibrated_spectra.root";

    gSystem->Exec(("mkdir -p " + output_dir).c_str());

    TFile* file = TFile::Open(rootFilename.c_str(), "READ");
    if (!file || file->IsZombie()) {
        std::cerr << "Error: Could not open file " << rootFilename << std::endl;
        return;
    }

    TFile* outFile = new TFile(output_rootfile.c_str(), "RECREATE");
    TTree* calibTree = new TTree("calibration", "Energy calibration data");

    std::vector<std::string> detector_names = {
        "EB18_A", "EB18_B", "EB18_C", "EB18_D", "EB18_E", "EB18_F", "EB18_G",
        "MB1_1", "MB1_2", "MB1_3", "MB2_1", "MB2_2", "MB2_3"
    };

    std::map<std::string, std::pair<std::pair<int, int>, std::pair<int, int>>> detector_rois = {
        {"EB18_A", {{1450, 1510}, {2620, 2700}}},
        {"EB18_B", {{1420, 1480}, {2570, 2640}}},
        {"EB18_C", {{1420, 1480}, {2570, 2630}}},
        {"EB18_D", {{1440, 1500}, {2590, 2680}}},
        {"EB18_E", {{1420, 1500}, {2580, 2660}}},
        {"EB18_F", {{1430, 1510}, {2580, 2660}}},
        {"EB18_G", {{1440, 1520}, {2610, 2690}}},
        {"MB1_1",  {{1420, 1500}, {2570, 2630}}},
        {"MB1_2",  {{1415, 1480}, {2570, 2630}}},
        {"MB1_3",  {{1420, 1500}, {2575, 2625}}},
        {"MB2_1",  {{1430, 1510}, {2590, 2670}}},
        {"MB2_2",  {{1430, 1510}, {2600, 2670}}},
        {"MB2_3",  {{1420, 1500}, {2600, 2680}}},
        {"Ron100", {{1400, 1480}, {2540, 2620}}}
    };

    int n_det = detector_names.size();
    int nx = 4;
    int ny = (n_det + nx - 1) / nx;

    TCanvas* peakCanvas_40K = new TCanvas("peakCanvas_40K", "40K Peak Fitting", 1200, 900);
    peakCanvas_40K->Divide(nx, ny);

    TCanvas* peakCanvas_Tl = new TCanvas("peakCanvas_Tl", "208Tl Peak Fitting", 1200, 900);
    peakCanvas_Tl->Divide(nx, ny);

    outFile->mkdir("Energies");
    outFile->mkdir("40K_fitting");
    outFile->mkdir("Tl_fitting");
    outFile->mkdir("Calibrations");

    double intercept, slope;
    std::string det_name;
    calibTree->Branch("detector", &det_name);
    calibTree->Branch("intercept", &intercept);
    calibTree->Branch("slope", &slope);

    for (size_t i = 0; i < detector_names.size(); ++i) {
        det_name = detector_names[i];
        TDirectory* dir = (TDirectory*)file->Get("Energies");
        if (!dir) {
            std::cerr << "Energies directory not found in input file." << std::endl;
            continue;
        }

        TH1F* ronHist = (TH1F*)dir->Get(det_name.c_str());
        if (!ronHist) {
            std::cerr << "Histogram not found for detector: " << det_name << std::endl;
            continue;
        }

        auto roi = detector_rois[det_name];
        TF1* fit_40K = new TF1(("fit_40K_" + det_name).c_str(), "gaus + pol0(3)", roi.first.first, roi.first.second);
        TF1* fit_208Tl = new TF1(("fit_208Tl_" + det_name).c_str(), "gaus + pol0(3)", roi.second.first, roi.second.second);

        ronHist->Fit(fit_40K, "RQ");
        ronHist->Fit(fit_208Tl, "RQ");

        double peak_40K = fit_40K->GetParameter(1);
        double peak_208Tl = fit_208Tl->GetParameter(1);

        TGraph* graph = new TGraph(2);
        graph->SetPoint(0, peak_40K, 1460.8);
        graph->SetPoint(1, peak_208Tl, 2614.5);
        graph->SetTitle(("Calibration: " + det_name).c_str());
        graph->GetXaxis()->SetTitle("Channel");
        graph->GetYaxis()->SetTitle("Energy (keV)");

        TF1* linearFit = new TF1(("linearFit_" + det_name).c_str(), "[0] + [1]*x", 0, 3000);
        graph->Fit(linearFit, "RQ");

        double intercept = linearFit->GetParameter(0);
        double slope = linearFit->GetParameter(1);
        calibTree->Fill();

        int nbins = ronHist->GetNbinsX();
        double eMin = intercept + slope * ronHist->GetBinCenter(1);
        double eMax = intercept + slope * ronHist->GetBinCenter(nbins);

        TH1F* energyHist = new TH1F(("energy_" + det_name).c_str(),
            (det_name + " Energy Spectrum;Energy (keV);Counts").c_str(),
            nbins, eMin, eMax);
        energyHist->Sumw2();  // Ensure proper handling of bin errors
        for (int j = 1; j <= nbins; j++) {
            energyHist->SetBinContent(j, ronHist->GetBinContent(j));
        }

        if (!outFile->cd("Energies")) {
            std::cerr << "Warning: Could not cd to Energies folder!" << std::endl;
        } else {
            energyHist->Write();
        }

        outFile->cd("40K_fitting");
        fit_40K->Write();

        outFile->cd("Tl_fitting");
        fit_208Tl->Write();

        outFile->cd("Calibrations");
        graph->Write();
        linearFit->Write();

        TH1F* hist40K = (TH1F*)ronHist->Clone(("hist40K_" + det_name).c_str());
        TH1F* histTl = (TH1F*)ronHist->Clone(("histTl_" + det_name).c_str());

        peakCanvas_40K->cd(i + 1);
        hist40K->SetLineColor(kBlue);
        hist40K->GetXaxis()->SetRangeUser(peak_40K - 50, peak_40K + 50);
        hist40K->Draw("HIST");
        fit_40K->SetLineColor(kRed);
        fit_40K->Draw("SAME");

        peakCanvas_Tl->cd(i + 1);
        histTl->SetLineColor(kBlue);
        histTl->GetXaxis()->SetRangeUser(peak_208Tl - 50, peak_208Tl + 50);
        histTl->Draw("HIST");
        fit_208Tl->SetLineColor(kGreen);
        fit_208Tl->Draw("SAME");
    }

    // Explicitly write all directories
    outFile->cd("Energies"); outFile->GetDirectory("Energies")->Write();
    outFile->cd("40K_fitting"); outFile->GetDirectory("40K_fitting")->Write();
    outFile->cd("Tl_fitting"); outFile->GetDirectory("Tl_fitting")->Write();
    outFile->cd("Calibrations"); outFile->GetDirectory("Calibrations")->Write();

    outFile->cd();
    calibTree->Write();
    outFile->Close();
    file->Close();

    // peakCanvas_40K->SaveAs((output_dir + "/40K_fits.pdf").c_str());
    // peakCanvas_Tl->SaveAs((output_dir + "/Tl_fits.pdf").c_str());

    std::cout << "Calibration and spectra writing complete." << std::endl;
}
