#include "TF1.h"
#include "TCanvas.h"
#include "TLine.h"
#include "TLatex.h"

void arctan_fit() {
    // Create a canvas
    TCanvas *c = new TCanvas("c", "Arctan Fit Visualization", 800, 600);

    // Define the arctan function
    TF1 *f = new TF1("f", "[0]*(atan((x-[4])/[1])-atan((x-[4]-[2])/[3]))+[5]", 1608, 1658);
    
    // Set parameter values (modify if needed)
    f->SetParameter(0, 1.0);  // Amplitude (wg)
    f->SetParameter(1, 1.2);  // Gamma1/2
    f->SetParameter(2, 30.0); // dE
    f->SetParameter(3, 2.6);  // Gamma2/2
    f->SetParameter(4, 1618.0); // Resonance energy
    f->SetParameter(5, 0.0);  // Offset

    // Draw the function
    f->SetLineColor(kBlue);
    f->SetLineWidth(2);
    f->Draw();

    // Calculate FWHM
    double E_res = f->GetParameter(4);   // Resonance energy
    double dE = f->GetParameter(2);      // Energy difference
    double Gamma1 = 2 * f->GetParameter(1); // Full width from Gamma1/2
    double Gamma2 = 2 * f->GetParameter(3); // Full width from Gamma2/2
    double FWHM = dE + (Gamma1 / 2) + (Gamma2 / 2);

    // Find half-max level
    double max_value = f->GetMaximum();
    double half_max = max_value / 2;

    // Draw a horizontal line at half max
    TLine *line = new TLine(E_res - FWHM / 2, half_max, E_res + FWHM / 2, half_max);
    line->SetLineColor(kRed);
    line->SetLineStyle(2);
    line->Draw();

    // Add FWHM text
    TLatex *text = new TLatex(E_res, half_max + 0.02, Form("FWHM = %.2f keV", FWHM));
    text->SetTextSize(0.03);
    text->SetTextColor(kRed);
    text->Draw();

    // Update the canvas
    c->Update();
}
