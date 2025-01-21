//------------------------------------------------------------------------
//************************ TPLEIADESDisplay.h ****************************
//------------------------------------------------------------------------
//       The GSI Online Offline Object Oriented (Go4) Project
//         Experiment Data Processing at EE department, GSI
//------------------------------------------------------------------------
// Copyright (C) 2000- GSI Helmholtzzentrum fuer Schwerionenforschung GmbH
//                     Planckstr. 1, 64291 Darmstadt, Germany
// Contact:            http://go4.gsi.de
//------------------------------------------------------------------------
// This software can be used under the license agreements as stated
// in Go4License.txt file which is part of the distribution.
//------------------------------------------------------------------------

#ifndef TPLEIADESDISPLAY_H
#define TPLEIADESDISPLAY_H

#include "TGo4EventProcessor.h"
#include "TPLEIADESDetEvent.h"

#include <vector>

class TPLEIADESParam;
class TPLEIADESChanDisplay;

class TPLEIADESDisplay : public TGo4EventProcessor
{
    public:
        TPLEIADESDisplay();
        TPLEIADESDisplay(const char* name);
        virtual ~TPLEIADESDisplay();

        static TPLEIADESParam* fParDisplay; // required to use fPar in histogram construction

    private:

    ClassDef(TPLEIADESDisplay, 1)
};

class TPLEIADESDetDisplay : public TPLEIADESDisplay
{
    public:
        TPLEIADESDetDisplay();
        TPLEIADESDetDisplay(TPLEIADESDetector* theDetector);
        virtual ~TPLEIADESDetDisplay();

        TPLEIADESDetector* fDetector;

        void AddChanDisplay(TPLEIADESChanDisplay* chanDisplay);
        void ClearChanDisplay();
        Int_t GetNumChannels();
        TPLEIADESChanDisplay* GetChanDisplay(TString name);

        /** init display creates histograms **/
        void InitDisplay();

        TH1* hDetHitPattern;
        TH2* hDetEnergyPattern;

    private:
        std::vector<TPLEIADESChanDisplay*> fChannels;

    ClassDef(TPLEIADESDetDisplay, 1)
};

class TPLEIADESChanDisplay : public TPLEIADESDisplay
{
    public:
        TPLEIADESChanDisplay();
        TPLEIADESChanDisplay(TPLEIADESDetChan* theChannel);
        virtual ~TPLEIADESChanDisplay();

        TPLEIADESDetChan* fChannel;

        TString GetChanName();

        /** histogram functions **/
        void InitDisplay();     // initialises histograms
        void FillTraces();      // overwrites trace histograms

        TH1* hHitMultiplicity;
        TH1* hTraceChan;
        TH1* hTraceBLRChan;
        TH1* hBIBOXFiltChan;
        TH1* hMWDFiltChan;

    ClassDef(TPLEIADESChanDisplay, 1)
};

class TPLEIADESPhysDisplay : public TPLEIADESDisplay
{
    public:
        TPLEIADESPhysDisplay();
        virtual ~TPLEIADESPhysDisplay();

        /** histogram functions **/
        void InitClipStatsHists(TPLEIADESDetEvent* fInEvent);   // initialises clipping statistics histograms
        void InitPHReconHists(TPLEIADESDetEvent* fInEvent);     // initialises pulse height reconstruction histograms

        /** clipping statistics histograms **/
        std::vector<TH1*> hRiseTimeNSides;      TH1* hRiseTimeCrysFr;       TH1* hRiseTimeCrysBk;
        std::vector<TH1*> hRiseTimePSides;
        std::vector<TH1*> hReentryTimeNSides;   TH1* hReentryTimeCrysFr;    TH1* hReentryTimeCrysBk;
        std::vector<TH1*> hPulseTimeNSides;     TH1* hPulseTimeCrysFr;      TH1* hPulseTimeCrysBk;
        std::vector<TH1*> hClipHeightNSides;    TH1* hClipHeightCrysFr;     TH1* hClipHeightCrysBk;
        std::vector<TH1*> hEndHeightNSides;     TH1* hEndHeightCrysFr;      TH1* hEndHeightCrysBk;
        std::vector<TH2*> hClipLenVHghtNSides;  TH2* hClipLenVHghtCrysFr;   TH2* hClipLenVHghtCrysBk;

        /** pulse height recon histograms **/
        std::vector<std::vector<TH1*>> hTOThreshNSides;     std::vector<TH1*> hTOThreshCrysFr;      std::vector<TH1*> hTOThreshCrysBk;
        std::vector<TH1*> hExpFitNSides;        TH1* hExpFitCrysFr;         TH1* hExpFitCrysBk;
        std::vector<std::vector<TH1*>> hExpIntegNSides;     std::vector<TH1*> hExpIntegCrysFr;      std::vector<TH1*> hExpIntegCrysBk;
        std::vector<TH1*> hDecayTimesNSides;    TH1* hDecayTimesCrysFr;     TH1* hDecayTimesCrysBk;

    ClassDef(TPLEIADESPhysDisplay, 1)
};

#endif // TPLEIADESDISPLAY_H

//----------------------------END OF GO4 HEADER FILE ---------------------
