#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <stdint.h>
#include "TFile.h"
#include "TTree.h"
#include "TH1F.h"
#include "TCanvas.h"

struct EventData {
    uint16_t board_id;
    uint16_t channel_id;
    uint64_t timestamp;
    uint16_t energy;
    double energy_kev;
    uint32_t flag;
    std::vector<uint16_t> waveform;
};

void readBinFile() {
    int run_number = 4180;
    std::string binFilename = "/felsdata/2024_10_SetupAndTests/Runs_DAQ2/since_Run4171/DAQ/Run" + std::to_string(run_number) + "_high/RAW/DataR_Run" + std::to_string(run_number) + "_high.BIN";
    std::string rootFilename = "/felsdata/Gastarget/Measurements/n14_ag/Run_" + std::to_string(run_number) + "_high.root";
    std::vector<EventData> events;

    // Read the .bin file
    std::ifstream file(binFilename, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Error opening file: " << binFilename << std::endl;
        return;
    }
    // Read the header (16 bits)
    uint16_t header;
    file.read(reinterpret_cast<char*>(&header), sizeof(header));

    // Check if the file is in the correct format
    if ((header & 0xFF00) != 0xCA00) {
        std::cerr << "Invalid file format: Header does not match DPP_PHA format." << std::endl;
        return;
    }

    // Read the data from the file
    while (file) {
        EventData event;
        // Read board ID (16 bits)
        file.read(reinterpret_cast<char*>(&event.board_id), sizeof(event.board_id));
        // Read channel ID (16 bits)
        file.read(reinterpret_cast<char*>(&event.channel_id), sizeof(event.channel_id));
        // Read timestamp (64 bits)
        file.read(reinterpret_cast<char*>(&event.timestamp), sizeof(event.timestamp));

        // Read energy (16 bits) if bit 0 of the header is set
        if (header & 0x0001) {
            file.read(reinterpret_cast<char*>(&event.energy), sizeof(event.energy));
        }
        // Read energy in keV/MeV (64 bits) if bit 1 of the header is set
        if (header & 0x0002) {
            file.read(reinterpret_cast<char*>(&event.energy_kev), sizeof(event.energy_kev));
        }
        // Read flags (32 bits)
        file.read(reinterpret_cast<char*>(&event.flag), sizeof(event.flag));

        // Read waveform if bit 3 of the header is set
        if (header & 0x0008) {
            uint8_t waveform_code;
            uint32_t num_samples;
            file.read(reinterpret_cast<char*>(&waveform_code), sizeof(waveform_code));
            file.read(reinterpret_cast<char*>(&num_samples), sizeof(num_samples));
            event.waveform.resize(num_samples);
            file.read(reinterpret_cast<char*>(event.waveform.data()), num_samples * sizeof(uint16_t));
        }
        // Add the event to the list if the file read was successful
        if (file) {
            events.push_back(event);
        }
    }

    // Close the binary file stream
    file.close();

    // Create the ROOT file and save the data into a TTree
    TFile rootFile(rootFilename.c_str(), "RECREATE");
    TTree tree("tree", "DPP PHA Data");

    // Define branches in the TTree
    EventData event;
    tree.Branch("board_id", &event.board_id, "board_id/s");
    tree.Branch("channel_id", &event.channel_id, "channel_id/s");
    tree.Branch("timestamp", &event.timestamp, "timestamp/l");
    tree.Branch("energy", &event.energy, "energy/s");
    tree.Branch("energy_kev", &event.energy_kev, "energy_kev/D");
    tree.Branch("flag", &event.flag, "flag/i");
    tree.Branch("waveform", &event.waveform);

    const long long GB = 1000000000LL; // 1e9 byte
    tree.SetMaxTreeSize(1*GB);
    
    // int64_t previousTimestamp = 0;
    // const size_t queueSize = 1024*32;
    // RawEventQueue queue;
    // Fill the TTree with the event data
    for (const auto& e : events) {
        event = e;
        tree.Fill();
    }
    // Write the TTree to the ROOT file and close the file
    rootFile.Write();
    rootFile.Close();

    // Plot the energy histogram
    TH1F* hist = new TH1F("hist", "Energy Histogram", 1000, 0, 6000);

    for (const auto& e : events) {
        hist->Fill(e.energy);
    }

    TCanvas* canvas = new TCanvas("canvas", "Energy Histogram", 800, 600);
    hist->Draw();
    // canvas->SaveAs("energy_histogram.png");
    // delete canvas;
    // delete hist;
}