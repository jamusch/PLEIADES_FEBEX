//------------------------------------------------------------------------
//************************ TPLEIADESRawEvent.cxx *************************
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

#ifndef TPLEIADESRAWEVENT_H
#define TPLEIADESRAWEVENT_H

// disable this define to speed up analysis, only histogram filling then
#define TPLEIADES_FILL_TRACES 1

#include "TGo4EventElement.h"
#include "TGo4CompositeEvent.h"


#define MAX_SFP          4
#define MAX_SLAVE        5
#define N_CHA            16


//-----------------------------------------------------------------------
// this is the lowest class for the individual FEBEX channel readouts
//-----------------------------------------------------------------------

class TPLEIADESFebChannel : public TGo4EventElement
{
    public:
        TPLEIADESFebChannel();
        TPLEIADESFebChannel(const char *name, Short_t id);
        virtual ~TPLEIADESFebChannel();

        /** Method called by the framework to clear the event element. */
        void Clear(Option_t *opt = "") override;

        /** FEBEX special channel properties **/
        UInt_t fRPolarity;
        UInt_t fRHitMultiplicity;
        Int_t  fRFPGAEnergy;
        Int_t  fRFPGAHitTime;
        std::vector<Double_t> fRFPGATRAPEZ;

        /** FEBEX trace properties **/
        #ifdef TPLEIADES_FILL_TRACES
        Int_t fRTrapezEnergy;
        std::vector<Double_t>   fRTrace;
        std::vector<Double_t>   fRTraceBLR;
        std::vector<Double_t>   fRTraceTRAPEZ;
        #endif

    ClassDefOverride(TPLEIADESFebChannel,1)
};


//-----------------------------------------------------------------------
// this class represents one FEBEX-3 board with 16 Feb Channels
//-----------------------------------------------------------------------

class TPLEIADESFebBoard : public TGo4CompositeEvent
{
     public:
        TPLEIADESFebBoard();
        TPLEIADESFebBoard(const char *name, UInt_t unid, Short_t id);
        virtual ~TPLEIADESFebBoard();

        /** get unique ID of board in setup **/
        UInt_t GetBoardID() { return fUniqueId; }

        /** get channel objects created with board **/
        TPLEIADESFebChannel* GetChannel(UInt_t id)
        {
            return (TPLEIADESFebChannel*) getEventElement(id);
        }

        /** Method called by the framework to clear the event element. */
        void Clear(Option_t *opt = "") override;

        Int_t GetLastEventNumber() { return fLastEventNumber; }
        void SetLastEventNumber(Int_t num) { fLastEventNumber = num; }

    private:
        UInt_t fUniqueId;           // unique id of the board, as set by TPLEIADESParam::SetConfigBoards
        Int_t  fLastEventNumber;    // check sequence number of events and report missing events

    ClassDefOverride(TPLEIADESFebBoard,1)
};


//-----------------------------------------------------------------------
// this is the top event structure with all FEBEX boards in the chain
//-----------------------------------------------------------------------

class TPLEIADESRawEvent : public TGo4CompositeEvent
{
    public:
        TPLEIADESRawEvent();
        TPLEIADESRawEvent(const char *name, Short_t id=0);
        virtual ~TPLEIADESRawEvent();

        /** Method called by the framework to clear the event element. **/
        void Clear(Option_t *opt = "") override;

        /** access to board subcomponent by unique id **/
        TPLEIADESFebBoard* GetBoard(UInt_t unid);

        /** this array keeps the unique id numbers of configured FEBEX boards **/
        static std::vector<UInt_t> fgConfigBoards;

        Int_t fSequenceNumber;      // event sequence number incremented by MBS Trigger
        Bool_t fPhysTrigger;        // was event a physics trigger

    ClassDefOverride(TPLEIADESRawEvent,1)
};

#endif //TPLEIADESRAWEVENT_H

//----------------------------END OF GO4 HEADER FILE ---------------------

