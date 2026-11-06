#include "TFile.h"
#include "TDirectory.h"
#include "TH1F.h"
#include "TCanvas.h"
#include "TStyle.h"
#include "TF1.h"
#include "TGraph.h"
#include "TLegend.h"
#include "TTree.h"
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <sys/stat.h>
#include <sys/types.h>

void Ecalibration_v2() {
    int run_number = 4441;
    std::string rootFilename = "/felsdata/n14ag_Felsenkeller_2025/Rootfiles_high/Run" + std::to_string(run_number) + "_high.root";
    TFile* file = TFile::Open(rootFilename.c_str(), "READ");
    if (!file || file->IsZombie()) {
        std::cerr << "Error: Could not open file " << rootFilename << std::endl;
        return;
    }

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


    std::string output_file = "/felsdata/n14ag_Felsenkeller_2025/Rootfiles_high/Anup/Run" + std::to_string(run_number) + "_high/Ecalibrated.root";
    std::string output_dir = output_file.substr(0, output_file.find_last_of('/'));
    struct stat info;
    if (stat(output_dir.c_str(), &info) != 0) {
        mkdir(output_dir.c_str(), 0777);
    }
    TFile* outFile = new TFile(output_file.c_str(), "RECREATE");

    TTree* tree = new TTree("calibration_tree", "Calibration Results");
    std::string detector;
    float peak40K, energy40K, peakTl, energyTl, slope, intercept;
    tree->Branch("detector", &detector);
    tree->Branch("peak40K", &peak40K);
    tree->Branch("energy40K", &energy40K);
    tree->Branch("peakTl", &peakTl);
    tree->Branch("energyTl", &energyTl);
    tree->Branch("slope", &slope);
    tree->Branch("intercept", &intercept);

    TCanvas* canvas40K = new TCanvas("canvas40K", "40K Fits", 1600, 1200);
    canvas40K->Divide(4, 4);
    TCanvas* canvasTl = new TCanvas("canvasTl", "208Tl Fits", 1600, 1200);
    canvasTl->Divide(4, 4);

    TDirectory* dirEnergies = outFile->mkdir("Energies");
    TDirectory* dir40K = outFile->mkdir("40K_Fitting");
    TDirectory* dirTl = outFile->mkdir("208Tl_Fitting");
    TDirectory* dirCalib = outFile->mkdir("Calibration");

    for (size_t i = 0; i < detector_names.size(); ++i) {
        detector = detector_names[i];
        TH1F* hist = (TH1F*)file->Get(("Energies/" + detector).c_str());
        if (!hist) continue;

        auto roi_40K = detector_rois[detector].first;
        auto roi_Tl = detector_rois[detector].second;

        TF1* fit40K = new TF1("fit40K", "gaus + pol0(3)", roi_40K.first, roi_40K.second);
        TF1* fitTl = new TF1("fitTl", "gaus + pol0(3)", roi_Tl.first, roi_Tl.second);

        hist->Fit(fit40K, "RQ");
        hist->Fit(fitTl, "RQ");

        peak40K = fit40K->GetParameter(1);
        peakTl = fitTl->GetParameter(1);
        energy40K = 1460.8;
        energyTl = 2614.5;

        TGraph* graph = new TGraph(2);
        graph->SetPoint(0, peak40K, energy40K);
        graph->SetPoint(1, peakTl, energyTl);

        TF1* linFit = new TF1("linFit", "[0]+[1]*x", 0, 3000);
        graph->Fit(linFit, "Q");

        slope = linFit->GetParameter(1);
        intercept = linFit->GetParameter(0);

        int nbins = hist->GetNbinsX();
        double eMin = intercept + slope * hist->GetBinCenter(1);
        double eMax = intercept + slope * hist->GetBinCenter(nbins);
        TH1F* eHist = new TH1F((detector + "_calib").c_str(), "", nbins, eMin, eMax);

        for (int j = 1; j <= nbins; ++j) {
            eHist->SetBinContent(j, hist->GetBinContent(j));
        }

        // Save to output
        dirEnergies->cd(); eHist->Write();
        dir40K->cd(); hist->Write((detector + "_40K").c_str()); fit40K->Write((detector + "_fit40K").c_str());
        dirTl->cd(); hist->Write((detector + "_Tl").c_str()); fitTl->Write((detector + "_fitTl").c_str());
        dirCalib->cd(); graph->Write((detector + "_graph").c_str()); linFit->Write((detector + "_linFit").c_str());

        tree->Fill();

        canvas40K->cd(i+1); hist->Draw("HIST"); fit40K->Draw("SAME");
        canvasTl->cd(i+1); hist->Draw("HIST"); fitTl->Draw("SAME");
    }

    outFile->cd();
    tree->Write();
    outFile->Close();

    canvas40K->Update();
    canvas40K->Draw();
    canvasTl->Update();
    canvasTl->Draw();

    std::cout << "Finished processing all detectors." << std::endl;
}
