//------------------------------------------------------------------------
//************************ TPLEIADESPhysEvent.h ***************************
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

#ifndef TPLEIADESPHYSEVENT_H
#define TPLEIADESPHYSEVENT_H

#include "TGo4CompositeEvent.h"

class TPLEIADESParam;

//------------------------------------------------------------------------
// TPLEIADESDetPhysics is a dependent class on TPLEIADESPhysEvent. It represents a detector with physics-processed values.
//------------------------------------------------------------------------

class TPLEIADESDetPhysics: public TGo4EventElement
{
    public:
        TPLEIADESDetPhysics();
        TPLEIADESDetPhysics(const char* name, Short_t id=0);
        virtual ~TPLEIADESDetPhysics();

        /** Method called by the framework to clear the event element. **/
        void Clear(Option_t *opt = "");

        /** actions on unique name of detector **/
        void SetDetName(TString dname) { fDetName = dname ; }
        TString GetDetName() { return fDetName; }

        /** actions on type of detector **/
        void SetDetType(TString dtype) { fDetType = dtype; }
        TString GetDetType() { return fDetType; }

        /** physics properties of detector **/
        Double_t fpEnergy  = -99;
        Double_t fnEnergy  = -99;
        Double_t fNormPosX = -99;
        Double_t fNormPosY = -99;

    private:
        TString fDetName;       // detector name setup in set_PLEIADESParam.C
        TString fDetType;       // detector type

    ClassDef(TPLEIADESDetPhysics, 1)
};

//------------------------------------------------------------------------
// TPLEIADESBasEvent is the base class for detector grouping
//------------------------------------------------------------------------

class TPLEIADESPhysEvent : public TGo4CompositeEvent
{
    public:
        TPLEIADESPhysEvent();
        TPLEIADESPhysEvent(const char* name, Short_t id=0);
        virtual ~TPLEIADESPhysEvent();

        /** Method called by the framework to clear the event element. **/
        void Clear(Option_t *opt = "");

        /** access to board subcomponent by unique id **/
        void BuildDetectors();      // build detectors with name and type
        TPLEIADESDetPhysics* GetDetector(TString dname);      //return detector by name

        /** this array keeps the unique names of configured detectors **/
        static std::vector<TString> fgConfigDetectors;

        static TPLEIADESParam* fParPEv; // required to use fPar in PhysEvent construction

    ClassDef(TPLEIADESPhysEvent, 1)
};

#endif // TPLEIADESPHYSEVENT_H

//----------------------------END OF GO4 HEADER FILE ---------------------
