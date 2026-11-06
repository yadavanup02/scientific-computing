#include "TROOT.h"
#include "TFile.h"
#include "TTree.h"
#include "TH1F.h"
#include "TCanvas.h"

#include <iostream>
#include <fstream>
#include <vector>
#include <queue>
#include <string>

using namespace std;

struct RawEvent {
    int16_t board;
    int16_t channel;
    int64_t timestamp;
    int16_t energyLong;
    int16_t energyShort;
    int32_t flags;
    int8_t waveformCode;
    int32_t nSamples;
    vector<int16_t> waveform; // Using vector to avoid manual new/delete
};

struct RawEventTimeComparison {
    bool operator()(const RawEvent& lhs, const RawEvent& rhs) {
        return lhs.timestamp > rhs.timestamp;
    }
};

typedef priority_queue<RawEvent, vector<RawEvent>, RawEventTimeComparison> RawEventQueue;

void readBinFile_v() {
    int run_number = 4182;
    string inputDirectory = "/felsdata/2024_10_SetupAndTests/Runs_DAQ2/since_Run4171/DAQ/Run" + std::to_string(run_number) + "_high/RAW/";
    string outputDirectory = "/felsdata/Gastarget/Measurements/n14_ag/";
    vector<string> binFiles = {inputDirectory + "DataR_Run" + to_string(run_number) + "_high.BIN"};
    string rootFilename = outputDirectory + "Run_" + to_string(run_number) + "_high.root";

    RawEventQueue queue;

    for (const auto& filename : binFiles) {
        ifstream inFile(filename, ios::binary);
        if (!inFile) {
            cerr << "Error opening file: " << filename << endl;
            continue;
        }

        uint16_t header;
        inFile.read(reinterpret_cast<char*>(&header), sizeof(header));
        if (!inFile) {
            cerr << "Failed to read header: " << filename << endl;
            continue;
        }

        bool fileHasEnergyLong = (header & (1 << 0)) != 0;
        bool fileHasEnergyShort = (header & (1 << 2)) != 0;
        bool fileHasWaveforms = (header & (1 << 3)) != 0;

        while (inFile) {
            RawEvent event;
            if (!inFile.read(reinterpret_cast<char*>(&event.board), sizeof(event.board))) break;
            if (!inFile.read(reinterpret_cast<char*>(&event.channel), sizeof(event.channel))) break;
            if (!inFile.read(reinterpret_cast<char*>(&event.timestamp), sizeof(event.timestamp))) break;

            if (fileHasEnergyLong) {
                if (!inFile.read(reinterpret_cast<char*>(&event.energyLong), sizeof(event.energyLong))) break;
            }
            if (fileHasEnergyShort) {
                if (!inFile.read(reinterpret_cast<char*>(&event.energyShort), sizeof(event.energyShort))) break;
            }

            if (!inFile.read(reinterpret_cast<char*>(&event.flags), sizeof(event.flags))) break;

            if (fileHasWaveforms) {
                if (!inFile.read(reinterpret_cast<char*>(&event.waveformCode), sizeof(event.waveformCode))) break;
                if (!inFile.read(reinterpret_cast<char*>(&event.nSamples), sizeof(event.nSamples))) break;

                if (event.nSamples > 0) {
                    event.waveform.resize(event.nSamples);
                    if (!inFile.read(reinterpret_cast<char*>(event.waveform.data()), event.nSamples * sizeof(int16_t))) break;
                }
            }

            queue.push(event);
        }
        inFile.close();
    }

    TFile outFile(rootFilename.c_str(), "RECREATE");
    TTree outTree("events", "events");

    int16_t board, channel, energyLong, energyShort;
    int64_t timestamp;
    int32_t flags, nSamples;
    vector<int16_t> waveform(10000, 0); // Fixed size buffer to avoid overflows

    outTree.Branch("board", &board, "board/S");
    outTree.Branch("channel", &channel, "channel/S");
    outTree.Branch("timestamp", &timestamp, "timestamp/L");
    outTree.Branch("energyLong", &energyLong, "energyLong/S");
    outTree.Branch("energyShort", &energyShort, "energyShort/S");
    outTree.Branch("flags", &flags, "flags/I");
    outTree.Branch("nSamples", &nSamples, "nSamples/I");
    outTree.Branch("waveform", waveform.data(), "waveform[nSamples]/S");

    while (!queue.empty()) {
        RawEvent event = queue.top();
        queue.pop();

        board = event.board;
        channel = event.channel;
        timestamp = event.timestamp;
        energyLong = event.energyLong;
        energyShort = event.energyShort;
        flags = event.flags;
        nSamples = event.nSamples;

        if (!event.waveform.empty()) {
            copy(event.waveform.begin(), event.waveform.begin() + min(nSamples, 10000), waveform.begin());
        }

        outTree.Fill();
    }

    outTree.Write();
    outFile.Close();
}
