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

    ClassDef(TPLEIADESDetChan, 1)
};

//------------------------------------------------------------------------
// TPLEIADESNormPos is a dependent class on TPLEIADESDetector. It is a special class for the normalised position measurement of the DSSD.
//------------------------------------------------------------------------

class TPLEIADESNormPos : public TGo4EventElement
{
    public:
        TPLEIADESNormPos();
        TPLEIADESNormPos(const char* name, Short_t id=0);
        virtual ~TPLEIADESNormPos();

        /** Method called by the framework to clear the event element. **/
        void Clear(Option_t *opt = "");

    ClassDef(TPLEIADESNormPos, 1)
};

//------------------------------------------------------------------------
// TPLEIADESDector is a dependent class on TPLEIADESDetEvent. It represents a physical detector.
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

        /** Method called by the framework to clear the event element. **/
        void Clear(Option_t *opt = "");

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
        TPLEIADESDetector* GetDetector(TString dname);

        /** this array keeps the unique names of configured detectors **/
        static std::vector<TString> fgConfigDetectors;

    ClassDef(TPLEIADESDetEvent, 1)
};

#endif // TPLEIADESDETEVENT_H

//----------------------------END OF GO4 SOURCE FILE ---------------------
