//------------------------------------------------------------------------
//*********************** TPLEIADESDetEvent.cxx **************************
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

#include "TPLEIADESDetEvent.h"

#include "TGo4Log.h"

//------------------------------------------------------------------------
// TPLEIADESDetChan is a dependent class on TPLEIADESDetector. It represents an output channel on the detector.
//------------------------------------------------------------------------

TPLEIADESDetChan::TPLEIADESDetChan() :
    TGo4EventElement()
{
    TGo4Log::Info("TPLEIADESDetChan: Create instance");
}

TPLEIADESDetChan::TPLEIADESDetChan(const char* name, Short_t id) :
    TGo4EventElement(name, name, id)
{
    TGo4Log::Info("TPLEIADESDetChan: Create instance %s with composite ID %d", name, id);
}

TPLEIADESDetChan::~TPLEIADESDetChan()
{
    TGo4Log::Info("TPLEIADESDetChan: Delete instance");
}

void TPLEIADESDetChan::Clear(Option_t *opt)
{

}


//------------------------------------------------------------------------
// TPLEIADESNormPos is a dependent class on TPLEIADESDetector. It is a special class for the normalised position measurement of the DSSD.
//------------------------------------------------------------------------

TPLEIADESNormPos::TPLEIADESNormPos() :
    TGo4EventElement()
{
    TGo4Log::Info("TPLEIADESNormPos: Create instance");
}

TPLEIADESNormPos::TPLEIADESNormPos(const char* name, Short_t id) :
    TGo4EventElement(name, name, id)
{
    TGo4Log::Info("TPLEIADESNormPos: Create instance %s with composite ID %d", name, id);
}

TPLEIADESNormPos::~TPLEIADESNormPos()
{
    TGo4Log::Info("TPLEIADESNormPos: Delete instance");
}

void TPLEIADESNormPos::Clear(Option_t *opt)
{

}

//------------------------------------------------------------------------
// TPLEIADESDector is a dependent class on TPLEIADESDetEvent. It represents a physical detector.
//------------------------------------------------------------------------

TPLEIADESDetector::TPLEIADESDetector() :
    TGo4CompositeEvent()
{
    TGo4Log::Info("TPLEIADESDetector: Create instance");
}

TPLEIADESDetector::TPLEIADESDetector(const char* name, Short_t id) :
    TGo4CompositeEvent(name, name, id)
{
    TGo4Log::Info("TPLEIADESDetector: Create instance %s with composite ID %d", name, id);
}

TPLEIADESDetector::~TPLEIADESDetector()
{
    TGo4Log::Info("TPLEIADESDetector: Delete instance");
}

void TPLEIADESDetEvent::Clear(Option_t *opt)
{
    TGo4CompositeEvent::Clear();
}


//------------------------------------------------------------------------
// TPLEIADESDetEvent is the base class for detector grouping
//------------------------------------------------------------------------

TPLEIADESDetEvent::TPLEIADESDetEvent() :
    TGo4CompositeEvent()
{
    TGo4Log::Info("TPLEIADESDetEvent: Create instance");
}

TPLEIADESDetEvent::TPLEIADESDetEvent(const char* name, Short_t id) :
    TGo4CompositeEvent(name, name, id)
{
    TGo4Log::Info("TPLEIADESDetEvent: Create instance %s with composite ID %d", name, id);
}

TPLEIADESDetEvent::~TPLEIADESDetEvent()
{
    TGo4Log::Info("TPLEIADESDetEvent: Delete instance");
}

void TPLEIADESDetEvent::Clear(Option_t *opt)
{
    TGo4CompositeEvent::Clear();
}

//----------------------------END OF GO4 SOURCE FILE ---------------------
