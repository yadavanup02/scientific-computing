#define __STDC_LIMIT_MACROS
#include <iostream>
using namespace std;
#include <fstream>
#include "stdio.h"
#include <string>
#include <iomanip>
#include "TLegend.h"
#include "TCanvas.h"
#include "TPad.h"
#include "TTree.h"
#include "TH1D.h"
#include "TH2D.h"
#include "TLegend.h"
#include "TFile.h"
#include "TStyle.h"
#include "TRandom3.h"
#include "TDatime.h"
#include "TLine.h"
#include "TArrow.h"
#include "TSystem.h"
#include "TString.h"
#include <TUUID.h>
#include "TLegend.h"
#include "TPaveStats.h"
#include "TF1.h"
#include "TFitResult.h"
#include "TLatex.h"
#include "TMath.h"
#include "TStopwatch.h"
#include "THStack.h"
#include "TNamed.h"
#include "TGraph.h"
#include "TROOT.h"
#include "TApplication.h"

#define ROIS 4
#define MAXRONLINES 1000000
#define nentriesMAX 1000000000
#define digi 32
#define TWO62 2147483648*2147483648
// #define TWOSIXTY (1 << 61) - 1

// rsync -avztp ~/Experiments/n14ag/Root/rootK40*.* felsdaq@fwklux10:/felsdata/n14ag_Felsenkeller_2025/Root/

// 26.03.2025 -- removed EB17 and Ort90 -- simply edited the number of pads.
// 07.04.2025 -- updated to run also for DAQ1.
// 14.04.2025 -- added EB17

