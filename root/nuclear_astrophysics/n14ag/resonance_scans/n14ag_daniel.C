void plotAnupn14ag() {

    TString fileName[3] = {"scan573keV_v2.root",  "scan1136keV_v3.root", "scan1618keV_v2.root"};

    TString plotName[3] = {"#it{E}_{#alpha} = 573 keV resonance", "#it{E}_{#alpha} = 1136 keV resonance", "#it{E}_{#alpha} = 1618 keV resonance"};

    double resoEn[3] = {573., 1136., 1618.};

    TString cName[3] = {"c8", "c", "c"};

    TH1D *histo[3];
    TFile *file[3];
    TGraphErrors *graph[3];
    TList *list[3];
    TObject *obj[3];
    TF1 *fun[3];
    double *y;
    TLatex *resoText[3];
    
    TCanvas *c1 = new TCanvas("c1", "c1", 500,300);
    c1->Divide(3,1);

    TCanvas *readCanvas[3];

    for (int i=0; i<3; i++) {
        cout << "=================== " <<fileName[i] << endl;
        file[i] = new TFile(fileName[i], "READ");
        readCanvas[i] = (TCanvas *)file[i]->Get(cName[i]);
        // readCanvas[i]->ls();
        graph[i] = (TGraphErrors *)readCanvas[i]->FindObject("Graph");
        // graph[i]->Print();
        c1->cd(i+1);
        // gPad->SetGridx();
        // gPad->SetGridy();
        gPad->SetLeftMargin(0.12);
        if (i>0) {
            gPad->SetLeftMargin(0.12);
            }
        gPad->SetRightMargin(0.02);
        gPad->SetTopMargin(0.05);
        gPad->SetBottomMargin(0.08);

        y = graph[i]->GetY();
        for (int k=0; k < graph[i]->GetN(); k++) {
            y[k] += 1.e-12;
            }
        gPad->SetLogy();
        graph[i]->Draw("ap");
        graph[i]->SetMarkerStyle(kOpenSquare);
        graph[i]->SetMarkerSize(0.5);
        graph[i]->GetHistogram()->SetTitleOffset(1.8);
        graph[i]->SetPoint(graph[i]->GetN(), resoEn[i]+70.,1.e-12);
        graph[i]->GetXaxis()->SetRangeUser(resoEn[i]-10., resoEn[i]+60.);
        graph[i]->GetXaxis()->SetTitleOffset(1.);
        graph[i]->GetYaxis()->SetTitleOffset(1.8);
        graph[i]->GetYaxis()->SetRangeUser(1.e-5,.3);
        // graph[i]->SetTitle(plotName[i]+";#it{E}_{#alpha} [keV];Yield [#muC^{-1}]");
        graph[i]->SetTitle(plotName[i]+";;");
        if (i==0) {
            graph[i]->SetTitle(";;Yield [#muC^{-1}]");
            }
        if (i==1) {
            graph[i]->SetTitle("^{14}N(#alpha,#gamma)^{18}F;#it{E}_{#alpha} [keV];");
            graph[i]->GetHistogram()->SetTitleSize(2.5,"");
            graph[i]->GetXaxis()->CenterTitle(true);
            }
        if (i==2) {
            graph[i]->SetTitle(";;");
            }

        resoText[i] = new TLatex(resoEn[i]+25.,.2,plotName[i]);
        resoText[i]->SetTextColor(kBlue);
        resoText[i]->SetTextSize(0.06);
        resoText[i]->SetTextAlign(22);
        resoText[i]->Draw();


        list[i] = (TList *)graph[i]->GetListOfFunctions();
        // list[i]->Print();
        fun[i] = (TF1 *)list[i]->FindObject("totalFit");
        // fun[i]->Print();
        fun[i]->SetLineColor(kBlue);
        // fun[i]->Draw();
        // graph[i]->GetYaxis()->SetLabelOffset(1.5);
        }

    gStyle->SetLineScalePS(1.5);
    c1->Print("plotAnupn14ag.pdf");
}