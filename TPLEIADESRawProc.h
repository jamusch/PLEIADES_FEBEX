//------------------------------------------------------------------------
//************************* TPLEIADESRawProc.h ***************************
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

#ifndef TPLEIADESRAWPROC_H
#define TPLEIADESRAWPROC_H

#include "TGo4EventProcessor.h"
#include "TPLEIADESRawEvent.h"
#include "TF1.h"

#include "stdint.h"

// comment out define statements to turn on/off

// JAM 3-2025: configuration of trigger types of combined system
#define TRIGGER_TYPE_FEBEX 3
#define TRIGGER_TYPE_VME 1


#define WR_TIME_STAMP     1   // white rabbit time stamp is head of data

#ifdef WR_TIME_STAMP
    #define SUB_SYSTEM_ID_FEB1      0x100
    #define SUB_SYSTEM_ID_FEB2      0x200
    #define SUB_SYSTEM_ID_VME       0x400
    #define TS__ID_L16         0x3e1
    #define TS__ID_M16         0x4e1
    #define TS__ID_H16         0x5e1
    #define TS__ID_X16         0x6e1
#endif // WR_TIME_STAMP

#define USE_MBS_PARAM     1

/* ------------------------------------------------
// now these definitions are in TPLEIADESRawEvent.h
#define BIBOX 1            // toggle if BIBOX filter is used
#define MWD    1           // toggle if MWC (moving window deconvolution) filter is used
#define MAX_SFP           4
#define MAX_SLAVE        16
#define N_CHA            16
------------------------------------------------ */

#define NIK_EXTRA_HISTS 1   // toggle to remove Nik's extra histogram
#define DEC_CONST_FIT 1     // toggle if preamp decay constants are fitted

//#define ADC_RES            2000./16384.   // res in mV: +/-1V range by 14-bit ADC
#define ADC_RES            4000./16384.   // res in mV: +/-2V range by 14-bit ADC

#ifdef USE_MBS_PARAM
    #define MAX_TRACE_SIZE   8000   // in samples
    #define MAX_BIBOX_N_AVG  2000   // in samples
#else 
    #define     TRACE_SIZE    3000   // in samples 1024
    #define MAX_TRACE_SIZE TRACE_SIZE // JAM 26-03-25
    //#define     BIBOX_N_AVG    64   // in samples
    // nr of slaves on SFP 0  1  2  3
    //                     |  |  |  |
    #define NR_SLAVES    { 5, 5, 0, 0}
    #define ADC_TYPE     { 0xffff, 0xffff, 0, 0} // 12 bit: 0, 14 bit: 1
                                            // bit 0 fuer slave module 0
                                            // bit n fuer slave module n (max n = 31)

    #define POLARITY {0x00000000, 0x0000ffff, 0x00000000, 0x00000000}
    // for max. 32 FEBEX per SFP, each bit represents a FEBEX
    //  0: positive signals, 1 negative signals
#endif

#define BASE_LINE_SUBT_START  0
#define BASE_LINE_SUBT_SIZE   20

#ifdef BIBOX
    #define BIBOX_N_GAP  200
    #define BIBOX_N_AVG  100
#endif // BIBOX

#ifdef MWD
    #define MWD_WIND    400      // difference length, i.e. width of moving window
    #define MWD_AVG     300      // integration length, i.e. width of low-pass filter. NB: MWD_WIND must be bigger than MWD_AVG
    //#define MWD_TAU    1538   // decay constant for pole-zero correction. now defined on line 189 of TPLEIADESRawProc.cxx due to multiple values needed
    #define MWD_SAMP    570     // sampling point, i.e. bin from which filter is sampled. Should be within the flat top, which ends at MWD_WIND + MWD_AVG.
#endif // MWD

#define RON  "\x1B[7m"
#define RES  "\x1B[0m"

class TPLEIADESParam;
class TH1;

class TPLEIADESRawProc : public TGo4EventProcessor
{
    public:
        TPLEIADESRawProc() ;
        TPLEIADESRawProc(const char* name);
        virtual ~TPLEIADESRawProc() ;
        void f_make_histo (Int_t);

        /** event processing function **/
        Bool_t BuildEvent(TGo4EventElement* target);

    private:
        /** reference to output data **/
        TPLEIADESRawEvent* fOutEvent;  //!

        /** parameter for runtime settings **/
        TPLEIADESParam* fPar;
#ifdef WR_TIME_STAMP
        UInt_t fWR_SubsystemID;
        ULong_t fWR_Timestamp;
        ULong_t fWR_Timestamp_prev;
        Double_t fWR_delta_t;
        TH1*  h_wr_delta_t;
        TH1* h_wr_subsystemid;
#endif
        TH1  *h_trace        [MAX_SFP][MAX_SLAVE][N_CHA];  //!
        TH1  *h_trace_blr    [MAX_SFP][MAX_SLAVE][N_CHA];  //!
        TH1  *h_trace_blr_fit [MAX_SFP][MAX_SLAVE][N_CHA];  //!
        TH1  *h_fpga_e       [MAX_SFP][MAX_SLAVE][N_CHA];  //!
        TH1  *h_bibox_fpga   [MAX_SFP][MAX_SLAVE][N_CHA];  //!
        #ifdef BIBOX
        TH1  *h_corr_e_fpga_bibox [MAX_SFP][MAX_SLAVE][N_CHA];  //!
        TH1  *h_bibox_f      [MAX_SFP][MAX_SLAVE][N_CHA];  //!
        TH1  *h_bibox_e      [MAX_SFP][MAX_SLAVE][N_CHA];  //!
        #endif // BIBOX
        #ifdef MWD
        TH1  *h_mwd_d        [MAX_SFP][MAX_SLAVE][N_CHA];  //!
        TH1  *h_mwd_c        [MAX_SFP][MAX_SLAVE][N_CHA];  //!
        TH1  *h_mwd_i        [MAX_SFP][MAX_SLAVE][N_CHA];  //!
        TH1  *h_mwd_e        [MAX_SFP][MAX_SLAVE][N_CHA];  //!
        #endif // MWD
        #ifdef NIK_EXTRA_HISTS
        TH1  *h_trgti_hitti  [MAX_SFP][MAX_SLAVE][N_CHA];  //!
        TH1  *h_ch_hitpat    [MAX_SFP][MAX_SLAVE][N_CHA];  //!
        TH1  *h_hitpat       [MAX_SFP][MAX_SLAVE];         //!
        TH1  *h_ch_hitpat_tr [MAX_SFP][MAX_SLAVE][N_CHA];  //!
        TH1  *h_hitpat_tr    [MAX_SFP][MAX_SLAVE];         //!
        TH1  *h_ch_hitpat_di [MAX_SFP][MAX_SLAVE][N_CHA];  //!
        TH1  *h_hitpat_di    [MAX_SFP][MAX_SLAVE];         //!
        TH1  *h_peak         [MAX_SFP][MAX_SLAVE][N_CHA];  //!
        TH1  *h_valley       [MAX_SFP][MAX_SLAVE][N_CHA];  //!
        TH1  *h_adc_spect    [MAX_SFP][MAX_SLAVE][N_CHA];  //!
        #endif // NIK_EXTRA_HISTS

    ClassDef(TPLEIADESRawProc,1)
};

#endif //TPLEIADESRAWPROC_H

//----------------------------END OF GO4 HEADER FILE ---------------------
