#include "TROOT.h"
#include "TFile.h"
#include "TTree.h"

#include <iostream>
using std::cout;
using std::cerr;
using std::endl;

#include <fstream>
using std::ifstream;

#include <string>
using std::string;

#include <vector>
using std::vector;

#include <queue>
using std::priority_queue;

#include "CoMPASSDataDirectory.hh"


struct RawEvent {
    int16_t board;
    int16_t channel;
    int64_t timestamp;
    int16_t energyLong;
    //float64_t energyMeV; // NOT IMPLEMENTED!
    int16_t energyShort;
    int32_t flags;
    int8_t waveformCode;
    int32_t nSamples;
    int16_t* waveform = nullptr;
};

/// Auxiliary struct, used to order RawEvent by timestamp
struct RawEventTimeComparison
{
    bool operator()(const RawEvent &lhs, const RawEvent &rhs)
    {
        return lhs.timestamp > rhs.timestamp;
    }
};

typedef priority_queue<RawEvent, vector<RawEvent>, RawEventTimeComparison> RawEventQueue;


int Axel_ParseBinaryFiles(const TString inputDirectory, const TString outFileName = "./out.root")
{
    const bool writeWaveforms = true;

    ROOT::EnableImplicitMT();
    
    auto cdr = CoMPASS::DataDirectory(inputDirectory.Data());
    const auto files = cdr.GetListOfFilePaths();
    auto nextFile = files.begin();
    
    if (files.size() == 0)
    {
        cerr << "No list mode files found in directory." << endl;
        return 1;
    }
    
    ifstream inFile;
    
    inFile.open(*nextFile, ios::binary | ios::in);   
    if (!inFile.good())
    {
        cerr << "Failed to open " << *nextFile << endl;
        return 1;
    }
    nextFile += 1;
    
    // Read and check header
    //
    bool fileHasEnergyLong;
    bool fileHasEnergyMeV;
    bool fileHasEnergyShort;
    bool fileHasWaveforms;
    //
    {
        uint16_t header;

        inFile.read(reinterpret_cast<char*>(&header), 2);
        if (inFile.gcount() != 2)
        {
            cerr << "Failed to read header." << endl;
            return 1;
        }
        
        if ((header >> 4) != 0xCAE)
        {
            cerr << "Binary file header does not start with 0xCAEX!" << endl;
            return 1;
        }

        auto options = (header % 0xCAE0);

        fileHasEnergyLong  = ((options & (1 << 0)) != 0);
        if (fileHasEnergyLong) {
            cout << "File has Elong." << endl;
        }
        
        fileHasEnergyMeV   = ((options & (1 << 1)) != 0);
        if (fileHasEnergyMeV) {
            cout << "File has Ecal." << endl;
        }

        fileHasEnergyShort = ((options & (1 << 2)) != 0);
        if (fileHasEnergyShort) {
            cout << "File has Eshort." << endl;
        }

        fileHasWaveforms   = ((options & (1 << 3)) != 0);
        if (fileHasWaveforms) {
            cout << "File has Waveforms." << endl;
        }

    }
    
    RawEvent currentEvent, outEvent;
    const int32_t maxWaveformSize = 10000;
    int16_t outputWaveform[maxWaveformSize];
    
    streamsize b1, b2;
    bool endOfFile = false;
    unsigned long iEvent = 0;
    
    TFile outFile(outFileName, "RECREATE", "");//, ROOT::RCompressionSetting::EDefaults::EValues::kUseSmallest);
    
    TTree outTree("events", "events");
        outTree.Branch("board", &outEvent.board, "board/S");
        outTree.Branch("channel", &outEvent.channel, "channel/S");
        outTree.Branch("timestamp", &outEvent.timestamp, "timestamp/L");
        if (fileHasEnergyLong) {
            outTree.Branch("E", &outEvent.energyLong, "E/S");
        }
        if (fileHasEnergyShort) {
            outTree.Branch("Eshort", &outEvent.energyShort, "Eshort/S");
        }
        outTree.Branch("flags", &outEvent.flags, "flags/I");
        if (fileHasWaveforms && writeWaveforms) {
            outTree.Branch("nSamples", &outEvent.nSamples, "nSamples/I");
            outTree.Branch("waveform", outputWaveform, "waveform[nSamples]/S");
        }
    
    const long long GB = 1000000000LL; // 1e9 byte
    outTree.SetMaxTreeSize(1*GB);
    
    int64_t previousTimestamp = 0;
    const size_t queueSize = 1024*32;
    RawEventQueue queue;
        
    while (true)
    {
        iEvent += 1;
        // cout << iEvent << ".." << endl;
        
        inFile.read(reinterpret_cast<char*>(&currentEvent.board), 2);
        b1 = inFile.gcount();
        
        inFile.read(reinterpret_cast<char*>(&currentEvent.channel), 2);
        inFile.read(reinterpret_cast<char*>(&currentEvent.timestamp), 8);
        
        if (fileHasEnergyLong) {
            inFile.read(reinterpret_cast<char*>(&currentEvent.energyLong), 2);
        }
        if (fileHasEnergyMeV) {
            inFile.ignore(4); // NOT IMPLEMENTED: skipping over this data
        }
        if (fileHasEnergyShort) {
            inFile.read(reinterpret_cast<char*>(&currentEvent.energyShort), 2);
        }
        
        inFile.read(reinterpret_cast<char*>(&currentEvent.flags), 4);
        
        if (fileHasWaveforms)
        {
            inFile.read(reinterpret_cast<char*>(&currentEvent.waveformCode), 1);
            inFile.read(reinterpret_cast<char*>(&currentEvent.nSamples), 4);
            
            if (currentEvent.nSamples > maxWaveformSize)
            {
                cerr << "Found waveform too large to fit into buffer. Please increase buffer size." << endl;
                cerr << "Waveform: " << currentEvent.nSamples << endl;
                cerr << "Buffer: " << maxWaveformSize << endl;
                return 2;
            }
            
            currentEvent.waveform = new int16_t[currentEvent.nSamples];
            
            inFile.read(reinterpret_cast<char*>(currentEvent.waveform), 2*currentEvent.nSamples);
            //inFile.ignore(2*currentEvent.nSamples);
            
            if (iEvent == 1 && fileHasWaveforms)
            {
                cout << std::hex << currentEvent.waveform[0] << " "
                     << std::hex << currentEvent.waveform[1] << " "
                     << std::hex << currentEvent.waveform[2] << " "
                     << std::hex << currentEvent.waveform[3] << " "
                     << endl;

                cout << std::dec << currentEvent.waveform[0] << " "
                     << std::dec << currentEvent.waveform[1] << " "
                     << std::dec << currentEvent.waveform[2] << " "
                     << std::dec << currentEvent.waveform[3] << " "
                     << endl;
            }
        }

        b2 = inFile.gcount();
        
        if (iEvent <= 10)
        {
            cout << currentEvent.board << "/" << currentEvent.channel << " (" << currentEvent.timestamp << "): " << currentEvent.energyLong << endl;
        }
        
        endOfFile = (b2 == 0);
        
        if (endOfFile && b1 == 0)
        {
            cout << "Reached end of file." << endl;
            inFile.close();

            cout << "outTree Entries: " << outTree.GetEntries() << endl;
            
            if (nextFile == files.end())
            {
                cout << "This was the last file. Finishing up." << endl;
                break;
            }
            
            //if (iEvent > 58254222+10)
            //    break; // DEBUG
            
            inFile.open(*nextFile, ios::binary | ios::in);   
            if (!inFile.good())
            {
                cerr << "Failed to open " << *nextFile << endl;
                return 1;
            }
            else
            {
                cout << "Opened " << *nextFile << "." << endl;
            }
            nextFile += 1;
            
            continue;
        }
        
        if (endOfFile && b1 != 0)
        {
            cerr << "Reached end of file. Last event incomplete!" << endl;
            break;
        }
        
        // Queue is filled up. Take oldest event and write to file.
        if (queue.size() == queueSize)
        {
            outEvent = queue.top();
            queue.pop();
            if (outEvent.timestamp < previousTimestamp)
            {
                cerr << "Time stamps were not in order!" << endl;
                return 2;
            }
            previousTimestamp = outEvent.timestamp;
            if (fileHasWaveforms && writeWaveforms) {
                memcpy(outputWaveform, outEvent.waveform, 2*outEvent.nSamples);
            }
            outTree.Fill();
            
            delete[] outEvent.waveform;
        }
        // then fill current event from the file into the queue.
        queue.push(currentEvent);
    }
    
    // Write remaining events from queue to file
    while (!queue.empty())
    {
        outEvent = queue.top();
        queue.pop();
        
        if (outEvent.timestamp < previousTimestamp)
        {
            cerr << "Time stamps were not in order!" << endl;
            return 2;
        }
        previousTimestamp = outEvent.timestamp;        
        /*        cout << ".." << endl
                     << std::hex << outEvent.waveform[0] << " "
                     << std::hex << outEvent.waveform[1] << " "
                     << std::hex << outEvent.waveform[2] << " "
                     << std::hex << outEvent.waveform[3] << " "
                     << endl;        
        */
        if (fileHasWaveforms && writeWaveforms) {
            memcpy(outputWaveform, outEvent.waveform, 2*outEvent.nSamples);
        }
        outTree.Fill();
        delete[] outEvent.waveform;
    }

    auto *cFile = outTree.GetCurrentFile();
    cFile->Write();
    cFile->Close();
    
    return 0;
    
    /*
    for (const auto &f : files)
    {
        cout << f << endl;
    }
    
    return 0; 
    */
}
