//------------------------------------------------------------------------
//************************ TPLEIADESAnalysis.cxx ************************
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

#include "TPLEIADESAnalysis.h"

#include "TGo4Version.h"
#include "TGo4Log.h"
#include "TGo4StepFactory.h"
#include "TGo4AnalysisStep.h"


//------------------------------------------------------------------------
TPLEIADESAnalysis::TPLEIADESAnalysis() : TGo4Analysis()
{

}

//------------------------------------------------------------------------
// this constructor is called by go4analysis executable
TPLEIADESAnalysis::TPLEIADESAnalysis(int argc, char **argv) :
    TGo4Analysis(argc, argv)
{
    if(!TGo4Version::CheckVersion(__GO4BUILDVERSION__))
    {
        TGo4Log::Error("Go4 version mismatch");
        exit(-1);
    }

    TGo4Log::Info("Create TPLEIADESAnalysis name: %s", argv[0]);

    // Create step 1: RawEvent unpacking
    TGo4StepFactory* factory1 = new TGo4StepFactory("Raw Unpacking Factory");
    factory1->DefEventProcessor("PLEIADESRawProc","TPLEIADESRawProc");   // object name, class name
    factory1->DefOutputEvent("PLEIADESRawEvent","TPLEIADESRawEvent");    // object name, class name

    TGo4EventSourceParameter *sourcepar = new TGo4MbsFileParameter(GetDefaultTestFileName());

    TString parname = TString::Format("%sOutput", argv[0]);
    TGo4FileStoreParameter* storepar = new TGo4FileStoreParameter(parname.Data());
    storepar->SetOverwriteMode(kTRUE);

    TGo4AnalysisStep *step1 = new TGo4AnalysisStep("Raw Unpacking", factory1, sourcepar, storepar);
    step1->SetSourceEnabled(kTRUE);
    step1->SetStoreEnabled(kFALSE);
    step1->SetProcessEnabled(kTRUE);
    step1->SetErrorStopEnabled(kTRUE);
    AddAnalysisStep(step1);

    // Create step 2: Detector-based event building
    TGo4StepFactory *factory2 = new TGo4StepFactory("Det Event Factory");
    factory2->DefInputEvent("PLEIADESRawEvent","TPLEIADESRawEvent");    // object name, class name
    factory2->DefEventProcessor("PLEIADESDetProc","TPLEIADESDetProc");  // object name, class name
    factory2->DefOutputEvent("PLEIADESDetEvent","TPLEIADESDetEvent");   // object name, class name

    TGo4AnalysisStep *step2 = new TGo4AnalysisStep("Det Event Building", factory2, nullptr, nullptr);
    step2->SetSourceEnabled(kFALSE);
    step2->SetStoreEnabled(kFALSE);
    step2->SetProcessEnabled(kTRUE);
    step2->SetErrorStopEnabled(kTRUE);
    AddAnalysisStep(step2);

    // uncomment following line to define custom passwords for analysis server
    // DefineServerPasswords("PLEIADESadmin", "PLEIADESctrl", "PLEIADESview");
}

//------------------------------------------------------------------------
TPLEIADESAnalysis::~TPLEIADESAnalysis()
{
    TGo4Log::Info("TPLEIADESAnalysis: Delete instance");
}

//------------------------------------------------------------------------
Int_t TPLEIADESAnalysis::UserPreLoop()
{
    // all this is optional:
    TGo4Log::Info("TPLEIADESAnalysis: PreLoop");
    // get pointer to input event (used in postloop and event function):
    fMbsEvent = dynamic_cast<TGo4MbsEvent *>(GetInputEvent("Raw Unpacking"));   // of first step
    if(fMbsEvent) fMbsEvent->PrintMbsFileHeader();
        // fileheader structure (lmd file only):

    fEvents = 0; // event counter
    fLastEvent = 0; // number of last event processed
    return 0;
}

//------------------------------------------------------------------------
Int_t TPLEIADESAnalysis::UserPostLoop()
{
    // all this is optional:
    TGo4Log::Info("TPLEIADESAnalysis: PostLoop");
    TGo4Log::Info("Last event  #: %d Total events: %d", fLastEvent, fEvents);
    fMbsEvent = nullptr; // reset to avoid invalid pointer if analysis is changed in between
    fEvents = 0;
    return 0;
}

//------------------------------------------------------------------------
Int_t TPLEIADESAnalysis::UserEventFunc()
{
    // all this is optional:
    // This function is called once for each event after all steps.
    if(fMbsEvent)
    {
        fEvents++;
        fLastEvent = fMbsEvent->GetCount();
    }

    if((fEvents == 1) || IsNewInputFile())
    {
        TGo4Log::Info("First event #: %d", fLastEvent);
    }
    return 0;
}

//----------------------------END OF GO4 SOURCE FILE ---------------------
