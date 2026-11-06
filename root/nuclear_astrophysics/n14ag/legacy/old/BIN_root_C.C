#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "TFile.h"
#include "TTree.h"
#include "TH1F.h"

typedef struct {
    uint16_t board_id;
    uint16_t channel_id;
    uint64_t timestamp;
    uint16_t energy;
    double energy_kev;
    uint32_t flag;
    uint16_t* waveform;
    uint32_t waveform_length;
} Event;

void read_dpp_pha_bin(const char* file_path, TTree* tree) {
    FILE* f = fopen(file_path, "rb");
    if (!f) {
        printf("Error opening file: %s\n", file_path);
        return;
    }

    uint16_t header;
    fread(&header, sizeof(uint16_t), 1, f);
    
    if ((header & 0xFF00) != 0xCA00) {
        printf("Invalid file format\n");
        fclose(f);
        return;
    }

    Event event;
    event.waveform = NULL;

    while (!feof(f)) {
        size_t read_size;
        
        read_size = fread(&event.board_id, sizeof(uint16_t), 1, f);
        if (read_size != 1) break;
        
        read_size = fread(&event.channel_id, sizeof(uint16_t), 1, f);
        if (read_size != 1) break;
        
        read_size = fread(&event.timestamp, sizeof(uint64_t), 1, f);
        if (read_size != 1) break;

        if (header & 0x0001) {
            read_size = fread(&event.energy, sizeof(uint16_t), 1, f);
            if (read_size != 1) break;
        } else {
            event.energy = 0;
        }

        if (header & 0x0002) {
            read_size = fread(&event.energy_kev, sizeof(double), 1, f);
            if (read_size != 1) break;
        } else {
            event.energy_kev = 0.0;
        }

        read_size = fread(&event.flag, sizeof(uint32_t), 1, f);
        if (read_size != 1) break;

        if (header & 0x0008) {
            uint8_t waveform_code;
            read_size = fread(&waveform_code, sizeof(uint8_t), 1, f);
            if (read_size != 1) break;

            read_size = fread(&event.waveform_length, sizeof(uint32_t), 1, f);
            if (read_size != 1) break;

            event.waveform = (uint16_t*)malloc(event.waveform_length * sizeof(uint16_t));
            read_size = fread(event.waveform, sizeof(uint16_t), event.waveform_length, f);
            if (read_size != event.waveform_length) break;
        } else {
            event.waveform = NULL;
            event.waveform_length = 0;
        }

        tree->Fill();

        if (event.waveform) {
            free(event.waveform);
            event.waveform = NULL;
        }
    }

    fclose(f);
}

int BIN_root_C(int argc, char** argv) {
    if (argc != 2) {
        printf("Usage: %s <run_number>\n", argv[0]);
        return 1;
    }

    int run_number = atoi(argv[1]);
    
    char bin_file_path[256];
    char root_file_path[256];
    sprintf(bin_file_path, "/felsdata/2024_10_SetupAndTests/Runs_DAQ2/since_Run4171/DAQ/Run%d_high/RAW/DataR_Run%d_high.BIN", 
            run_number, run_number);
    sprintf(root_file_path, "/felsdata/Gastarget/Measurements/n14_ag/Run_%d_output.root", run_number);

    TFile* outFile = new TFile(root_file_path, "RECREATE");
    TTree* tree = new TTree("tree", "DPP-PHA Data");

    Event event;
    tree->Branch("board_id", &event.board_id, "board_id/s");
    tree->Branch("channel_id", &event.channel_id, "channel_id/s");
    tree->Branch("timestamp", &event.timestamp, "timestamp/l");
    tree->Branch("energy", &event.energy, "energy/s");
    tree->Branch("energy_kev", &event.energy_kev, "energy_kev/D");
    tree->Branch("flag", &event.flag, "flag/i");
    tree->Branch("waveform", &event.waveform, "waveform[waveform_length]/s");
    tree->Branch("waveform_length", &event.waveform_length, "waveform_length/i");

    read_dpp_pha_bin(bin_file_path, tree);

    TH1F* hEnergy = new TH1F("hEnergy", "Energy Spectrum", 65536, 0, 65536);
    tree->Draw("energy>>hEnergy");

    outFile->Write();
    outFile->Close();

    return 0;
}