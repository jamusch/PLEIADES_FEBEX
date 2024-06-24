//------------------------------------------------------------------------
//************************ TPLEIADESDetEvent.h ***************************
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

#ifndef TPLEIADESDETEVENT_H
#define TPLEIADESDETEVENT_H

#include "TGo4CompositeEvent.h"
#include "TPLEIADESRawEvent.h"

class TPLEIADESParam;
class TPLEIADESDetDisplay;

//------------------------------------------------------------------------
// TPLEIADESDetChan is a dependent class on TPLEIADESDetector. It represents an output channel on the detector.
//------------------------------------------------------------------------

class TPLEIADESDetChan : public TGo4EventElement
{
    public:
        TPLEIADESDetChan();
        TPLEIADESDetChan(const char* name, Short_t id=0);
        virtual ~TPLEIADESDetChan();

        /** Method called by the framework to clear the event element. **/
        void Clear(Option_t *opt = "");

        /** actions on unique name of detector **/
        void SetDetName(TString dname) { fDetName = dname ; }
        TString GetDetName() { return fDetName; }

        /** actions on Go4EventElement ID of detector **/
        void SetDetId(Short_t id) { fDetId = id ; }
        Short_t GetDetId() { return fDetId; }

        /** actions on type of detector **/
        void SetDetType(TString dtype) { fDetType = dtype; }
        TString GetDetType() { return fDetType; }

        /** actions on unique channel mapping **/
        void SetChanMap(UInt_t map) { fUniqChanMap = map; }
        UInt_t GetChanMap() { return fUniqChanMap; }

        /** actions on channel type **/
        void SetChanType(TString type) { fChanType = type; }
        TString GetChanType() { return fChanType; }

        /** FEBEX special channel properties **/
        UInt_t fDHitMultiplicity;
        UInt_t fDFPGAEnergy;
        UInt_t fDFPGAHitTime;
        std::vector<Double_t> fDFPGATRAPEZ;

        /** FEBEX trace properties **/
        #ifdef TPLEIADES_FILL_TRACES
        Int_t fDTrapezEnergy;
        std::vector<Double_t>   fDTrace;
        std::vector<Double_t>   fDTraceBLR;
        std::vector<Double_t>   fDTraceTRAPEZ;
        #endif

    private:
        TString fDetName;       // detector name setup in set_PLEIADESParam.C
        Short_t fDetId;         // Go4EventElement id of detector
        TString fDetType;       // detector type
        TString fChanType;      // type of channel
        UInt_t fUniqChanMap;    // channel map to DAQ position

    ClassDef(TPLEIADESDetChan, 1)
};

//------------------------------------------------------------------------
// TPLEIADESDetector is a dependent class on TPLEIADESDetEvent. It represents a detector with unprocessed values.
//------------------------------------------------------------------------

class TPLEIADESDetector : public TGo4CompositeEvent
{
    public:
        TPLEIADESDetector();
        TPLEIADESDetector(const char* name, Short_t id=0);
        virtual ~TPLEIADESDetector();

        /** actions on unique name of detector **/
        void SetDetName(TString dname) { fDetName = dname ; }
        TString GetDetName() { return fDetName; }

        /** actions on type of detector **/
        void SetDetType(TString dtype) { fDetType = dtype; }
        TString GetDetType() { return fDetType; }

        /** setup detector based on name and type **/
        void SetupDetector();      // setup channels based on name and type

        /** get channel objects created with board **/
        TPLEIADESDetChan* GetChannel(TString chname);
        TPLEIADESDetChan* GetChannel(UInt_t id) { return (TPLEIADESDetChan*) getEventElement(id); }

        /** Method called by the framework to clear the event element. **/
        void Clear(Option_t *opt = "");

        static TPLEIADESParam* fParDet; // required to use fPar in DetEvent construction

    private:
        TString fDetName;       // detector name setup in set_PLEIADESParam.C
        TString fDetType;       // detector type

    ClassDef(TPLEIADESDetector, 1)
};

//------------------------------------------------------------------------
// TPLEIADESDetEvent is the base class for detector grouping
//------------------------------------------------------------------------

class TPLEIADESDetEvent : public TGo4CompositeEvent
{
    public:
        TPLEIADESDetEvent();
        TPLEIADESDetEvent(const char* name, Short_t id=0);
        virtual ~TPLEIADESDetEvent();

        /** Method called by the framework to clear the event element. **/
        void Clear(Option_t *opt = "");

        /** access to board subcomponent by unique id **/
        void BuildDetectors();      // build detectors with name and type
        TPLEIADESDetector* GetDetector(TString dname);      //return detector by name

        /** this array keeps the unique names of configured detectors **/
        static std::vector<TString> fgConfigDetectors;

        static TPLEIADESParam* fParDEv; // required to use fPar in DetEvent construction

        std::vector<TPLEIADESDetDisplay*> fDetDisplays;

        Int_t fSequenceNumber;  // tracks MBS event number
        Bool_t fPhysTrigger;    // tracks MBS physics trigger

    ClassDef(TPLEIADESDetEvent, 1)
};

#endif // TPLEIADESDETEVENT_H

//----------------------------END OF GO4 HEADER FILE ---------------------