void rootK40_DAQ2_250428_Tree(
    TString runName = "Run4422",
    double Ep = 0.,
    int outK40 = 1,
    int outRoot = 2,        // 1= output ROOT file -- 2= output ROOT Tree
    TString setupName = "sinceRun4246",
    TString prefixPath = "/felsdata/n14ag_Felsenkeller_2025/Runs_DAQ2/",
    double startSeconds = 0.,
    double stopSeconds = 1.e8,
    // TString setupName = "since_Run1572",
    int daq = 2,        // DAQ1 == 1, DAQ2 == 2
    TString eCalFileName = "/felsdata/h2pg_Felsenkeller/Rootfiles_high/Daniel/Run1617_high_230427_0732.root",
    int eCal = 1,       // Number of energy calibration parameters
    double tDiff = 5.e-6,
    // TString prefixPath = "./"
    // TString prefixPath = "/net/cns/projects/astro/Felsenkeller/Runs/"
    int debugPlot = 0
    ){

    // Longer backlook if no beam
    double backlookSeconds = 3600.*3.;
    if ((Ep >= 0.2) || (outK40 == 0)) {
        backlookSeconds = 3600. * 168.;
        }

    // Kanalbelegung siehe: Steffen Meeting 04.07.2022
    // Update 15.07.2023
    TString channel[digi];
    TString channelName[digi] = {
        "EB18/A",   "EB18/B",   "EB18/C",   "EB18/D",   "EB18/E",   "EB18/F",   "EB18/G",   "MB2/1",
        "MB2/2",    "MB2/3",    "Ron100",   "Ron100/1",    "Ron100/2",    "Ort90",    "Ron104BGO", "frei",
        "EB17/A",   "EB17/B",   "EB17/C",   "EB17/D",   "EB17/E",    "EB17/F",   "EB17/G",  "MB1/1",
        "MB1/2",    "MB1/3",    "MB1/BGO",  "Cu3",     "CuCo",     "Ron100/3",      "CuTa",     "Ron100/4"};

    int channelHPGe[digi] = {
        1,    1,    1,    1,    1,    1,    1,    1,
        1,    1,    1,    1,    1,    1,    2,    0,
        1,    1,    1,    1,    1,    1,    1,    1,
        1,    1,    2,    0,    0,    1,    0,    1};

    double defaultK40[digi] = { // value 1.46082 means no calibration wanted.
        1460.,  1460.,  1460.,  1460.,  1460.,  1460.,  1460.,  1460.,
        1460.,  1460.,  1460.,  1460.,  4995.,  1464.,  0.,     0.,
        1460.,  1460.,  1460.,  1460.,  1460.,  1460.,  1460.,  1448.,
        1448.,  1462.,  0.,     0.,     0.,     4975.,  0.,     5029.};
        
    double defaultK40low[digi] = { // value 1.46082 means no calibration wanted.
        4900.,  4900.,  2701.,  5380.,  4900.,  4900.,  5060.,  5135.,
        4900.,  5660.,  4900.,  4900.,  4995.,  1464.,  0.,     0.,
        4900.,  4900.,  4900.,  4900.,  4900.,  4900.,  4900.,  4900.,
        4900.,  4900.,  0.,     0.,     0.,     4975.,  0.,     5029.};
        
    if (daq == 1) {  // default calibration for DAQ1.
        eCal = 1;
        for (int i=0; i<digi; i++) {
            defaultK40[i] = defaultK40low[i];
            } // for (int i=0; i<digi; i+)
        } // if (eCal == 101)

    /*
    const int pads = 22;
    int cd[pads] = {
        0,  1,  2,  3,  4,  5,  6,
        23, 24, 25, 7,  8,  9,  13,
        16, 17, 18, 19, 20, 21, 22,
        10
        };
        // 1. Zeile = EB18
        // 2. Zeile = MB1, MB2, Can60
        // 3. Zeile = EB17
        // 4. Zeile = Ron100
    const int pads = 14;
    int cd[pads+8] = {
        0,  1,  2,  3,  4,  5,  6,
        23, 24, 25, 7,  8,  9,  10,
        0,  0,  0,  0,  0,  0,  0,  0
        };
        // 1. Zeile = EB18
        // 2. Zeile = MB1, MB2, Ron100
    */
    int pads = 21;
    // Number of pads in display is adjusted - if EB17 is off, we don't show it.
    if (daq <= 2) {pads = 14;}
    int cd[22] = {
        0,  1,  2,  3,  4,  5,  6,
        23, 24, 25, 7,  8,  9,  10,
        16, 17, 18, 19, 20, 21, 22,
        0
        };
        // 1. Zeile = EB18
        // 2. Zeile = MB1, MB2, Ron100
        // 3. Zeile = EB17

    const int eCalMax = 4;
    if (eCal > eCalMax) {
        eCal = eCalMax;
        }
    // int eCal = 2;
        // The parameter eCal tells the script how many calibration lines should be used.
        // eCal = 1 -> use only K-40
        // eCal = 2 -> use K-40 and Tl-208
        // see eEcal below for the list of lines.
    double channelEcal[digi][eCalMax];
    double eEcal[eCalMax] = {1.46082, 2.6145, 0.609, 6.130}, a=0., b=0.;

    // for (int i=0; i<digi; i++) { cout << i << " " << channelName[i] << " " << channelHPGe[i] << endl; }

    // Variable definitions.
    FILE *fin[1000];
    short int det = 0;
    Long_t id=0,size=0,flags=0,modtimel=0,daqtimel;
    TDatime daqTime = TDatime();
    TDatime staTime = TDatime();
    TDatime modTime = TDatime();
    UInt_t firstActiveModTimeInt = 0;
    TString firstActiveModTime = "";
    TString modTimeString = "";

    Long_t q_cuta = 0, q_cuco = 0, q_cu3 = 0;

    TStopwatch *rootTime = new TStopwatch();
    int i,j,k,m,p,line=0,first,second;
    rootTime->Start();
    double time_ratio = 0, reso=0.;

    int runFiles = 1;

    // Variable definitions by Steffen.
    uint16_t headerN = 0;   // UM5960_Compass_UserManual_rev20.pdf p. 67 this has been added!
    int16_t boardN = 0;     // UM5960_Compass_UserManual_rev12.pdf p. 63 okay!
    int16_t channelN = 0;   // UM5960_Compass_UserManual_rev12.pdf p. 63 okay!
    
    uint64_t readTimeTag[1]; // UM5960_Compass_UserManual_rev12.pdf p. 63 okay!
    
    int16_t readEnergy[1];  // UM5960_Compass_UserManual_rev12.pdf p. 63 okay!
    int headersize=0, wrapped=0;
    
    int32_t readExtras[1];  // UM5960_Compass_UserManual_rev12.pdf p.63 it is called "flags"
    UInt_t header[1];
    
    int32_t numSamples = 0; // UM5960_Compass_UserManual_rev12.pdf p. 63 okay!
    int16_t samples = 0;

    long int linesRead = 0;
    TH1D *histoRaw[digi];
    TH1D *histoRate[digi];
    TH2D *histo2[digi];
    TH1D *histoTillEcal[digi];
    int histoChannels = 16384; // 32768;
    int firstActiveChannelRead = -1 ;
    TF1 *calTF1[digi];
    TGraph *calGraph[digi];
    double xbins[histoChannels+1];
    double xdefault[1] = {-1.}, ydefault[1] = {-1.};

    // General cosmetics.
    gStyle->SetOptStat(1001111);
    gStyle->SetOptFit(0);
    gStyle->SetStripDecimals(kFALSE);
    gStyle->SetLineScalePS(1);

    
    // Default energy calibration, just for safety. Linear 1:1 calibration.
    for (int i=0; i<digi; i++) {
        calTF1[i] = new TF1(Form("calTF1[%02d]",i),"[0] + [1] * x + [2] * x * x",0.,histoChannels);
        calTF1[i]->SetParameters(0., 1., 0.);
        calGraph[i] = new TGraph(1,xdefault,ydefault);
        }

    // Read in energy calibration, if there is a valid path.
    if ((eCal < 0) && (gSystem->GetPathInfo(eCalFileName,&id,&size,&flags,&daqtimel)==0)) {
        TFile *eCalFile = new TFile(eCalFileName,"READ");
        // gROOT->cd();
        cout << "===============================================================================" << endl;
        cout << "Ecal read in from " << eCalFileName << " : ";
        for (int i=0; i<pads; i++) {
            calTF1[cd[i]] = (TF1 *)eCalFile->Get(Form("calTF1[%02d]",cd[i]));
            if  (calTF1[cd[i]]->GetParameter(1) != 1.) {
                defaultK40[cd[i]] = calTF1[cd[i]] -> GetX(eEcal[0],500,histoChannels,1.e-10,100,kFALSE);
                } else {
                defaultK40[cd[i]] = 1.46082;
                calTF1[cd[i]] = new TF1(Form("calTF1[%02d]",cd[i]),"[0] + [1] * x + [2] * x * x",0.,histoChannels);
                calTF1[cd[i]]->SetParameters(0., 1., 0.);
                }
            cout << channelName[cd[i]] << " [" << cd[i] << "] " << defaultK40[cd[i]] << " ";
            }
        cout << "===============================================================================" << endl;
        cout << endl;
        }


    // Create Root file.
    TString rootFileName = "";
    // TFile *rootFile = new TFile(runName + Form("_%.0fs.root",backlookSeconds),"RECREATE");


    // Preliminary data file creation.
    TFile *rootPrelimFile;
    if (outRoot == 2) {
        rootPrelimFile = new TFile("/felsdata/n14ag_Felsenkeller_2025/Rootfiles_high/Daniel/"+runName+"_prelim.root", "RECREATE");
        } else {
        // Make sure that the prelimDataTree does not end up in the ROOT file and inflates its size.
        // gROOT->cd();
        }

    // Running time in hours.
    double runHours = 100.;
    
    TString runFileName[1000];

    // Variables for the preliminary data tree.
    unsigned short detector = 0;
    unsigned short energy = 0;
    int32_t extras = 0;
    uint64_t timeTag = 0, timeBackstepTag = 0;
    uint64_t minTimeTag = 0, maxTimeTag = 0;
    uint64_t twosixty = ((uint64_t) 1 << 61) - (uint64_t) 1;
    double backStepAllow = 1.;

    // Plant the data tree.
    TTree *prelimDataTree = new TTree("prelimDataTree",channelName[det]);
    prelimDataTree->Branch("detector",         &detector ,      "det/s");
    prelimDataTree->Branch("energy",      &energy    , "energy/s");
    prelimDataTree->Branch("extras",     &extras, "extras/I");
    prelimDataTree->Branch("timeTag",     &timeTag, "Time/l");

    // Debug histos.
    const int           t2326 = 1000;
    uint64_t            timeTag23[t2326], timeTag24[t2326], timeTag25[t2326], timeTag26[t2326];
    int                 timePointer23 = 0, timePointer24 = 0, timePointer25 = 0, timePointer26 = 0;
    for (int i=0; i<t2326; i++) {
        timeTag23[i] = 0;
        timeTag24[i] = 0;
        timeTag25[i] = 0;
        timeTag26[i] = 0;
        }
    TH1D *timing2326 = new TH1D("timing2326","timing2326;#it{t}_{23} - #it{t}_{26} [s]",100000,-1000.*tDiff,+1000.*tDiff);
    TH1D *timing2426 = new TH1D("timing2426","timing2426;#it{t}_{24} - #it{t}_{26} [s]",100000,-1000.*tDiff,+1000.*tDiff);
    TH1D *timing2526 = new TH1D("timing2526","timing2526;#it{t}_{25} - #it{t}_{26} [s]",100000,-1000.*tDiff,+1000.*tDiff);
    TH1D *timing2626 = new TH1D("timing2626","timing2626;#it{t}_{26} - #it{t}_{26} [s]",10000,0.,+10.*tDiff);

    TH1D *timeHistoDebug[digi][digi];
    for (int i=0; i<digi; i++) {
        for (int k=i; k<digi; k++) {
            timeHistoDebug[i][k] = new TH1D(Form("timeHistoDebug[%02d][%02d]",i,k),"Timing before sorting " + channelName[i] + " - " + channelName[k] + Form(" ;#it{t}_{%02d} - #it{t}_{%02d} [s];Counts",i,k),10000,-10.*tDiff,+10.*tDiff);
            } //
        } // for (int i=0; i<digi; i++)

    double              chanTime[digi];     // Needed for the sorting of the data.
    // uint64_t            chanTimeTag[digi];


    // Determine the number of files in the run.
    runFileName[0] = prefixPath + setupName + "/DAQ/" + runName + "_high/RAW/DataR_" + runName + "_high.BIN";
    runFileName[1] = prefixPath + setupName + "/DAQ/" + runName + "_high/RAW/DataR_" + runName + "_high_1.BIN";

    while ( gSystem->GetPathInfo(runFileName[runFiles],&id,&size,&flags,&modtimel)==0 ) {
        // Debugging output for modtime.
        modTime.Set(modtimel);
        cout << "File " << runFileName[runFiles] << " modified= " << modTime.AsSQLString() << " size= " << size << endl;
        runFiles ++;
        runFileName[runFiles] = prefixPath + setupName + "/DAQ/" + runName + "_high/RAW/DataR_" + runName + Form("_%d.BIN",runFiles);
        } // while ( gSystem->GetPathInfo(runFileName[runFiles],&id,&size,&flags,&modtimel)==0 )

    if (gSystem->GetPathInfo(prefixPath + setupName + "/DAQ/" + runName + "_high/settings.xml",
        &id,&size,&flags,&daqtimel)==0) {
        daqTime.Set(daqtimel);
        cout << runName << " started " << daqTime.AsSQLString() << endl;
        }

    cout << "===============================================================================" << endl;

    // Open last data file, in order to determine total run time.
    for (i=runFiles-1; i<runFiles; i++) {

        // Check data file mod time and whether it is okay.
        gSystem->GetPathInfo(runFileName[i],&id,&size,&flags,&modtimel);
        modTime.Set(modtimel);

        // Open the file.
        fin[i] = fopen(runFileName[i], "r");
        if (!fin[i]) {
             cout << "Error when trying to open file " << runFileName[i] << endl;
            exit(-1);
            } else {
            cout << "Opened file " << runFileName[i] << ", last modified ";
            // Determine last modification time of data file.
            cout << modTime.AsSQLString() << endl;
            }  // if (!fin[i])

        linesRead = 0;
        int debug=0;

        // UM5960_Compass_UserManual_rev20.pdf
        // p. 68: "Note: In the .bin format, the Header is present just once at the beginning of the file and not in each event."
        // New 24.10.2022.
        if (i==0) {
            fread(&headerN, 1, sizeof(headerN), fin[i]);
            cout << "headerN= " << headerN << " headerN & 0 = " << (headerN & 0) << endl;
            }

        while (     fread(&boardN, 1, sizeof(boardN), fin[i]) &&
                    fread(&channelN, 1, sizeof(channelN), fin[i]) &&
                    (sizeof(readTimeTag) == fread(&readTimeTag, 1, sizeof(readTimeTag), fin[i])) )
                    {

                fread(&readEnergy, 1, sizeof(readEnergy), fin[i]);
                fread(&readExtras, 1, sizeof(readExtras), fin[i]);
                // fread(&numSamples, 1, sizeof(numSamples), fin[i]);
                // fread(&samples ,numSamples, sizeof(samples), fin[det]);
                linesRead++;
                // New 24.10.2022 deleted "fread(&numSamples, 1, sizeof(numSamples), fin[i])"

                detector = (channelN % 16) + 16 * (boardN % 4);
                energy = readEnergy[0] & 32767;
                timeTag = (readTimeTag[0] & twosixty);

                if ((timeTag==0) || (linesRead<3) || ((linesRead%10000000) == 0)) {
                    cout << "linesRead= " << linesRead << " timeTag= " << readTimeTag[0] << " seconds= " << readTimeTag[0]/1.e12 <<  " det= " << detector << " en= " << energy << endl;
                    }
                // New 24.10.2022 % 4000 instead of % 8000
                if ((detector < 32) && ((timeTag % 4000) != 0) && (debug<2) ) {
                    cout << "detector= " << detector << " timeTag= " << timeTag << " time= " << timeTag/1.e12 << endl;
                    debug++;
                    }
            } // while ()
        cout << "Last timeTag         in file = " << timeTag << " in seconds= " << timeTag/1.e12 <<  endl;
        fclose ( fin[i] );
        } // for (i=runFiles-1; i<runFiles; i++)

    // Determine running time (using info from last file) and backlook time.
    UInt_t fileSystemRunningTime = modTime.Convert() - daqTime.Convert();
    maxTimeTag = timeTag;

        // Catch overly large files (more than 10 BIN files)
        if (runFiles > 10) {
            minTimeTag = maxTimeTag - maxTimeTag * 10. / (double) runFiles;
            }

        // Determine min and max time.
        if (maxTimeTag/1.e12 > stopSeconds) {
            cout << "maxTimeTag/1.e12= " << maxTimeTag/1.e12 << " > " << stopSeconds << " -> ";
            maxTimeTag = (uint64_t) (stopSeconds*1.e12);
            cout << "maxTimeTag/1.e12= " << maxTimeTag/1.e12 << endl;
            }
        if (maxTimeTag - backlookSeconds*1.e12 > 0) {
            minTimeTag = maxTimeTag - backlookSeconds*1.e12;
            } else {
            minTimeTag = 0;
            }   // if (maxTimeTag - backlookSeconds*1.e12 > 0)
        if (minTimeTag/1.e12 < startSeconds) {
            cout << "minTimeTag/1.e12= " << minTimeTag/1.e12 << " < " << startSeconds << " -> ";
            minTimeTag = (uint64_t) (startSeconds*1.e12);
            cout << "minTimeTag/1.e12= " << minTimeTag/1.e12 << endl;
            }
        cout << "maxTimeTag = " << maxTimeTag/1.e12 << ", minTimeTag= " << minTimeTag/1.e12 << endl;
        double thisRunRealtime = (maxTimeTag-minTimeTag)*1.e-12;
        
    // Determine modification time of file,
        // firstActiveChannelRead = det ;
        firstActiveModTime = modTime.AsSQLString();
        modTimeString = Form("_%02d%02d%02d_%02d%02d",modTime.GetYear()%2000, modTime.GetMonth(), modTime.GetDay(), modTime.GetHour(), modTime.GetMinute());
        firstActiveModTimeInt = modTime.Convert();
        TString dateDirString = Form("%02d%02d%02d/",modTime.GetYear()%100, modTime.GetMonth(), modTime.GetDay());

        // Time when the first DAQ file started: daqTime
        // daqTime.Set(modtimel - (long int)(maxTimeTag/1.e12) );

        // Time when the backlook period for K-40 integration starts: staTime
        staTime.Set(modtimel - (long int)(maxTimeTag/1.e12) + (long int)(minTimeTag/1.e12) );
        // Catch problem with CAEN time stamps.
        if ( (UInt_t)(maxTimeTag/1.e12-1.) > fileSystemRunningTime + 30 ) {
            staTime.Set(daqTime.Convert());
            minTimeTag = 0;
            cout << "!!! (UInt_t)(maxTimeTag/1.e12-1.) >  fileSystemRunningTime  + 30) !!!" << endl;
            cout << "(UInt_t)(maxTimeTag/1.e12-1.)= " << (UInt_t)(maxTimeTag/1.e12-1.) << " fileSystemRunningTime= " << fileSystemRunningTime << endl;
            }

        // Prepare count rate - histograms .
        runHours = maxTimeTag / 1.e12 / 3600.;

    // Check the time now, i.e. when the script is run.
    TDatime nowTime = TDatime();
    UInt_t nowTimeInt = nowTime.Convert();

    cout << "===============================================================================" << endl;

    TH1D *histoRateDigi[2];
    TH2D *histoRate2D[2];
    for (int i=0; i<2; i++) {
        histoRateDigi[i] = new TH1D(Form("histoRateDigi[%d]",i),";Nominal time [s];Rate[s^{-1}]",maxTimeTag / 1.e12 /60,0,maxTimeTag / 1.e12);
        histoRate2D[i] = new TH2D(Form("histoRate2D[%d]",i),Form("Digitizer %d;Event number;Nominal time [s]",i),1000,0, 500. * fileSystemRunningTime, maxTimeTag / 1.e12 /60,0,maxTimeTag / 1.e12);
        }

    // Create the histograms.
    for (det = 0; det < digi ; det++) {

        // Create the histogram for the K-40 analysis.
        histoRaw[det]  = new TH1D(Form("histoRaw[%02d]",det),channelName[det]+";;",
            histoChannels, 0., histoChannels);
            
        // Create calibrated histogram for Till.
        histoTillEcal[det]  = new TH1D(Form("histoTillEcal[%02d]",det),channelName[det]+";#it{E}_{#gamma} [MeV];Counts / keV;",
            20000, 0., 20.);


        // Create the histogram for the counting rate.
        histoRate[det] = new TH1D(Form("histoRate[%02d]",det),"Rates;#it{t} [h];Trigger rate [s^{-1}]",
            100,0.,runHours);
        // Fill the histogram for the counting rate.

        } // for (det = 0; det < digi ; det++)

    // ====================================
    // Open the data files, now for reading.
    // ====================================
    long long int eventCounter = 0;
    for (i=0; i<runFiles; i++) {

        // Check data file mod time and whether it is okay.
        gSystem->GetPathInfo(runFileName[i],&id,&size,&flags,&modtimel);
        modTime.Set(modtimel);
        fin[i] = fopen(runFileName[i], "r");
        if (!fin[i]) {
             cout << "Error when trying to open file " << runFileName[i] << endl;
            exit(-1);
            } else {
            cout << "Opened file " << runFileName[i] << ", last modified ";
            // Determine last modification time of data file.
            cout << modTime.AsSQLString() << endl;
            }  // if (!fin[i])

        // UM5960_Compass_UserManual_rev20.pdf
        // p. 68: "Note: In the .bin format, the Header is present just once at the beginning of the file and not in each event."
        // New 24.10.2022.
        if (i==0) {
            fread(&headerN, 1, sizeof(headerN), fin[i]);
            }
        // cout << "headerN= " << headerN << " headerN & 0 = " << (headerN & 0) << endl;

        while (     fread(&boardN, 1, sizeof(boardN), fin[i]) &&
                    fread(&channelN, 1, sizeof(channelN), fin[i]) &&
                    (sizeof(readTimeTag) == fread(&readTimeTag, 1, sizeof(readTimeTag), fin[i])) )
                    {

            fread(&readEnergy, 1, sizeof(readEnergy), fin[i]);
            fread(&readExtras, 1, sizeof(readExtras), fin[i]);
            // fread(&numSamples, 1, sizeof(numSamples), fin[i]);
            // fread(&samples ,numSamples, sizeof(samples), fin[det]);
            // New 24.10.2022 deleted "fread(&numSamples, 1, sizeof(numSamples), fin[i])"
            
            detector = (channelN % 16) + 16 * (boardN % 4);
            energy = readEnergy[0] & 32767;
            // timeTag = (readTimeTag[0] & twosixty);
            timeTag = readTimeTag[0];                   // mod 23.05.2023
            extras = readExtras[0] & 32767;

            eventCounter ++;

            if ( (energy>0) && (energy<32768) ) {       // mod 23.05.23 16384 -> 32768
                if ((timeTag > minTimeTag) && (timeTag < maxTimeTag)) {
                    histoRaw[detector]->Fill(energy);
                    }
                histoRate[detector]->Fill(timeTag/1.e12/3600.);
                } // if ( (energy>0) )
            
            // Charge integration.
            if ((detector == 30) && ((timeTag > minTimeTag) && (timeTag < maxTimeTag))) {
                q_cuta ++;
                // cout << "detector 30" << endl;
                }
            if ((detector == 28) && ((timeTag > minTimeTag) && (timeTag < maxTimeTag))) {
                q_cuco ++;
                }
            if ((detector == 27) && ((timeTag > minTimeTag) && (timeTag < maxTimeTag))) {
                q_cu3 ++;
                }

            if ((outRoot == 2) && ((energy > 0) || (detector == 26)) && (timeTag > minTimeTag) && (timeTag < maxTimeTag) ) {
                    prelimDataTree->Fill();
                    }

            // Fill time difference histograms, for the gamma-ray detectors (HPGe and BGO).
            for (int k=0; k<digi; k++) {
                if ((channelHPGe[detector]>0) && (channelHPGe[k]>0)) {
                    if (k < detector) {
                        // timeHisto[k][det]->Fill(chanTime[k] - timeTag/1.e12);
                        timeHistoDebug[k][detector]->Fill(chanTime[k] - timeTag/1.e12);
                        } else {
                        // timeHisto[det][k]->Fill(timeTag/1.e12 - chanTime[k]);
                        timeHistoDebug[detector][k]->Fill(timeTag/1.e12 - chanTime[k]);
                        } // else if (k < detector)
                } // if ((channelHPGe[detector]>0) && (channelHPGe[k]>0))
                } // for (int k=0; k<digi; k++)

            // Fill the intermediate variables for the detector at hand.
            chanTime[detector]  = timeTag / 1.e12;

        if ( (outRoot==2) && ((prelimDataTree->GetEntries() < 2) || (prelimDataTree->GetEntries() % 1000000 == 0)) ) {
            cout << "prelimDataTree->GetEntries()= " << prelimDataTree->GetEntries() << Form(" %.0f / %.0f seconds",timeTag/1.e12,maxTimeTag/1.e12) << '\r';
            }

        } // while ()

        } // for (i=0; i<runFiles; i++)

    cout << "prelimDataTree->GetEntries() = " << prelimDataTree->GetEntries() <<  " eventCounter= " << eventCounter << endl;

    TCanvas *c1 = new TCanvas("c1","c1",1600,800);
    // TCanvas *c1 = new TCanvas("c1","c1",1600,1100);
    // c1->Divide(7,5,.0001,.0001);
    c1->Divide(7,4,.001,.001);
    TString drawCommand = "";
    TString gateCommand = "";
    int k40c = 0;
    TF1 *turkgaus[digi], *turk2615[digi];
    TText *k40Text[digi];
    TPaveText *k40PaveText[digi];
    TPaveText *titlePaveText[digi];
    
    // If there is significant beam on target, we do *not* attempt an energy calibration.
    /*
    if ( (q_cuta*.01/thisRunRealtime > 1.) && (gSystem->GetPathInfo(eCalFileName,&id,&size,&flags,&daqtimel)==0) ) {
        eCal = -1;
        }
    */

    // Male alles Schritt für Schritt
    for (int i=0; i<pads; i++) {
        if ( channelHPGe[cd[i]] == 1) {
            c1->cd(i+1);
            gPad->SetLogy();

            histoRaw[cd[i]]->SetLineColor(kBlue);
            histoRaw[cd[i]]->SetFillColorAlpha(kBlue,.4);
            histoRaw[cd[i]]->Draw();
            gPad->Update();
            if ((titlePaveText[i] = ((TPaveText *)(gPad->FindObject("title")))) != 0) {
                titlePaveText[i]->SetTextSize(.11);
                }
            turkgaus[cd[i]] = new TF1(Form("turkgaus[%02d]",cd[i]),"gaus + [3]",0.,20.);

            if ((eCal > 0) && (defaultK40[cd[i]] == 1.46082)) {
                // We work on a channel like EB17D where calibration is hopeless.
                turkgaus[cd[i]]->SetParameters(-1.,-1.,-1.,-1.);
                channelEcal[cd[i]][0] = -1.;

                } else {
                // We do not work on a channel like EB17D where calibration is hopeless.

            // Do calibration.
            histoRaw[cd[i]]->GetXaxis()->SetRange((int)defaultK40[cd[i]]*.975,(int)defaultK40[cd[i]]*1.025);
            gPad->Update();
            k40c = histoRaw[cd[i]]->GetMaximumBin();
            // cout << "Debug cal " << cd[i] << " " << channelName[cd[i]] << " " << defaultK40[cd[i]] << " " << k40c; // << endl;

            // If there is an external calibration, implement it now.
            // Otherwise, calibrate with default K-40 value.
            if (eCal == -1) {
                for (int k=0; k<=histoChannels; k++) {
                    xbins[k] = calTF1[cd[i]]->Eval(k);
                    }
                histoRaw[cd[i]] -> GetXaxis() -> Set(histoChannels,xbins);
                } else {
                histoRaw[cd[i]]->GetXaxis()->SetLimits(0.,eEcal[0]/(double)k40c*(double)histoChannels);
                // cout << " /// " << eEcal[0]/(double)k40c*(double)histoChannels;
                }
            histoRaw[cd[i]]->GetXaxis()->SetRangeUser(eEcal[0]*.97,eEcal[0]*1.03);
            // cout << " histoRaw[cd[i]]->GetXaxis()->GetBinCenter(histoRaw[cd[i]]->GetMaximumBin())= " << histoRaw[cd[i]]->GetXaxis()->GetBinCenter(histoRaw[cd[i]]->GetMaximumBin()) << endl;
            gPad->Update();
            turkgaus[cd[i]]->SetParameters(histoRaw[cd[i]]->GetBinContent(histoRaw[cd[i]]->GetMaximumBin()), histoRaw[cd[i]]->GetXaxis()->GetBinCenter( histoRaw[cd[i]]->GetMaximumBin()), .002, histoRaw[cd[i]]->GetBinContent(histoRaw[cd[i]]->GetMinimumBin()) );
            if (histoRaw[cd[i]]->Fit(Form("turkgaus[%02d]",cd[i]),"rq") == 0) {
                // Fit does not fail. Use the fitted center of the Gaussian.
                channelEcal[cd[i]][0] = turkgaus[cd[i]]->GetParameter(1)/eEcal[0]*(double)k40c;
                } else {
                // If fit fails, the result is != 0, and this is marked in the parameters.
                turkgaus[cd[i]]->SetParameters(-1.,-1.,-1.,-1.);
                channelEcal[cd[i]][0] = -1.;
                }
                } // else if ((eCal > 0) && (defaultK40[cd[i]] == 1.46082))

            // Ausgabe.
            k40PaveText[cd[i]] = new TPaveText(.13,.13,.87,.23,"NDC NB");
            k40PaveText[cd[i]]->SetTextAlign(12);
            // k40PaveText[cd[i]]->SetTextSize(0.08);
            if (channelEcal[cd[i]][0] > 0.) {
                reso = TMath::Abs(turkgaus[cd[i]]->GetParameter(2)*2.355*1000.);
                } else {
                reso = -1.;
                }

            if ( (reso >= 1.7) && (reso <= 14.5) ) {
                // Everything okay, and we have acceptable resolution
                k40PaveText[cd[i]]->SetTextColor(kBlue);
                // channelEcal[cd[i]][0] = turkgaus[cd[i]]->GetParameter(1)/eEcal[0]*(double)k40c;
                // If fit does not fail, we set the E range according to the precise fit result.
                if (eCal > 0) {
                    histoRaw[cd[i]]->GetXaxis()->SetLimits(0.,eEcal[0]/channelEcal[cd[i]][0]*(double)histoChannels);
                    }
                k40PaveText[cd[i]]->AddText(Form("%.1f ch, %.1f keV",   channelEcal[cd[i]][0],
                    reso));
                } else {
                // Something is amiss.
                k40PaveText[cd[i]]->SetTextColor(kRed);
                if (histoRaw[cd[i]]->Integral(2, histoRaw[cd[i]]->GetNbinsX() ) == 0.) {
                    channelEcal[cd[i]][0] = -1.;
                    k40PaveText[cd[i]]->AddText("Spectrum empty");
                    } else {
                    // Fit successful, but resolution is bad.
                    // If fit does not fail, we set the E range according to the precise fit result.
                    if ( (eCal > 0) && (reso != -1.) ) {
                        channelEcal[cd[i]][0] = turkgaus[cd[i]]->GetParameter(1)/eEcal[0]*(double)k40c;
                        histoRaw[cd[i]]->GetXaxis()->SetLimits(0.,eEcal[0]/channelEcal[cd[i]][0]*(double)histoChannels);
                        }
                    k40PaveText[cd[i]]->AddText(Form("%.1f ch, %.1f keV",  channelEcal[cd[i]][0]  ,
                            reso));
                    }
                }

            k40PaveText[cd[i]]->SetFillColorAlpha(kWhite,.6);
            k40PaveText[cd[i]]->Draw();

            // Prüfe, ob Auflösung allzu schlecht ist.
            // if ( (reso < 1.8) || (reso > 4.5) || (turkgaus[cd[i]]->GetParameter(0)/turkgaus[cd[i]]->GetParameter(3) < 2. ) ) {
            if ( (reso < 1.7) || (reso > 4.7) ) {
                gPad->SetFillColorAlpha(kRed-7,.5);
                gPad->SetFillStyle(1001);
                gPad->Update();
                // cout << "kRed-7 " << cd[i] << " reso= " << reso << endl;
                }

            // Bei unplausiblem Bild wird das gesamte Spektrum gezeigt und zu Kanälen zurückgekehrt.
            // Der zweite Parameter ist der "Kontrast", d.h. das Verhältnis Peak/Background.
            if ( (reso < 1.5) ||  (turkgaus[cd[i]]->GetParameter(0)/turkgaus[cd[i]]->GetParameter(3) < 2. ) ) {
                if (eCal > 0) {
                    // Following two lines inserted 28.03.2025.
                    histoRaw[cd[i]]->GetXaxis()->SetLimits(0.,histoChannels);
                    histoRaw[cd[i]]->GetXaxis()->SetRange(800,histoChannels);
                    } else {
                    histoRaw[cd[i]]->GetXaxis()->SetLimits(0.,histoChannels);
                    histoRaw[cd[i]]->GetXaxis()->SetRange(800,histoChannels);
                    }
                gPad->Update();
                if (histoRaw[cd[i]]->Integral(2, histoRaw[cd[i]]->GetNbinsX() ) > 0.) {
                    k40PaveText[cd[i]]->AddText(Form("%d ch max",
                        histoRaw[cd[i]]->GetMaximumBin()));
                    gPad->SetGridx();
                    }
                if (eCal > 0) {
                    histoRaw[cd[i]]->GetXaxis()->SetRange(0,histoRaw[cd[i]]->FindLastBinAbove());
                    }
                }

            } // if ((channelHPGe[cd[i]] == 1)  )
        } // for (int i=0; i<pads; i++)

    // Now, repeat the fit for the other calibration lines.
    double fitX[eCalMax], fitY[eCalMax];
    if ((q_cuta*.01/thisRunRealtime < 0.1) && (eCal>3) && (Ep < 0.2) ) { //
        eCal = 3;
        cout << "/////// Set eCal=3" << endl;
        }
    int eCalUsed=0;
    // Now, repeat the fit for the other calibration lines.
    if (eCal > 0) {
    for (int i=0; i<pads; i++) {
    
        // Create calibration TF1, and set some dummy calibration.
        calTF1[cd[i]]->SetParameters(0., eEcal[0] / defaultK40[cd[i]] , 0.);

        // Look at other calibration lines.
        if (( channelHPGe[cd[i]] == 1) && (histoRaw[cd[i]]->GetXaxis()->GetBinCenter(16383) < 100.) && (defaultK40[cd[i]] != 1.46082)) {
            c1->cd(i+1);
            for (int c=1; c<eCal; c++) {
            histoRaw[cd[i]]->GetXaxis()->SetRangeUser(eEcal[c]*.96,eEcal[c]*1.04);
            gPad->Update();
            turkgaus[cd[i]]->SetParameters(histoRaw[cd[i]]->GetBinContent(histoRaw[cd[i]]->GetMaximumBin()), eEcal[0]/channelEcal[cd[i]][0]*histoRaw[cd[i]]->GetMaximumBin(), .002 * sqrt(eEcal[c]/eEcal[0]), histoRaw[cd[i]]->GetBinContent( histoRaw[cd[i]]->GetXaxis()->FindBin(eEcal[c]*.97))+.1 );
            if ((histoRaw[cd[i]]->Fit(Form("turkgaus[%02d]",cd[i]),"rq") != 0) ||
                (histoRaw[cd[i]]->GetBinContent(histoRaw[cd[i]]->GetMaximumBin()) / histoRaw[cd[i]]->GetBinContent(histoRaw[cd[i]]->GetMinimumBin()) < 3.) || (histoRaw[cd[i]]->GetBinContent(histoRaw[cd[i]]->GetMaximumBin()) < 6.)) {
                // If fit fails, the result is != 0, and this is marked in the parameters.
                // The same in case of too bad contrast, or low number of entries
                turkgaus[cd[i]]->SetParameters(-1.,-1.,-1.,-1.);
                channelEcal[cd[i]][c] = -1.;
                } else {
                channelEcal[cd[i]][c] = turkgaus[cd[i]]->GetParameter(1)/eEcal[0] * channelEcal[cd[i]][0] ;
                }
            } // for (int c=1; c<eCal; c++)

        // Now, adopt a calibration based on all eCal lines.
        if (channelEcal[cd[i]][0] > 0.) {
            // histoRaw[cd[i]] -> GetXaxis() -> SetRange(0,histoChannels);
            eCalUsed = 0;
            for (int c=0; c<eCal; c++) {
                if (channelEcal[cd[i]][c] > 0.) {
                    fitX[eCalUsed] = channelEcal[cd[i]][c];
                    fitY[eCalUsed] = eEcal[c];
                    eCalUsed ++;
                    } else {
                    // cout << "discard " << eEcal[c] << endl;
                    }
                }
            calGraph[cd[i]] = new TGraph(eCalUsed,fitX,fitY);
            // calGraph[cd[i]] = new TGraph(eCal,channelEcal[cd[i]],eEcal);
            if (eCalUsed > 1) {
                if (eCalUsed < 4) {
                    calTF1[cd[i]]->FixParameter(2,0.);
                    }
                calGraph[cd[i]]->Fit(Form("calTF1[%02d]",cd[i]),"qn");
                } else {
                calTF1[cd[i]]->SetParameters(0., fitY[0]/fitX[0], 0.);
                }
            cout << "cal " << cd[i] << " " << channelName[cd[i]];
            for (int c=0; c<eCal; c++) {
                cout << " " << channelEcal[cd[i]][c];
                }
            cout << " [" << calTF1[cd[i]]->Eval(0.) << "..." << calTF1[cd[i]]->Eval(histoChannels) << "] MeV" << endl;

            // Now, implement the calibration.
            // histoRaw[cd[i]] -> GetXaxis() -> SetLimits(calTF1[cd[i]]->Eval(0.), calTF1[cd[i]]->Eval(histoChannels));
            for (int k=0; k<=histoChannels; k++) {
                xbins[k] = calTF1[cd[i]]->Eval(k);
                }
            histoRaw[cd[i]] -> GetXaxis() -> Set(histoChannels,xbins);
            } // if ( (channelEcal[cd[i]][0] != -1) && (channelEcal[cd[i]][1] != -1)) {

        // For regular 40K operation, return to usual view.
        histoRaw[cd[i]]->GetXaxis()->SetRangeUser(eEcal[0]*.97,eEcal[0]*1.03);
        gPad->Update();
        turkgaus[cd[i]]->SetParameters(histoRaw[cd[i]]->GetMaximum(), histoRaw[cd[i]]->GetBinCenter( histoRaw[cd[i]]->GetMaximumBin()), .001, histoRaw[cd[i]]->GetBinContent( histoRaw[cd[i]]->GetXaxis()->FindBin(eEcal[0]*.97))+.1);
        histoRaw[cd[i]]->Fit(Form("turkgaus[%02d]",cd[i]),"rq","",eEcal[0]*.97,eEcal[0]*1.03);
        } // if ( channelHPGe[cd[i]] == 1)
        } // for (int i=0; i<pads; i++)
    } //     if (!externalCal)

    c1->cd(1);
    TPaveText *pave = new TPaveText(.11,.6,.75,.89,"NDC NB");
    pave->SetTextColor(kBlack);
    pave->SetTextAlign(11);
    // pave->SetTextSize(0.08);
    pave->AddText(runName)->SetTextSize(.12);
    pave->AddText("Status at " + firstActiveModTime);
    pave->AddText(Form("%.1f sec",(maxTimeTag-minTimeTag)*1.e-12));
    if (nowTimeInt - firstActiveModTimeInt > 1800) {
        pave->AddText(Form("File stopped %.1f h ago!",(nowTimeInt - firstActiveModTimeInt)/3600.));
        // cout << "Debug Pave added" << endl;
        // pave->Print();
        }
    pave->SetFillColorAlpha(kWhite,.5);
    pave->SetFillStyle(0);
    pave->Draw();
    if ((nowTimeInt - firstActiveModTimeInt > 1800) && (outRoot == 0)) {
        gPad->SetFillColorAlpha(kRed-7,.5);
        gPad->SetFillStyle(1001);
        gPad->Update();
        }

    // Text output only for DAQ1, not DAQ2.
    if ( ( daq == 1 ) && ( outK40==1 ) ) {
        // Öffne Ausgabefile.
        ofstream fout(prefixPath+"../K40check/"+dateDirString+runName+modTimeString+".txt");
        fout << "#\t" << firstActiveModTime << "\tupdated." << endl;
        fout << "#\t" << runHours << "\trunning time [hours]." << endl;
        if (nowTimeInt - firstActiveModTimeInt > 3600) {
            fout << Form(" --- File outdated, %.1f hours old",(nowTimeInt - firstActiveModTimeInt)/3600.) << endl;
            } else {
            fout << endl;
            }

        // fout << "#\t" << channelFile[firstActiveChannelRead] << endl;
        fout << "#\t" << (maxTimeTag-minTimeTag)*1.e-12 << "\tK-40 integration time." << endl;
        fout << "Channel\tName\tK40channel\tK40fwhm[keV]" << endl;
    } // if ( ( daq == 1 ) && ( outK40==1 ) )


    // Now make the rate plots.
    for (int i=0; i<pads; i++) {
        // Skaliere die Raten mit der Binbreite in Sekunden, damit cps rauskommt.
        histoRate[cd[i]]->Scale(1./( runHours/100. * 3600.));
        }
    // Skalierung auch für die BGOs, aber nochmal durch 100 geteilt
    histoRate[14]->Scale(1./( runHours/100. * 3600.) / 100.);
    histoRate[26]->Scale(1./( runHours/100. * 3600.) / 100.);

    // Skalierung für die Stromstärken. 1 cps = 1e-8 A = .01 uA
    histoRate[30]->Scale(1./( runHours/100. * 3600.) *.01);
    histoRate[28]->Scale(1./( runHours/100. * 3600.) *.01);
    histoRate[27]->Scale(1./( runHours/100. * 3600.) *.01);

    // Rates EB18.
    // cout << "Rates EB18" << endl;
    c1->cd(pads+1);
    double yMax = 0.;
    TLegend *leg22 = new TLegend(.15,.5,.5,.89);
    for (int i=0; i<pads; i++) {
        if (histoRate[cd[i]]->GetMaximum()*1.5 > yMax) {
            yMax = histoRate[cd[i]]->GetMaximum()*1.5;
            }
        }
    for (int i=0; i<7; i++) {
        if (i==0) {
            histoRate[cd[i]]->Draw("hist");
            histoRate[cd[i]]->SetStats(kFALSE);
            } else {
            histoRate[cd[i]]->Draw("hist same");
            }
        histoRate[cd[i]]->SetLineColor(kOrange+i);
        leg22->AddEntry(histoRate[cd[i]],"Rate "+channelName[cd[i]],"l");
        }
    histoRate[cd[0]]->GetYaxis()->SetRangeUser(0.,yMax);
    leg22->SetBorderSize(0);
    leg22->SetFillStyle(0);
    leg22->Draw();

    // Rates next seven detectors
    c1->cd(pads+2);
    // cout << "Rates MB" << endl;
    TLegend *leg23 = new TLegend(.15,.5,.7,.89);
    for (int i=7; i<14; i++) {
        if (i==7) {
            histoRate[cd[i]]->Draw("hist");
            histoRate[cd[i]]->SetStats(kFALSE);
            histoRate[cd[i]]->GetYaxis()->SetRangeUser(0.,yMax);
            } else {
            histoRate[cd[i]]->Draw("hist same");
            }
        histoRate[cd[i]]->SetLineColor(kTeal+i);
        leg23->AddEntry(histoRate[cd[i]],"Rate "+channelName[cd[i]],"l");
        }

    /*
    histoRate[26]->Draw("hist same");
    histoRate[26]->SetLineColor(kTeal+14);
    leg23->AddEntry(histoRate[26],"Rate "+channelName[26]+"/100","l");

    histoRate[14]->Draw("hist same");
    histoRate[14]->SetLineColor(kTeal+15);
    leg23->AddEntry(histoRate[14],"Rate "+channelName[14]+"/100","l");
    */

    histoRate[cd[7]]->GetYaxis()->SetRangeUser(0.,yMax);
    leg23->SetBorderSize(0);
    leg23->SetFillStyle(0);
    leg23->Draw();

    // ================================================================
    // Rates EB17.
    c1->cd(pads+3);
    TLegend *leg24 = new TLegend(.15,.5,.7,.89);
    for (int i=14; i<21; i++) {
        if (i<pads) {
        if (i==14) {
            histoRate[cd[i]]->Draw("hist");
            histoRate[cd[i]]->SetStats(kFALSE);
            histoRate[cd[i]]->GetYaxis()->SetRangeUser(0.,yMax);
            } else {
            histoRate[cd[i]]->Draw("hist same");
            }
        histoRate[cd[i]]->SetLineColor(kAzure+i);
        leg24->AddEntry(histoRate[cd[i]],"Rate "+channelName[cd[i]],"l");
        } // if (i < pads)
        }
    if (14 < pads) {
    histoRate[cd[14]]->GetYaxis()->SetRangeUser(0.,yMax);
    leg24->SetBorderSize(0);
    leg24->SetFillStyle(0);
    leg24->Draw();
    } // if (14 < pads)

    // ================================================================
    // Beam current.
    c1->cd(pads+4);
    TLegend *leg4 = new TLegend(.15,.7,.7,.89);

    histoRate[30]->SetTitle("Beam current;#it{t} [h];Current [#muA]");
    histoRate[30]->Draw("hist");
    histoRate[30]->SetStats(kFALSE);
    histoRate[30]->GetYaxis()->SetRangeUser(0.,15.);
    histoRate[30]->SetLineColor(kRed);
    leg4->AddEntry(histoRate[30],"Current "+channelName[30]+Form(", %.1f mC",q_cuta*.01/1000.),"l");

    histoRate[28]->Draw("hist same");
    histoRate[28]->SetLineColor(kBlue);
    leg4->AddEntry(histoRate[28],"Current "+channelName[28]+Form(", %.1f mC",q_cuco*.01/1000.),"l");

    histoRate[27]->Draw("hist same");
    histoRate[27]->SetLineColor(kBlue);
    leg4->AddEntry(histoRate[27],"Current "+channelName[27]+Form(", %.1f mC",q_cu3*.01/1000.),"l");

    leg4->SetBorderSize(0);
    leg4->SetFillStyle(0);
    leg4->Draw();

    // ================================================================
    // Debugging output.
    if (debugPlot == 2 ) {
    // Overwrite the pad contents for debugging purposes.
    histoRateDigi[0]->Scale(1./ histoRateDigi[0]->GetXaxis()->GetBinWidth(1));
    histoRateDigi[1]->Scale(1./ histoRateDigi[1]->GetXaxis()->GetBinWidth(1));
    histoRateDigi[0]->Draw("hist");
    histoRateDigi[0]->GetYaxis()->SetRangeUser(0., histoRateDigi[0]->GetMaximum()*5.);
    histoRateDigi[0]->SetLineColor(kBlue);
    histoRateDigi[1]->Draw("hist same");
    histoRateDigi[1]->SetLineColor(kRed);
    TLegend *leg24b = new TLegend(.15,.7,.7,.89);
    for (int i=0; i<2; i++) {
        leg24b->AddEntry(histoRateDigi[i],Form("Digitizer %d = %.0f s^{-1}",i,
            histoRateDigi[i]->GetEntries()/((double) modTime.Convert() - daqTime.Convert()) ),"l");
        }
    leg24b->SetBorderSize(0);
    leg24b->SetFillStyle(0);
    leg24b->Draw();

    // Even more debugging.
    c1->cd(22);
    gPad->SetLogy(kFALSE);
    gPad->SetLeftMargin(.15);
    gPad->SetFillColorAlpha(kWhite,.5);
    histoRate2D[0]->Draw("colz");
    histoRate2D[0]->GetYaxis()->SetRangeUser(0., maxTimeTag/1.e12);
    c1->cd(23);
    gPad->SetLogy(kFALSE);
    gPad->SetLeftMargin(.15);
    gPad->SetFillColorAlpha(kWhite,.5);
    histoRate2D[1]->Draw("colz");
    histoRate2D[1]->GetYaxis()->SetRangeUser(0., maxTimeTag/1.e12);
    } // if (debugPlot == 2)
    // End debugging output.

    // ================================================================
    // Pad with extra information.
    c1->cd(pads+5);
    TPaveText *pave27 = new TPaveText(.05,.2,.95,.9,"NDC NB");
    pave27->SetTextColor(kBlack);
    pave27->SetTextAlign(11);
    // pave->SetTextSize(0.08);
    pave27->AddText(runName)->SetTextSize(.12);
    pave27->AddText(prefixPath);
    
    TString daqTimeString = daqTime.AsSQLString();
    TString staTimeString = staTime.AsSQLString();

    pave27->AddText("Run created " + daqTimeString );
    pave27->AddText("K-40 test   " + staTimeString );
    pave27->AddText("Last  write " + firstActiveModTime );
    pave27->AddText(Form("CAEN realtime = %.1f hours", runHours));
    pave27->AddText(Form("CAEN realtime for this plot = %.0f s", thisRunRealtime));
    pave27->AddText(Form("Realtime from files = %d s",
            modTime.Convert() - daqTime.Convert()));
    pave27->AddText(Form("Charge = %.2f mC (%.2f #muA)",
            q_cuta*.01/1000., q_cuta*.01/thisRunRealtime));
    pave27->AddText(Form("Collim = %.2f mC (%.2f #muA)",
            q_cuco*.01/1000., q_cuco*.01/thisRunRealtime));
    cout << "charge= " << q_cuta << " collim= " << q_cuco << " in ticks" << endl;
    // pave27->AddText(Form("#it{E}_{p} #approx %.0f keV", Ep*1000.));
    pave27->AddText(" ");
    pave27->AddText(Form("DAQ%d",daq))->SetTextSize(.12);

    pave27->SetBorderSize(0);
    pave27->SetFillColorAlpha(kWhite,.5);
    pave27->SetFillStyle(0);

    if (nowTimeInt - firstActiveModTimeInt > 1800) {
        pave27->AddText(Form("File stopped %.1f h ago!",(nowTimeInt - firstActiveModTimeInt)/3600.));
        if (outRoot == 0) {
            gPad->SetFillColorAlpha(kRed-7,.5);
            gPad->SetFillStyle(1001);
            } // if (outRoot == 0)
        }

    pave27->Draw();

    // Angle plots.
    const int angles = 8;
    double angle[angles] = {38., 38., 43., 90., 90., 120., 137., 145.}, Egamma[angles], vc, Egamma0, fullDoppler;
    double fittedAngle[angles]= { 35, 45, 53, 86, 90, 125, 110, 118};
        // These values have been fitted on 23.05.2023 by Daniel for run 1765.
        // Nota bene they include a mis-calibration (nonlinearity) observed in Ron100. This artificially increases the energy and decreases the Ron100 angle.
        // True Ron100 angle is higher than this fitted value.

    TString angleName[angles] = {"38^{#circ} MB2",
                                "38^{#circ} EB17ABCEFG" ,
                                "43^{#circ} EB18AF" ,
                                "90^{#circ} EB18BGE" ,
                                "90^{#circ} Can60" ,
                                "120^{#circ} MB1" ,
                                "137^{#circ} EB18CD" ,
                                "145^{#circ} Ron100" };
                    // Angles are for now taken from Freimann p.  29, Table 2.3 - only averages
    /* TString channelName[digi] = {
        "EB18/A",   "EB18/B",   "EB18/C",   "EB18/D",   "EB18/E",   "EB18/F",   "EB18/G",   "MB2/1",
        "MB2/2",    "MB2/3",    "Ron100",   "Ron100/1",    "Ron100/2",    "Can60",    "Can60BGO", "frei",
        "EB17/A",   "EB17/B",   "EB17/C",   "EB17/D",   "EB17/E",    "EB17/F",   "EB17/G",  "MB1/1",
        "MB1/2",    "MB1/3",    "MB1/BGO",  "Cu3",     "CuCo",     "Ron100/3",      "CuTa",     "Ron100/4"}; */
    int angleDet[angles][7] = {
        {7, 8, 9, -1, -1, -1, -1 },
        {16, 17, 18, 20, 21, 22, -1},
        {0, 5, -1, -1, -1, -1, -1 },
        {1, 6, 4, -1, -1, -1, -1 },
        {13, -1, -1, -1, -1, -1, -1 },
        {23, 24, 25, -1, -1, -1, -1 },
        {2, 3, -1, -1, -1, -1, -1 },
        {10, -1, -1, -1, -1, -1, -1 }
        };
    // TH1D *histAngle[angles];
    TH1D *histAngle[angles];
    TH1D *histAngleAB[angles];
    const int roiLineNo = 4;
    TLine *roiLine[angles][roiLineNo], bgLine[angles];
    TArrow *roiArrow[angles];
    TRandom3 *r3Louis = new TRandom3(0);
    TPaveText *titlePaveTextAngle[angles];

    const int namedNo = 8;
    TString namedName[namedNo] = {"Run", "Charge[mC]", "Collimator[mC]", "Realtime[s]", "Starttime", "Stoptime", "Yield[mC^{-1}]", "DYield[mC^{-1}]"};
    TString namedContents[namedNo];
    TNamed *named[namedNo];

    double roi[angles][2];
    double net[angles], dnet[angles], raw[angles], left[angles], right[angles], dleft[angles], dright[angles];
    TLatex *intText[angles];
    double ta61RealTime = 0., ta61Charge = 0.;
    double noBeamRealTime = 0.;
    double beamScale = 0., noBeamScale = 0.;

     for (int k=0; k<angles; k++) {
        // Compute gamma energy.
        // 2H(p,gamma)
        Egamma0 = 5.49347508 + 0.957 * (Ep+.01) * 2.0141 / (1.0078+2.0141);
        Egamma0 = Egamma0 - Egamma0*Egamma0 / 2. / (1.0078+2.0141) / 931.494;
        vc = sqrt(2. * 0.957 * (Ep+.01) * 1.0078 * 931.494) / (1.0078+2.0141) / 931.494;
        // 13C(p,gamma)
        Egamma0 = 7.5505636 + 0.957 * (Ep+.01) * 13.00335483534 / (1.0078 + 13.00335483534);
        Egamma0 = Egamma0 - Egamma0*Egamma0 / 2. / (1.0078 + 13.00335483534) / 931.494;
        vc = sqrt(2. * 0.957 * (Ep+.01) * 1.0078 * 931.494) / (1.0078 + 13.00335483534) / 931.494;
        // Compute Doppler
        fullDoppler = vc * Egamma0;
        // Egamma[k] = Egamma0 + cos(angle[k]/180. * TMath::Pi()) * fullDoppler;

        } // for (int k=0; k<angles; k++)
    cout << endl;

        // Prepare sum of net entries.
        double netsum=0., dnetsum=0.;

        dnetsum = sqrt(dnetsum);

        // pave27->AddText(Form("Yield = %.3f #pm %.3f mC^{-1}", netsum / (q_cuta*.01/1000.), dnetsum /    (q_cuta*.01/1000.)));



    // File output for 40K script.
    if (outK40 == 1) {
        c1->Print(prefixPath+"../K40check/"+dateDirString+runName+modTimeString+".pdf");
        c1->Print(prefixPath+"../K40check/"+dateDirString+runName+modTimeString+".eps");
        }

    for (int i=0; i<digi; i++) {

        if ( debugPlot >= 3 ) {
            cout << Form("histoRate[%02d] %6.0f overflow %6.0f (%2.0f%%) ",i, histoRate[i]->GetEntries(),
                histoRate[i]->GetBinContent(histoRate[i]->GetNbinsX()+1), histoRate[i]->GetBinContent(histoRate[i]->GetNbinsX()+1)/histoRate[i]->GetEntries()*100.) << channelName[i] << endl;
            }

        }

    TFile *saveFile = NULL;

    // File output for run analysis.
    if (outRoot > 0) {
        c1->Print("/felsdata/n14ag_Felsenkeller_2025/Rootfiles/"+runName+modTimeString+".pdf");
        saveFile = new TFile("/felsdata/n14ag_Felsenkeller_2025/Rootfiles/"+runName+modTimeString+"_Tree.root", "RECREATE");
        cout << "created ROOT file " << saveFile->GetName() << endl;

        saveFile->mkdir("Canvas");
        saveFile->cd("Canvas");
        c1->Write();

        /*
        saveFile->mkdir("histAngle");
        saveFile->cd("histAngle");
        for (int k=0; k<angles; k++) {
            histAngle[k]->Write();
            } // for (int k=0; k<angles; k++)
        */
 
        saveFile->mkdir("histoRaw");
        saveFile->cd("histoRaw");
        for (int i=0; i<digi; i++) {
            histoRaw[i]->Write();
        }
            
        saveFile->mkdir("histoRate");
        saveFile->cd("histoRate");
        for (int i=0; i<digi; i++) {
            histoRate[i]->Write();
        }

        saveFile->mkdir("Calibration");
        saveFile->cd("Calibration");
        for (int i=0; i<pads; i++) {
            calTF1[cd[i]]->Write();
            }
        // cout << "Debug 1 " << endl;
        for (int i=0; i<pads; i++) {
            //if ((eCal > 0) && (channelEcal[cd[i]][0] > 0.)  && (calGraph[cd[i]]->GetN() > 0.)) {
                // cout << cd[i] << " " << channelName[cd[i]] << " ===================" << endl;
                // calGraph[cd[i]]->Print();
                calGraph[cd[i]]->SetName(Form("calGraph[%02d]",cd[i]));
                calGraph[cd[i]]->Write();
                // cout << channelName[cd[i]] << " " ;
              //  }
            }
        // cout << endl;   cout << "Debug 2 " << endl;

        namedContents[0] =  runName.Data();
        namedContents[1] =  Form("%.3f",q_cuta*.01/1000.);
        namedContents[2] =  Form("%.3f",q_cuco*.01/1000.);
        namedContents[3] =  Form("%.1f",thisRunRealtime);
        namedContents[4] =  daqTimeString.Data();
        namedContents[5] =  firstActiveModTime.Data();
        namedContents[6] =  Form("%.4f",netsum / (q_cuta*.01/1000.));
        namedContents[7] =  Form("%.4f",dnetsum / (q_cuta*.01/1000.));

        saveFile->mkdir("RunInfo");
        saveFile->cd("RunInfo");
        for (int k=0; k<namedNo; k++) {
            named[k] = new TNamed(namedName[k],namedContents[k]);
            named[k] -> Write();
            }
            
        cout << runName << Form(" RT= %.1f ",thisRunRealtime) <<
            Form(" Q= %.3f Yield= %.4f +/- %.4f",q_cuta*.01/1000., netsum / (q_cuta*.01/1000.), dnetsum / (q_cuta*.01/1000.)) << endl;
        } // if (outRoot > 0)

    if ((outK40 == 1) && (outRoot == 0)) {
        cout << "outK40 == 1 && outRoot == 0 - we will terminate the application here." << endl;
        gApplication->Terminate();
        }

    // Build the data tree.
    // =====================================================================================
    // Sort the prelimDataTree.
    // =====================================================================================
    cout << "===============================================================================" << endl;
    cout << "Realtime= " << rootTime->RealTime() << ", start working with prelimDataTree." << endl; rootTime->Continue();
    Int_t nentries = (Int_t)prelimDataTree->GetEntries();
    //Drawing variable pz with no graphics option.
    //variable timeTag stored in array fV1 (see TTree::Draw)
    prelimDataTree->SetEstimate(prelimDataTree->GetEntries());
    //*
    prelimDataTree->Draw("timeTag","","goff");
    // cout << "Realtime= " << rootTime->RealTime() << ", draw done." << endl; rootTime->Continue();
    Int_t *index = new Int_t[nentries];
    cout << "Created index, " << nentries << " entries." << endl;
    //sort array containing pz in decreasing order
    //The array index contains the entry numbers in decreasing order of pz -> increasing DB
    TMath::Sort(nentries,prelimDataTree->GetV1(),index,kFALSE);
    cout << "Realtime= " << rootTime->RealTime() << ", prelimDataTree sorted." << endl; rootTime->Continue();
    //*/
    // TEventList *sortedEventList = new TEventList("sortedEventList","sortedEventList",unsortedTree->GetEntries(),0);

    // ===================================================================================
    // Create timing histograms.
    // ===================================================================================
    TH1D *timeHisto[digi][digi];
    for (int i=0; i<digi; i++) {
        for (int k=i; k<digi; k++) {
            timeHisto[i][k] = new TH1D(Form("timeHisto[%02d][%02d]",i,k),"Timing " + channelName[i] + " - " + channelName[k] + Form(" ;#it{t}_{%02d} - #it{t}_{%02d} [s];Counts",i,k),10000,-10.*tDiff,+10.*tDiff);
            } //
        } // for (int i=0; i<digi; i++)

    // ===================================================================================
    // Create final data tree.
    // ===================================================================================
    double              eventTime = 0.;

    Double32_t          eventEner[digi];
    unsigned short int  eventExtr[digi];
    double              cumulTarg = 0.;
    double              cumulColl = 0.;
    double              cumulCell = 0.;

    TTree *felsTree = new TTree("felsTree",runName+ " raw data");
    felsTree->Branch(   "time", &eventTime,     "time/D");
    felsTree->Branch(   "targ", &cumulTarg,     "targ/D");
    felsTree->Branch(   "coll", &cumulColl,     "coll/D");
    felsTree->Branch(   "cell", &cumulCell,     "cell/D");
    for (int i=0; i<digi; i++) {
        felsTree->Branch(Form("MeV%02d",i), &eventEner[i],     Form("MeV%02d/d",i));
        // felsTree->Branch(Form("ctime%02d",i), &chanTime[i],     Form("ctime%02d/D",i));     // Debug DB
        felsTree->Branch(Form("extr%02d",i), &eventExtr[i],     Form("extr%02d/s",i));
        }

    // ===================================================================================
    // Fill final data tree.
    // ===================================================================================
    for (long int p=0; p<nentries; p++) {

        // Call entry from the sorted index.
        prelimDataTree->GetEntry(index[p]);
        // As a reminder, the branches are:
        //      prelimDataTree->Branch("det",         &detector ,      "det/s");
        //      prelimDataTree->Branch("energy",      &energy    , "energy/s");
        //      prelimDataTree->Branch("extras",     &extras, "extras/I");
        //      prelimDataTree->Branch("timeTag",     &timeTag, "Time/l");
        
        // Now, work on the sorting of the data into the tree.

            if ((detector < 0 ) || (detector >= digi) ) {
                cout << "detector= " << detector << endl;
                detector = detector % digi;
                }
            // Check whether the event must be closed.
            // This may be either if the time distance to the previous element is >= tDiff.
            // Or if the detector is a HPGe and there is a previous HPGe event that was not yet saved.
            if ( ((double)timeTag / 1.e12 - eventTime > tDiff) || ( ( channelHPGe[detector]==1 ) && (eventEner[detector] > 0.) ) )  {
                // Fill tree with data gathered before present event.
                felsTree->Fill();
                // Clear variables for the new event.
                for (int k=0; k<digi; k++) {
                        eventEner[k] = 0.;
                        eventExtr[k] = 0.;
                        }
                // Set time stamp for new event.
                eventTime = (double)timeTag / 1.e12;
                } // if ( ((double)timeTag / 1.e12 - eventTime > tDiff) || ( ( channelHPGe[det]==1 ) && (eventEner[det] > 0.) ) )


            // Fill time difference histograms, for the gamma-ray detectors (HPGe and BGO).
            for (int k=0; k<digi; k++) {
                if ((channelHPGe[detector]>0) && (channelHPGe[k]>0)) {
                    if (k < detector) {
                        // timeHisto[k][det]->Fill(chanTime[k] - timeTag/1.e12);
                        timeHisto[k][detector]->Fill(chanTime[k] - timeTag/1.e12);
                        } else {
                        // timeHisto[det][k]->Fill(timeTag/1.e12 - chanTime[k]);
                        timeHisto[detector][k]->Fill(timeTag/1.e12 - chanTime[k]);
                        } // else if (k < detector)
                } // if ((channelHPGe[detector]>0) && (channelHPGe[k]>0))
                } // for (int k=0; k<digi; k++)

            // Fill the intermediate variables for the detector at hand.
            chanTime[detector]  = timeTag / 1.e12;
            // chanTimeTag[detector]  = timeTag;
            // Only for non-pathological HPGe detectors and for BGO the energy signal is kept.
            // For HPGe detectors that cannot be calibrated, the energy is kept at zero.
            if (channelHPGe[detector] >= 1) {
                eventEner[detector] = calTF1[detector]->Eval((double)energy + r3Louis->Rndm() - 0.5);
                eventExtr[detector] = extras;
                } else {
                eventEner[detector] = energy;
                eventExtr[detector] = extras;
                } //  if (channelGamma[detector]>=1)
            if (extras == 0) {
                histoTillEcal[detector]->Fill(eventEner[detector]);
                }

            if (detector == 30) {
                // eventTarg += .01;
                cumulTarg += .01;
                }
            if (detector == 28) {
                cumulColl += .01;
                }
            if (detector == 27) {
                cumulCell += .01;
                }


        // Progress indicator.
        if (p % 100000 == 0) {
            cout << Form("%03.1f %% read: %6.0f of %6.0f seconds, %ld lines",
                eventTime/maxTimeTag/1.e-12*100.,eventTime, maxTimeTag/1.e12,p) << '\r';
            }

        } // for (long int p=0; p<nentries; p++)
        cout << endl;

    cout << "===============================================================================" << endl;
    cout << "Realtime= " << rootTime->RealTime() << ", final data tree filled." << endl; rootTime->Continue();
    cout << "prelimDataTree->GetEntries()= " << prelimDataTree->GetEntries() << " felsTree->GetEntries()= " << felsTree->GetEntries() << endl;
    cout << "===============================================================================" << endl;

    
        
        
        
    if (debugPlot == 1) {
    TCanvas *c2 = new TCanvas("c2","c2",100,100,1700,1000);
    c2->Divide(7,4);

    // ===========================================================
    // Show timing histograms.
    TLine *tDiffLine[2];
    tDiffLine[0] = new TLine(-tDiff,.9,-tDiff,1000.);
    tDiffLine[1] = new TLine(+tDiff,.9,+tDiff,1000.);
    tDiffLine[0]->SetLineColor(kRed);
    tDiffLine[1]->SetLineColor(kRed);

    /*
        "EB18/A",   "EB18/B",   "EB18/C",   "EB18/D",   "EB18/E",   "EB18/F",   "EB18/G",   "MB2/1",
        "MB2/2",    "MB2/3",    "Ron100",   "Ron100/1",    "Ron100/2",    "Can60",    "Can60BGO", "frei",
        "EB17/A",   "EB17/B",   "EB17/C",   "EB17/D",   "EB17/E",    "EB17/F",   "EB17/G",  "MB1/1",
        "MB1/2",    "MB1/3",    "MB1/BGO",  "Cu3",     "CuCo",     "Ron100/3",      "CuTa",     "Ron100/4"};

     */

    gStyle->SetOptStat(1111111);
    // Timing for EB18.
    cout << "Timing for EB18 " << endl;
    for (int i=1; i<7; i++) {
        c2->cd(i);
        gPad->SetLogy();
        timeHisto[0][i]->Draw();
        tDiffLine[0]->Draw();
        tDiffLine[1]->Draw();
        }
    // Timing for MB2.
    cout << "Timing for MB2 " << endl;
    for (int i=8; i<10; i++) {
        c2->cd(i-1);
        gPad->SetLogy();
        timeHisto[7][i]->Draw();
        tDiffLine[0]->Draw();
        tDiffLine[1]->Draw();
        }
    // Timing for Can60.
    cout << "Timing for Can60 " << endl;
        c2->cd(9);
        gPad->SetLogy();
        timeHisto[13][14]->Draw();
        tDiffLine[0]->Draw();
        tDiffLine[1]->Draw();

    // Timing for EB17.
    cout << "Timing for EB17 " << endl;
    for (int i=17; i<23; i++) {
        c2->cd(i+1-8);
        gPad->SetLogy();
        timeHisto[16][i]->Draw();
        tDiffLine[0]->Draw();
        tDiffLine[1]->Draw();
        }
    // Timing for MB1.
    cout << "Timing for MB1 " << endl;
    for (int i=24; i<27; i++) {
        c2->cd(i+1-9);
        gPad->SetLogy();
        timeHisto[23][i]->Draw();
        tDiffLine[0]->Draw();
        tDiffLine[1]->Draw();
        }

        


    if (outRoot > 0) {

        cout << "===============================================================================" << endl;

        c2->cd(19);
        felsTree->Draw("MeV13 >> histo13(10000,0.,20.)","MeV13 > 0.");
        felsTree->Draw("MeV13 >> histo13ACS(10000,0.,20.)","MeV13 > 0. && MeV14 == 0.","same");

        saveFile->cd("Canvas");
        c2->Write();

        } // if (outRoot > 0)
        
    } // if (debugPlot == 1)

    if (outRoot ==2 ) {
	saveFile->mkdir("Tree");
	saveFile->cd("Tree");
	felsTree->Write();
	} // if (outRoot == 2)

    if (outRoot >0 ) {
	saveFile->Close();
    } // if (outRoot > 0)


    /*
    cout << "===============================================" << endl;
    cout << "defaultK40[32] = { " << endl;
    for (k=0; k< digi; k++) {
        cout << Form("\t%.1f,", channelEcal[k][0]);
        if ( (k%8 == 0) && ( k>0 ) ) {
            cout << endl;
            }
        }
    cout << " };" << endl;
    */

    cout << "ROOT script effort: Real time (in seconds)= " << rootTime->RealTime() << " CPU time (in seconds)= " << rootTime->CpuTime() << endl;
}






