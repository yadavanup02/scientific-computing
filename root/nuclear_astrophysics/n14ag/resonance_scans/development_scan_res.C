
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

void scan_res(TString filename="resonance_1618keV.txt"){ 

	TGraphErrors *gr1 = new TGraphErrors(filename, "%lg %lg %lg");

/*	TF1 *f1 = new TF1("f1", "[0] * pow(1+exp(([1]-x)/[3]),-1) * pow(1+exp((x-[1]-[2])/[4]),-1) + [5]", 0, 2000);

	f1->SetParName(0, "H");
	f1->SetParName(1, "Er");
	f1->SetParName(2, "DE");
	f1->SetParName(3, "G1");
	f1->SetParName(4, "G2");
	f1->SetParName(5, "off");

	f1->SetParameter(0, 18);
	f1->SetParameter(1, 278);
	f1->SetParameter(2, 28);
	f1->SetParameter(3, 0.6);
	f1->SetParameter(4, 1.4);
	f1->FixParameter(5, 0.1);

	//f1->SetParLimits(0, 5, 12);
	//f1->SetParLimits(1, 1168, 1178);
	//f1->SetParLimits(2, 100, 300);
	//f1->SetParLimits(3, 0, 2);
	//f1->SetParLimits(4, 1, 100);
	//f1->SetParLimits(5, 0.1, 3);
	
	f1->SetLineStyle(2);
*/	
	gStyle->SetOptFit(111);

//// Fit with arctan function ////

	TF1* f1 = new TF1("f1", "[0]*(atan((x-[4])/[1])-atan((x-[4]-[2])/[3]))+[5]", 1658.3, 1712.0);

	f1->SetParName(0, "wg");
	f1->SetParName(1, "Gamma1/2");
	f1->SetParName(2, "DE");
	f1->SetParName(3, "Gamma2/2");
	f1->SetParName(4, "Res Energy");
	f1->SetParName(5, "Offset");

	//Yfit->SetParName(3,"E res");

	f1->SetParameter(0, 0.0006);
	f1->SetParameter(1, 0.03);
	f1->SetParameter(2, 12.2);
	f1->SetParameter(3, 1.4);
	f1->SetParameter(4, 1671.0);
	f1->SetParameter(5, 0.000557646);

	f1->SetLineStyle(2);

	TCanvas *c1 = new TCanvas("c1",filename,800,600);
	c1->cd();
	gPad->SetRightMargin(0.3);
	gr1->SetTitle(filename);//filename);
	gr1->GetXaxis()->SetTitle("E_{p} [keV]");
	gr1->GetYaxis()->SetTitle("Yield [a.u.]");
	gr1->SetMarkerStyle(20);
	gr1->SetMarkerSize(1);
	gr1->SetMarkerColor(1);
	gr1->SetMaximum(0.006);
	gr1->SetMinimum(0);
	gr1->Draw("AP");	
	gr1->Fit("f1","w");	
	c1->Modified();
 	c1->Update();
	c1->Print(filename + "_arctan.pdf"); 	
	c1->Modified();
 	c1->Update();
}
