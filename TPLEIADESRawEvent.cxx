//-----------------------------------------------------------------------
//************************* TPLEIADESParam.cxx **************************
//-----------------------------------------------------------------------
//       The GSI Online Offline Object Oriented (Go4) Project
//         Experiment Data Processing at EE department, GSI
//-----------------------------------------------------------------------
// Copyright (C) 2000- GSI Helmholtzzentrum fuer Schwerionenforschung GmbH
//                     Planckstr. 1, 64291 Darmstadt, Germany
// Contact:            http://go4.gsi.de
//-----------------------------------------------------------------------
// This software can be used under the license agreements as stated
// in Go4License.txt file which is part of the distribution.
//-----------------------------------------------------------------------

#include "TPLEIADESRawEvent.h"

#include "TGo4Log.h"

//-----------------------------------------------------------------------
// this is the lowest class for the individual FEBEX channel readouts
//-----------------------------------------------------------------------

TPLEIADESFebChannel::TPLEIADESFebChannel() :
   TGo4EventElement()
{
   TGo4Log::Info("TPLEIADESFebChannel: Create instance");
}

TPLEIADESFebChannel::TPLEIADESFebChannel(const char *name, Short_t index) :
   TGo4EventElement(name, name, index)
{
   TGo4Log::Info("TPLEIADESFebBoard: Create instance %s", name);
}

TPLEIADESFebChannel::~TPLEIADESFebChannel()
{
   TGo4Log::Info("TPLEIADESFebChannel: Delete instance");
}

void TPLEIADESFebChannel::Clear(Option_t *opt)
{
   // all members should be cleared, i.e. assigned to a "not filled" value
   /** FEBEX special channel properties **/
   fFPGAEnergy = 0;
   fFGPAHitTime = 0;
   fFPGATRAPEZ.clear();

   /** FEBEX trace properties **/
#ifdef TPLEIADES_FILL_TRACES
   fTrace.clear();
   fTraceBLR.clear();
   fTraceTRAPEZ.clear();
#endif
}

//-----------------------------------------------------------------------
// this class represents one FEBEX-3 board with 16 Feb Channels
//-----------------------------------------------------------------------

TPLEIADESFebBoard::TPLEIADESFebBoard() :
   TGo4CompositeEvent(),fLastEventNumber(-1)
{
   TGo4Log::Info("TPLEIADESFebBoard: Create instance");
}

TPLEIADESFebBoard::TPLEIADESFebBoard(const char *name, Short_t id) :
   TGo4CompositeEvent(name, name, id), fLastEventNumber(-1)
{
   TGo4Log::Info("TPLEIADESFebBoard: Create instance %s", name);

   //create channels for FEBEX board
   TString modname;
   for (int i=0; i < N_CHA; ++i)
   {
      modname.Form("PLEIADES_Board%02d_Ch%02d", id, i);
      addEventElement(new TPLEIADESFebChannel(modname.Data(), i));
   }
}

TPLEIADESFebBoard::~TPLEIADESFebBoard()
{
   TGo4Log::Info("TPLEIADESFebChannel: Delete instance");
}

void TPLEIADESFebBoard::Clear(Option_t *opt)
{
   TGo4CompositeEvent::Clear();
}

//-----------------------------------------------------------------------
// this is the top event structure with all FEBEX boards in the chain
//-----------------------------------------------------------------------

TPLEIADESRawEvent::TPLEIADESRawEvent() :
   TGo4EventElement()
{
   TGo4Log::Info("TPLEIADESRawEvent: Create instance");
}

TPLEIADESRawEvent::TPLEIADESRawEvent(const char *name) :
   TGo4EventElement(name)
{
   TGo4Log::Info("TPLEIADESRawEvent: Create instance %s", name);

   //create boards for SFP 1
   TString modname;
   for (int i=0; i<MAX_SLAVE; ++i)
   {
      modname.Form("PLEIADES_Board_%02d", i);
      addEventElement(new TPLEIADESFebBoard(modname.Data(), i));
   }
}

TPLEIADESRawEvent::~TPLEIADESRawEvent()
{
   TGo4Log::Info("TPLEIADESRawEvent: Delete instance");
}

TPLEIADESFebBoard* TPLEIADESRawEvent::GetBoard(UInt_t boardId)
{
   TPLEIADESFebBoard* theBoard = 0;
   Short_t numBoards = getNElements();
   for (int i=0; i<numBoards; ++i)
   {
      theBoard = (TPLEIADESFebBoard*) getEventElement(i);
      if (theBoard->GetBoardId() == boardId)
      {
         return theBoard;
      }
   }
   return 0;
}

void TPLEIADESRawEvent::Clear(Option_t *opt)
{
   TGo4CompositeEvent::Clear();
   fSequenceNumber = -1;
}
