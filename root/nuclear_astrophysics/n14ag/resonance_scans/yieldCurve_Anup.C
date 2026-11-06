
// rsync -avztp yieldCurve250509.C felsdaq@fwklux10:/felsdata/n14ag_Felsenkeller_2025/Root/

void yieldCurve_Anup () {

	const int points = 17;
    const int plotpoints = 17;
    const int detectors = 4;

    TString runNames[points][4] = {
        {"Run4450_high_fromBuffer", "", "", ""},
        {"Run4451_high_fromBuffer", "Run4552_high", "Run4554_high", ""},
        {"Run4557_high", "", "", ""},
        {"Run4562_high", "", "", ""},
        {"Run4587_high", "Run4588_high", "", ""},
        {"Run4591_high", "", "", ""},
        {"Run4592_high", "", "", ""},
        {"Run4595_high", "", "", ""},
        {"Run4596_high", "", "", ""},
        {"Run4599_high", "", "", ""},
        {"Run4602_high", "", "", ""},
        {"Run4604_high", "", "", ""},
        {"Run4606_high", "Run4608_high", "", ""},
        {"Run4610_high", "", "", ""},
        {"Run4612_high", "", "", ""},
        {"Run4614_high", "", "", ""},
        {"Run4616_high", "Run4618_high", "Run4620_high", "Run4622_high"}




        };

    double uA[points][4] = {
        {23.4, 0., 0., 0.},
        {23.3, 23.8, 23.3, 0.},
        {17.3, 0., 0., 0.},
        {23.4, 0., 0., 0.},
        {16.0, 15.3, 0., 0.},
        {15.6, 0., 0., 0.},
        {21.32, 0., 0., 0.},
        {19.2, 0., 0., 0.},
        {17.88, 0., 0., 0.},
        {13.4, 0., 0., 0.},
        {15.78, 0., 0., 0.},
        {16.247, 0., 0., 0.},
        {14.845, 15.38, 0., 0.},
        {18.441, 0., 0., 0.},
        {19.485, 0., 0., 0.},
        {20.123, 0., 0., 0.},
        {18.128, 17.571, 17.08, 16.225}


        };
    double rt[points][4];

    TString detName[detectors] = {"histoEB18noDE", "histoMB1", "histoMB2", "histoCal[10]"};
    TString detShort[detectors] = {"EB18noDE", "MB1", "MB2", "Ron100"};

    TH1D *histo[points][detectors];

    double ep[points] = { 580.04, 589.69, 599.18, 562.9, 575.06, 585., 609.18, 619.18, 594.5, 605.03, 582.5, 630.5, 581.28, 625.12, 640.01, 577.5, 602.15};
    double counts[points] = {0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0.,0., 0.};
    double sqcerr[points] = {0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0.};
    double bgcnts[points] = {0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0.};
    double sqbger[points] = {0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0.};
    double uC[points] = {0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0.};
    double y[points], dy[points];

    for (int i=0; i<points; i++) {  // Data points
        for (int m=0; m<detectors; m++) {   // Detectors.
            histo[i][m] = new TH1D(Form("histo[%d][%d]",i,m), Form("#it{E}_{#alpha}=%.0f ", ep[i]) + detShort[m], 10000, 0., 20. );
            } // for (int m=0; m<detectors; m++)
        } // for (int i=0; i<points; i++)

    TFile *readFile;
    TH1D *readHist;
    TLine *roiLine[points][detectors][6];
    double roi[6] = {3.718, 3.768, 3.798, 3.842, 3.872, 3.922};
    double raw, net, neterr, c, bg, bgerr, lmax;
    TString readString = "";
    TNamed *named;

    TCanvas *c2 = new TCanvas("c2", "c2", 1920,1080);
    c2->Divide(points,detectors);

    // General style points.
    gStyle->SetOptStat(kFALSE);
    //gStyle->SetLineScalePS(1);

    // Read in data and create histograms.
    for (int i=0; i<plotpoints; i++) {  // Data points -- we currently plot only 8 data points, because we do not yet have data for the final one.
        for (int m=0; m<detectors; m++) {   // Detectors.
            for (int k=0; k<4; k++) {   // Individual runs.
                if (runNames[i][k] != "") {
                    // Open the file.
                    readFile = new TFile("../Rootfiles_high/" + runNames[i][k] + "_Tree.root" , "READ");

                    // Read histogram.
                    readHist = (TH1D *)readFile->Get("histoCal/" + detName[m]);

                    // Add the histogram to the existing one.
                    histo[i][m]->Add(readHist);

                    // Read realtime and add the charge to the total charge.
                    named = (TNamed *)readFile->Get("RunInfo/Realtime[s]");
                    sscanf(named->GetTitle(), "%lf", &rt[i][m]);
                    if (m==0) {
                        uC[i] += rt[i][m] * uA[i][m];
                        }
                    readFile->Close();
                    } // if (runNames[i][k] != "")
                } // for (int k=0; k<4; k++)

            // Draw spectrum.
            c2->cd(1 + i + points*m);
            gPad->SetLogy();
            gPad->SetGridx();
            histo[i][m]->Draw("");
            histo[i][m]->SetLineColor(kBlue);
            histo[i][m]->SetFillColor(kBlue);
            histo[i][m]->GetXaxis()->SetRangeUser(3.65,3.95);
            histo[i][m]->GetYaxis()->SetRangeUser(.9,50.);

            for (int n = 0; n < 6; n++) {
                lmax = 10.;
                if ((n==2) || (n==3)) {
                    lmax = 30.;
                    }
                roiLine[i][m][n] = new TLine(roi[n], 0.9, roi[n], lmax);
                roiLine[i][m][n] -> SetLineColor(kRed);
                roiLine[i][m][n] -> SetLineWidth(2);
                roiLine[i][m][n] -> Draw();

                } // for (int n = 0; n < 6; n++)

            // Compute integral and error.
            // c      = histo[i][m]->GetXaxis()->FindBin(roi[3]) - histo[i][m]->GetXaxis()->FindBin(roi[2]) + 1;
            bg     = histo[i][m] ->Integral(histo[i][m]->GetXaxis()->FindBin(roi[0]), histo[i][m]->GetXaxis()->FindBin(roi[1])) +
                        histo[i][m] ->Integral(histo[i][m]->GetXaxis()->FindBin(roi[4]), histo[i][m]->GetXaxis()->FindBin(roi[5])) ;
            bgerr  = sqrt(bg)/2.;
            bg     = bg / 2.;
            raw    = histo[i][m] ->Integral(histo[i][m]->GetXaxis()->FindBin(roi[2]), histo[i][m]->GetXaxis()->FindBin(roi[3]));
            net    = raw - bg;
            neterr = sqrt(raw + bgerr*bgerr);

            histo[i][m]->SetTitle(Form("#it{E}_{#alpha}=%.0f ", ep[i]) + detShort[m] + Form(" | %.1f #pm %.1f", net, neterr));

            counts[i] += net;
            sqcerr[i] += neterr*neterr;
            bgcnts[i] += bg;
            sqbger[i] += bgerr*bgerr;
            
            } // for (int m=0; m<detectors; m++)

            // Check critical limit with Gilmore eq. 5.52, k_alpha = 1.0 for 68% confidence level, taking into account that var(B) = sqbger[i] here.
            if (((counts[i] + bgcnts[i]) < sqrt(bgcnts[i] + sqbger[i])) || (counts[i] < 0.)) {
                //Put error to 68% confidence level upper limit. // Gilmore eq. 5.58
                if (counts[i] > 0.) {
                    sqcerr[i] = TMath::Power(counts[i] + sqrt(counts[i] + bgcnts[i] + sqbger[i]),2.);
                    } else {
                    sqcerr[i] = counts[i] + bgcnts[i] + sqbger[i];
                    }
                // Net counts must be zero now, to make good graphics.
                counts[i] = 0.;
                }
            
            y[i]  = counts[i] / uC[i];
            dy[i] = sqrt(sqcerr[i]) / uC[i];

            cout << Form("E_alpha=%.0f | %.1f +/- %.1f", ep[i], counts[i], sqrt(sqcerr[i])) << endl;
                        
        } // for (int i=0; i<points; i++)

    TCanvas *c8 = new TCanvas("c8", "c8", 800,600);
    c8->cd();
    gPad->SetGridx();
    gPad->SetGridy();
    gStyle->SetStatW(0.0);         
    gStyle->SetStatH(0.0);
    TGraphErrors *yieldCurve = new TGraphErrors(points, ep, y, NULL, dy);
    yieldCurve->Draw("ap*");
    yieldCurve->SetMarkerStyle(20);
    yieldCurve->SetTitle(";#it{E}_{#alpha} [keV];Yield [uC^{-1}]");
    yieldCurve->SetMarkerStyle(kOpenSquare);
    yieldCurve->SetMarkerColor(kBlue);
    yieldCurve->GetYaxis()->SetRangeUser(0., 0.27e-3);
    yieldCurve->GetYaxis()->SetLabelSize(0.035);
    yieldCurve->GetYaxis()->SetLabelFont(42);

    TF1* fit = new TF1("totalFit", "[0]*(atan((x-[4])/[1]) - atan((x-[4]-[2])/[3]))+ 0.0", 562, 640);
    fit->SetParName(0, "wg");
    fit->SetParName(1, "Gamma1/2");
    fit->SetParName(2, "dE");
    fit->SetParName(3, "Gamma2/2");
    fit->SetParName(4, "Res energy");
    fit->SetParName(5, "Offset");

    fit->SetParameter(0, 0.00010);   
    fit->SetParameter(1, 1.1);   
    fit->SetParameter(2, 40.0);  
    fit->SetParameter(3, 2.7);   
    fit->SetParameter(4, 580.7); 
    fit->SetParameter(5, 0.005);

    yieldCurve->Fit("totalFit", "ER");
    
    // Draw half-maximum line and arrows
    double yMax1 = fit->GetMaximum(562, 590);
    double yMax2 = fit->GetMaximum(595, 630);
    // double E1 = fit->GetX(yMax1/2.0 , 1619, 1635);
    // double E2 = fit->GetX(yMax2/2.0 , 1635 , 1660);

    double xmin = 590;
    double xmax = 610;
    double average = fit->Integral(xmin, xmax)/(xmax - xmin);
    std::cout << "\nAverage : " << average << std::endl;
    double E1 = fit->GetX(average/2.0 , 562, 590);
    double E2 = fit->GetX(average/2.0 , 595 , 630);

    std::cout << "\nE1: " << yMax1 << std::endl;
    std::cout << "\nE2: " << yMax2 << std::endl;
    double jet_thickness =  E2 - E1;
    std::cout << "\nJet thickness:: " << jet_thickness << std::endl;
    
    double E_b1 = fit->GetX(0.12*average, 562.0, 590.0);
    double E_b2 = fit->GetX(0.88*average, 562.0, 590.0);
    double dE_beam = E_b2 - E_b1;
    printf("\nBeam spread dE_beam : %.3f keV\n", dE_beam);

    double E_str1 = fit->GetX(0.88*average, 610.0, 640.0);
    double E_str2 = fit->GetX(0.12*average, 610.0, 640.0);
    double dE_strag = E_str2 - E_str1;
    printf("\nBeam straggling dE_strag : %.3f keV\n", dE_strag);

    double dE_total = sqrt(- dE_beam*dE_beam + dE_strag*dE_strag);
    printf("\nTotal energy spread dE_total : %.3f keV\n", dE_total);
    
    //std::cout << "\nStraggling: " << E_strag << std::endl;

    double E_res = fit->GetX(0.50*average, 562.0, 590.0);
    printf("\nResonance energy: %.3f keV\n", E_res);




    //TLine *line1 = new TLine(E1, yMax1/2.0, E2, yMax2/2.0);
    TLine *line1 = new TLine(E1, average/2.0, E2, average/2.0);
    line1->SetLineColor(kBlue);
    line1->SetLineWidth(2);
    line1->Draw("same");

    TArrow *arrow1 = new TArrow(E1, average/2., E1 + 1, average/2.0, 0.015, "<|");
    arrow1->SetLineColor(kBlue);
    arrow1->Draw();

    TArrow *arrow2 = new TArrow(E2 -1 , average/2.0, E2, average/2.0, 0.015, "|>");
    arrow2->SetLineColor(kBlue);
    arrow2->Draw();

    // Add FWHM text
    //TLatex *text = new TLatex(1650, yHalfMax + 0.01, Form("Jet thickness = %.2f keV", jet_thickness));
    TLatex *text = new TLatex((E1+E2)/2. - 13, average /2.0 - 0.00002, Form("Jet thickness = %.2f keV", jet_thickness));
    text->SetTextSize(0.03);
    text->SetTextColor(kBlack);
    text->SetTextFont(42);
    text->Draw();
    printf("Jet thickness of the fitted function: %.3f keV\n", jet_thickness);


    // Save the plot
    TString output_dir = "/felsdata/n14ag_Felsenkeller_2025/Anup/Resonance_573keV/";
    gSystem->mkdir(output_dir, true);
    TString output_file = output_dir + "scan573keV_v2_.pdf";
    c8->SaveAs(output_file);

    
    TString root_output = output_dir + "scan573keV_v2_1.root";
    TFile* root_file = new TFile(root_output, "RECREATE");
    c8->Write();
    c8->Draw();
    root_file->Close(); 

    //c2->Print("yieldCurve250516_Spectra.pdf");
    //c8->Print("yieldCurve250516.pdf");
}
