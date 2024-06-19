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

class TPLEIADESChanDisplay;

class TPLEIADESDisplay : public TGo4EventProcessor
{
    public:
        TPLEIADESDisplay();
        TPLEIADESDisplay(const char* name);
        virtual ~TPLEIADESDisplay();

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

        /** init display creates histograms **/
        void InitDisplay();

    private:


    ClassDef(TPLEIADESChanDisplay, 1)
};

#endif // TPLEIADESDISPLAY_H

//----------------------------END OF GO4 HEADER FILE ---------------------
