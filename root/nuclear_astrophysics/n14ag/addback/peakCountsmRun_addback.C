// Addback Tree analysis
#include "TFile.h"
#include "TTree.h"
#include "TH1F.h"
#include "TCanvas.h"
#include "TString.h"

void peakCountsmRun_addback(int start_run, int end_run) {

    TCanvas tmpCanvas("tmpCanvas", "Temporary Canvas", 800, 600);


    for (int run_number = start_run; run_number <= end_run; run_number++) {

        TString input_file = TString::Format("/felsdata/n14ag_Felsenkeller_2025/Rootfiles_high/Run%d_high_Tree.root", run_number);
        TString output_dir = TString::Format("/felsdata/n14ag_Felsenkeller_2025/Anup/Run%d/", run_number);
        TString output_rootfile = output_dir + "Addback_tree.root";
        
        TFile* readFile = new TFile(input_file, "READ");
        if (!readFile || readFile->IsZombie()) {
            std::cerr << "Error: Could not open ROOT file!" << std::endl;
            return;
        }
        TTree* felsTree = (TTree*)readFile->Get("Tree/felsTree");
        if (!felsTree) {
            std::cerr << "Error: Could not find tree!" << std::endl;
            readFile->Close();
            return;
        }
        
        gSystem->mkdir(output_dir, true);
        TFile* outFile = new TFile(output_rootfile, "RECREATE");
        TDirectory* addbackDir = outFile->mkdir("Addback");
        addbackDir->cd();
        // Create addback histogram
        TH1F* EB18Addback = new TH1F("EB18Addback", "Addback EB18;Energy (MeV);Counts", 6000, 0, 6.);
        TH1F* MB1Addback = new TH1F("MB1Addback", "Addback MB1;Energy (MeV);Counts", 6000, 0, 6.);
        TH1F* MB2Addback = new TH1F("MB2Addback", "Addback MB2;Energy (MeV);Counts", 6000, 0, 6.);
        TH1F* AddbackAll = new TH1F("AddbackAll", "Addback All;Energy (MeV);Counts", 6000, 0, 6.);
        
        
        //c1->Divide(2,2);
        //c1->cd(1);
        felsTree->Draw("(MeV00+MeV01+MeV02+MeV03+MeV04+MeV05+MeV06)>>EB18Addback", 
                    "(MeV00+MeV01+MeV02+MeV03+MeV04+MeV05+MeV06) > 0");
        felsTree->SetMarkerColor(kBlue);

        //c1->cd(2);
        felsTree->Draw("(MeV23+MeV24+MeV25)>>MB1Addback", "(MeV23+MeV24+MeV25)> 0");

        //c1->cd(3);
        felsTree->Draw("(MeV07+MeV08+MeV09)>>MB2Addback", "(MeV07+MeV08+MeV09)> 0");
        
        TCanvas* c1 = new TCanvas("c1", "Addback analysis c1", 800, 600);
        felsTree->Draw("(MeV00+MeV01+MeV02+MeV03+MeV04+MeV05+MeV06+MeV23+MeV24+MeV25+MeV07+MeV08+MeV09)>>AddbackAll", 
            "(MeV00+MeV01+MeV02+MeV03+MeV04+MeV05+MeV06+MeV23+MeV24+MeV25+MeV07+MeV08+MeV09)> 0");
        AddbackAll->SetLineColor(kBlue);
        AddbackAll->Draw();


        // Save the canvas
        c1->SaveAs(output_dir + "Addback_Tree.pdf");

        // EB18Addback->Write();
        // MB1Addback->Write();
        // MB2Addback->Write();  
        AddbackAll->Write(); 
        c1->Write();
        outFile->Close();

        // Clean up
        //readFile->Close();
        // std::cout << "\nProcessing complete! Results saved in " << output_dir << std::endl;
        std::cout << "Press ENTER to exit..." << std::endl;
        std::cin.ignore();
    
    }

}
