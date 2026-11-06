#include "TROOT.h"
#include "TLine.h"
#include "TMath.h"
#include "TFile.h"
#include "TTree.h"
#include "TBrowser.h"
#include "TCanvas.h"
#include "TH1.h"
#include "TH2.h"
#include "TSpectrum.h"
#include "TF1.h"
#include "TStyle.h"
#include "TLine.h"
#include "TMarker.h"
#include "TGraph.h"
#include "TGraphErrors.h"
#include "TRandom.h"
#include "stdio.h"
#include "iostream"   
#include "fstream"

void scan_res_Anup(TString filename="resonance_1618keV.txt"){ 

	TGraphErrors *gr1 = new TGraphErrors(filename, "%lg %lg %lg");

	gStyle->SetOptFit(111);

	// Fit with arctan function
	//TF1* f1 = new TF1("f1", "[0]*(atan((x-[4])/[1])-atan((x-[4]-[2])/[3]))+[5]", 1650, 1720);
	TF1* f1 = new TF1("f1", "[0]*(atan((x-[4])/[1])-atan((x-[4]-[2])/[3]))", 1658, 1712);


	f1->SetParName(0, "wg");
	f1->SetParName(1, "Gamma1/2");
	f1->SetParName(2, "dE");
	f1->SetParName(3, "Gamma2/2");
	f1->SetParName(4, "Res energy");
	//f1->SetParName(5, "Offset");

	f1->SetParameter(0, 0.23);  // Amplitude (wg)
	f1->SetParameter(1, 3.0);   // Half-width of the first arctan (Gamma1/2)
	f1->SetParameter(2, 23.0);  // Energy difference (DE)
	f1->SetParameter(3, 3.0);   // Half-width of the second arctan (Gamma2/2)
	f1->SetParameter(4, 1674.0); // Resonance energy (Res Energy)
	//f1->SetParameter(5, 0.0);   // Vertical offset (Offset)

	f1->SetLineStyle(1);

	TCanvas *c1 = new TCanvas("c1",filename,800,600);
	c1->cd();
	gPad->SetRightMargin(0.3);
	// gr1->SetTitle(filename);
	gr1->GetXaxis()->SetTitle("TV [kV]");
	gr1->GetYaxis()->SetTitle("Yield [a.u.]");
	gr1->SetTitle("1618 keV Resonance");
	gr1->SetMarkerStyle(20);
	gr1->SetMarkerSize(1);
	gr1->SetMarkerColor(1);
	gr1->SetMaximum(0.3);
	gr1->SetMinimum(-0.006);
	gr1->Draw("AP");	
	gr1->Fit("f1","E");
	//gr1->Fit("f1","w");		
	// Customize the stat box
	TPaveStats *stats = (TPaveStats*)gr1->GetListOfFunctions()->FindObject("stats");
	if (stats) {
		stats->SetTextSize(0.02); // Smaller text size
		stats->SetX1NDC(0.2); // Left edge
		stats->SetX2NDC(0.25); // Right edge (closer to left edge)
		stats->SetY1NDC(0.2); // Bottom edge
		stats->SetY2NDC(0.25); // Top edge (closer to bottom edge)
	}
	c1->Modified();
 	c1->Update();
	c1->Print("1618keV_resonance_4592g_E.png"); 	
	c1->Modified();
 	c1->Update();
	// Calculate the FWHM of the fit
	TF1 *fit = gr1->GetFunction("f1");
	if (fit) {
		double sigma = fit->GetParameter(2); 
		double sigmaError = fit->GetParError(2);
		double FWHM = 2.35482 * sigma;
		double FWHMError = 2.35482 * sigmaError;
		std::cout << "FWHM of the fit: " << FWHM << " ± " << FWHMError << std::endl;
	}

}
