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

void resonanceStrength_v1(int start_run, int end_run) { 
    // Vectors to store data
    std::vector<double> resonance_strength, error;

    // Loop over run numbers
    for (int run_number = start_run; run_number <= end_run; run_number++) {
        TString input_file = TString::Format("/felsdata/n14ag_Felsenkeller_2025/Anup/Run%d/Peak1_LcLu_Run%d.txt", run_number, run_number);
        std::ifstream infile(input_file.Data());

        if (!infile.is_open()) {
            std::cerr << "Error opening file: " << input_file.Data() << std::endl;
            continue;
        }
        std::string line;
        while (std::getline(infile, line)) {
            // Skip header lines
            if (line.find("Detector") != std::string::npos || 
                line.find("Run Number") != std::string::npos ||  
                line.find("---") != std::string::npos) {
                continue;
            }

            std::stringstream ss(line);
            std::string detector;
            double peakE, gross, net, netErr, L_u, L_c;
            if (ss >> detector >> peakE >> gross >> net >> netErr) {
                if (detector == "EB18_F") {

                    double N_max = net;
                    double N_err = netErr;
                    // double neta = 0.00349251; 	   // exprimetal
                    // double neta_err = 3.19665e-06;
                    // double neta = 0.00317099; 	         //  Simulated EB18_G	2774.5	0.00317099	6.44872e-06
                    // double neta_err = 6.44872e-06;
                    double neta = 0.00194813; 	         //  Simulated EB18_F  2775.5  0.00194813	5.05128e-06
                    double neta_err = 5.05128e-06;
                    double Branching_ratio = 0.783;
                    double W = 1.0;                          // angular correction factor
                    double epsilon = 43.569063e-15;             //3.40084e-14;           // ev cm^2/ atoms
                    double M_14N = 14.003074;
                    double M_4He = 4.002603;
                    double COM_factor = M_14N / (M_14N + M_4He);
                    double E_r = 1136.0* 1e3;                        //resonance energy in eV
                    double I = 24.874e-6;                        // beam current in A  // 4443	
                    double live_time = 50465.31;                     // in seconds    


                    // The effective stopping power of the 14N target (Iliadis  Eq. 4.94) also from the Example 4.3 and 4.4
                    double epsilon_effective = COM_factor * epsilon * (1 + 0.3663 / 99.6337);

                    // The de Broglie wavelength from Iliadis Eq. 4.107
                    double lambda_sqr = std::pow((1.0 / COM_factor), 2 ) * 4.125e-18 / (M_4He * E_r );    // unit cm^2

                    // The total number of incident alpha particles (assuming positively charged alpha beam, q =1) amount to
                    double N_alpha = I * live_time / (1.0 * 1.6e-19);   // in number of particles

                    // ressonance strength from Iliadis Eq. 4.125 

                    double wg =  ( epsilon_effective / (lambda_sqr/ 2.0)) * ( N_max / (N_alpha * Branching_ratio * neta * W));
                    double wg_err = wg * std::sqrt( std::pow((N_err / N_max), 2) + std::pow((neta_err / neta), 2) + std::pow(0.01, 2) ); // 1% error in the beam current
                    //energy.push_back(beam_energy);
                    resonance_strength.push_back(wg);
                    error.push_back(wg_err);
                }
            }
        }
        infile.close();
        // Print the results
        for (size_t i = 0; i < resonance_strength.size(); ++i) {
            std::cout << "Run Number: " << run_number << ", Resonance Strength: " << resonance_strength[i] 
                      << " ± " << error[i] << std::endl;
        }
        // write the results in the text file
        std::ofstream output_file(TString::Format("/felsdata/n14ag_Felsenkeller_2025/Anup/Resonance_1136keV/Run%d/Resonance_Strength_v1.txt", run_number).Data(), std::ios::app);
        if (output_file.is_open()) {
            for (size_t i = 0; i < resonance_strength.size(); ++i) {
                output_file << "Run Number: " << run_number << ", Resonance Strength: " << resonance_strength[i] 
                            << " ± " << error[i] << std::endl;
            }
            output_file.close();
        } else {
            std::cerr << "Error opening output file!" << std::endl;
        }
    }
    

    std::cout << "Press ENTER to exit..." << std::endl;
    std::cin.ignore();
}
