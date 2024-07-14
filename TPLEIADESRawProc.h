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

// comment out define statements to turn on/off

//#define WR_TIME_STAMP     1   // white rabbit time stamp is head of data

#ifdef WR_TIME_STAMP
    #define SUB_SYSTEM_ID      0x200
    #define TS__ID_L16         0x3e1
    #define TS__ID_M16         0x4e1
    #define TS__ID_H16         0x5e1
    #define TS__ID_X16         0x6e1
#endif // WR_TIME_STAMP

#define USE_MBS_PARAM     1

#define TRAPEZ 1
#define MWD    1

// JAM23: now these definitions are in TPLEIADESRawEvent.h
//#define MAX_SFP           4
//#define MAX_SLAVE        16
//#define N_CHA            16

#define ADC_RES            4000./16384.   // res in mV: +/-2V range by 14-bit ADC

#ifdef USE_MBS_PARAM
    #define MAX_TRACE_SIZE    8000   // in samples
    #define MAX_TRAPEZ_N_AVG  2000   // in samples
#else 
    #define     TRACE_SIZE    1000   // in samples 1024
    //#define     TRAPEZ_N_AVG    64   // in samples
    // nr of slaves on SFP 0  1  2  3
    //                     |  |  |  |
    #define NR_SLAVES    { 0, 5, 0, 0}
    #define ADC_TYPE     { 0, 0xffff, 0, 0} // 12 bit: 0, 14 bit: 1
                                            // bit 0 fuer slave module 0
                                            // bit n fuer slave module n (max n = 31)

    #define POLARITY {0x00000000, 0x0000ffff, 0x00000000, 0x00000000}
    // for max. 32 FEBEX per SFP, each bit represents a FEBEX
    //  0: positive signals, 1 negative signals
#endif

#define BASE_LINE_SUBT_START  0
#define BASE_LINE_SUBT_SIZE   20

#ifdef TRAPEZ
    #define TRAPEZ_N_GAP  200
    #define TRAPEZ_N_AVG  100
#endif // TRAPEZ

#ifdef MWD
    #define MWD_WIND   1500
    #define MWD_AVG    1400   // MWD_WIND must be bigger than MWD_AVG
    //#define MWD_TAU   1000   // differential input febex  // in number of trace slices
    #define MWD_TAU   5000   // differential input febex  // in number of trace slices
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


        TH1  *h_trace        [MAX_SFP][MAX_SLAVE][N_CHA];  //!
        TH1  *h_trace_blr    [MAX_SFP][MAX_SLAVE][N_CHA];  //!
        TH1  *h_trapez_fpga  [MAX_SFP][MAX_SLAVE][N_CHA];  //!
        TH1  *h_trapez_f     [MAX_SFP][MAX_SLAVE][N_CHA];  //!
        TH1  *h_trapez_e     [MAX_SFP][MAX_SLAVE][N_CHA];  //!
        TH1  *h_fpga_e       [MAX_SFP][MAX_SLAVE][N_CHA];  //!
        TH1  *h_corr_e_fpga_trapez [MAX_SFP][MAX_SLAVE][N_CHA];  //!
        TH1  *h_mwd_f        [MAX_SFP][MAX_SLAVE][N_CHA];  //!
        TH1  *h_mwd_a        [MAX_SFP][MAX_SLAVE][N_CHA];  //!
        TH1  *h_mwd_e        [MAX_SFP][MAX_SLAVE][N_CHA];  //!
        TH1  *h_sum_trace    [MAX_SFP][MAX_SLAVE][N_CHA];  //!
        TH1  *h_peak         [MAX_SFP][MAX_SLAVE][N_CHA];  //!
        TH1  *h_valley       [MAX_SFP][MAX_SLAVE][N_CHA];  //!
        TH1  *h_trgti_hitti  [MAX_SFP][MAX_SLAVE][N_CHA];  //!
        TH1  *h_ch_hitpat    [MAX_SFP][MAX_SLAVE][N_CHA];  //!
        TH1  *h_hitpat       [MAX_SFP][MAX_SLAVE];         //!
        TH1  *h_ch_hitpat_tr [MAX_SFP][MAX_SLAVE][N_CHA];  //!
        TH1  *h_hitpat_tr    [MAX_SFP][MAX_SLAVE];         //!
        TH1  *h_ch_hitpat_di [MAX_SFP][MAX_SLAVE][N_CHA];  //!
        TH1  *h_hitpat_di    [MAX_SFP][MAX_SLAVE];         //!
        TH1  *h_test;
        TH1  *h_adc_spect    [MAX_SFP][MAX_SLAVE][N_CHA];  //!

    ClassDef(TPLEIADESRawProc,1)
};

#endif //TPLEIADESRAWPROC_H

//----------------------------END OF GO4 HEADER FILE ---------------------
