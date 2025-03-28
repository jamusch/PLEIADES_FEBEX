//------------------------------------------------------------------------
//************************ TPLEIADESRawProc.cxx **************************
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

#include "TPLEIADESRawProc.h"

#include "snprintf.h"
#include "Riostream.h"

#include "TH1.h"
#include "TH2.h"

#include "TGo4Log.h"
#include "TGo4MbsEvent.h"
#include "TGo4UserException.h"

#include "TPLEIADESParam.h"
#include "TPLEIADESFebexProc.h"

using namespace std;


//#ifdef USE_MBS_PARAM
//static UInt_t    l_tr_size = MAX_TRACE_SIZE;
//static UInt_t    l_tr    [MAX_TRACE_SIZE];
//static Double_t  f_tr_blr[MAX_TRACE_SIZE];
//static UInt_t    l_sfp_slaves  [MAX_SFP] = {MAX_SLAVE, MAX_SLAVE, MAX_SLAVE, MAX_SLAVE};
//static UInt_t    l_slaves=0;
//static UInt_t    l_trace=0;
//static UInt_t    l_e_filt=0;
//static UInt_t    l_pola        [MAX_SFP] = {0,0,0,0}; // all positive. not a real reset!
//#else
//static UInt_t    l_tr_size = TRACE_SIZE;
//static UInt_t    l_tr    [TRACE_SIZE];
//static Double_t  f_tr_blr[TRACE_SIZE];
//static UInt_t    l_sfp_slaves  [MAX_SFP] = NR_SLAVES;
//static UInt_t    l_sfp_adc_type[MAX_SFP] = ADC_TYPE;
//static UInt_t    l_pola        [MAX_SFP] = POLARITY;
//#endif // USE_MBS_PARAM
//
//static UInt_t    l_adc_type;
//static UInt_t    l_more_1_hit_ct=0;
//
//static UInt_t    l_first  = 0;
//static UInt_t    l_first2 = 0;

//------------------------------------------------------------------------
TPLEIADESRawProc::TPLEIADESRawProc() : TGo4EventProcessor("Proc"),fWR_SubsystemID(0), fWR_Timestamp(0), fWR_Timestamp_prev(0), fWR_delta_t(0)
{
    TGo4Log::Info("TPLEIADESRawProc: Create instance ");
}

//------------------------------------------------------------------------
// this one is used in standard factory
TPLEIADESRawProc::TPLEIADESRawProc(const char* name) : TGo4EventProcessor(name),fWR_SubsystemID(0), fWR_Timestamp(0), fWR_Timestamp_prev(0), fWR_delta_t(0)
{
    TGo4Log::Info("**** TPLEIADESRawProc: Create instance %s", name);
    fPar = dynamic_cast<TPLEIADESParam*>(MakeParameter("PLEIADESParam", "TPLEIADESParam", "set_PLEIADESParam.C"));
    if(fPar) { fPar->SetConfigBoards(); }





#ifdef WR_TIME_STAMP
    h_wr_delta_t= MakeTH1('F', "WRTimestamps/WR_Delta_time", "WR Time difference subsequent events",50000,0,500000,"#Delta t (ns)");
    h_wr_subsystemid = MakeTH1('I', "WRTimestamps/WR_Subsystems", "WR subsystem IDs", 10, 0, 10);
      if (IsObjMade())
      {
        h_wr_subsystemid->GetXaxis()->SetBinLabel(1 + 0, "0x100");
        h_wr_subsystemid->GetXaxis()->SetBinLabel(1 + 1, "0x200");
        h_wr_subsystemid->GetXaxis()->SetBinLabel(1 + 2, "0x300");
        h_wr_subsystemid->GetXaxis()->SetBinLabel(1 + 3, "0x400");
        h_wr_subsystemid->GetXaxis()->SetBinLabel(1 + 4, "0x500");
        h_wr_subsystemid->GetXaxis()->SetBinLabel(1 + 5, "0x600");
        h_wr_subsystemid->GetXaxis()->SetBinLabel(1 + 6, "0x700");
        h_wr_subsystemid->GetXaxis()->SetBinLabel(1 + 7, "0x800");
        h_wr_subsystemid->GetXaxis()->SetBinLabel(1 + 8, "0x900");
        h_wr_subsystemid->GetXaxis()->SetBinLabel(1 + 9, "unknown");
      }


#endif
      TString nm;
      nm.Form("%x",SUB_SYSTEM_ID_FEB1);
      fSubProcs[SUB_SYSTEM_ID_FEB1]= new TPLEIADESFebexProc(nm.Data(), (SUB_SYSTEM_ID_FEB1/0x100) -1);
      nm.Form("%x",SUB_SYSTEM_ID_FEB2);
      fSubProcs[SUB_SYSTEM_ID_FEB2]= new TPLEIADESFebexProc(nm.Data(), (SUB_SYSTEM_ID_FEB2/0x100) -1);
      //nm.Form("%d_",SUB_SYSTEM_ID_VME);
      //fSubProcs[SUB_SYSTEM_ID_VME]= new TPLEIADESVmeProc(nm.Data()); // TODO for optional scaler display in different unpacker class




}

//------------------------------------------------------------------------
TPLEIADESRawProc::~TPLEIADESRawProc()
{
    TGo4Log::Info("TPLEIADESRawProc: Delete instance ");
//    l_first  = 0; // JAM 13-Dec-2024: need to reset these flags to renew histogram handles when resubmitted from GUI!
//    l_first2 = 0;
}

//------------------------------------------------------------------------
// Build event is the unpacker function, it reads the MBS event and does stuff with it.
// Histograms are erased with the next word, so are mostly used for online analysis.
Bool_t TPLEIADESRawProc::BuildEvent(TGo4EventElement* target)
{
    Bool_t isValid = kFALSE;    // validity of output event

    TGo4MbsEvent *source = (TGo4MbsEvent*) GetInputEvent();	// event is called
    if(source == 0)
    {
        TGo4Log::Error("TPLEIADESRawProc: no input source !");
        return isValid;
    }


    fOutEvent= dynamic_cast<TPLEIADESRawEvent*>  (target);
    if(fOutEvent==0)
    {
        GO4_STOP_ANALYSIS_MESSAGE("NEVER COME HERE: output event is not configured, wrong class!");
        return isValid;
    }

    //  JAM 12-12-2023 take general event number from mbs event header. Note that subsystem sequence may differ:
    fOutEvent->fSequenceNumber = source->GetCount();
    // called by framework for each mbs input event.

    fOutEvent->SetValid(kFALSE); // initialize next output as not filled, i.e.it is only stored when something is in
    fOutEvent->fPhysTrigger = kFALSE;   // reset physics trigger each event

    isValid = kTRUE;            // input/ouput events look good

    //------------------------------------------------------------------------
    // object definitions. these objects will be used in reading the data stream.

//    UInt_t  l_i, l_j, l_k, l_l; // counters
//    Double_t    l_value=0;      // placeholder for filling histograms
//
    uint32_t    *pl_se_dat;     // pointer to data word from MBS sub event
    uint32_t    *pl_tmp;        // temporary pointer for manipulation
//
    UInt_t  l_dat_len;          // data length
    UInt_t  l_dat_len_byte;     // data length in bytes
//
    UInt_t  l_dat;              // data word
    UInt_t  l_trig_type;        // trigger type. used to make difference between febex and vme systems.
    UInt_t  l_trig_type_triva;  // another trigger type, not sure why we need two??
//    UInt_t  l_sfp_id;           // SFP ID (ie FEBEX crate)
//    UInt_t  l_feb_id;           // FEBEX card ID
//    UInt_t  l_cha_id;           // FEBEX Channel ID
//    UInt_t  l_n_hit;            // number of hits for subevent
//    //UInt_t  l_hit_id;
//    UInt_t  l_hit_cha_id;       // hit channel id
//    Long64_t    ll_time;        // hit time from special channel
//    Long64_t    ll_trg_time;    // trigger time from special channel
//    //Long64_t  ll_hit_time;
//    UInt_t  l_ch_hitpat   [MAX_SFP][MAX_SLAVE][N_CHA];  // channel hit pattern float array for filling histograms
//    UInt_t  l_ch_hitpat_tr[MAX_SFP][MAX_SLAVE][N_CHA];  // channel hit pattern for traces
//    UInt_t  l_first_trace [MAX_SFP][MAX_SLAVE];         // tracks first trace in FEBEX to fill -1 bin in hist
//
//    UInt_t  l_cha_head;         // 32bit word for channel header
//    UInt_t  l_cha_size;         // 32bit word for channel size (ie length in words)
//    UInt_t  l_trace_head;       // 32bit word for trace header
//    UInt_t  l_trace_size;       // 32bit word for trace size
//    UInt_t  l_trace_trail;      // 32bit word for checking trace trail
//
//    UInt_t  l_spec_head;        // 32bit word for checking special channel header
//    UInt_t  l_spec_trail;       // 32bit word for checking special channel trace trail
//    UInt_t  l_n_hit_in_cha;     // number of hits for specific channel
//    UInt_t  l_only_one_hit_in_cha;      // y/n for just one hit in channel
//    UInt_t  l_more_than_1_hit_in_cha;   // y/n for more than one hit
//    UInt_t  l_hit_time_sign;    // bit for polarity of hit time relative to trigger
//    Int_t   l_hit_time;          // magnitude of hit time
//
//    UInt_t  l_hit_cha_id2;      // hit channel for FPGA energy
//    //UInt_t  l_fpga_energy_sign; // bit for polarity of FPGA energy
//    Int_t  l_fpga_energy;       // magnitude of FPGA energy
//    Int_t   l_fpga_filt_on_off; // 4bit for checking if FPGA filter mode is on
//    //Int_t l_fpga_filt_mode;   // not used?
//    Int_t   l_dat_trace;        // word for storing trace used in FGPA filter
//    Int_t   l_dat_filt;         // word for storing FGPA filter
//    Int_t   l_filt_sign;        // polarity of FPGA filter for correction
//
    static UInt_t   l_evt_ct = 0;         // MBS event counter
    static UInt_t   l_evt_ct_phys = 0;  // physics event counter
//
//    UInt_t  l_pol = 0;          // polarity of FEBEX cards
//    UInt_t  l_pol_array      [MAX_SFP][MAX_SLAVE][N_CHA];   // array of polarities
//    UInt_t  l_fpga_e_found   [MAX_SFP][MAX_SLAVE][N_CHA];   // array of y/n if FPGA energy found
//    Int_t   l_fpga_e         [MAX_SFP][MAX_SLAVE][N_CHA];   // array of magnitude of FPGA energy
//    Int_t   l_fpga_hitti     [MAX_SFP][MAX_SLAVE][N_CHA];   // array of hit time values from FPGA
//
//    UInt_t  l_dat_fir;          // word for first half of trace
//    UInt_t  l_dat_sec;          // word for second half of trace
//
//    UInt_t  l_bls_start = BASE_LINE_SUBT_START;     // start for trace baseline calculation
//    UInt_t  l_bls_stop  = BASE_LINE_SUBT_START + BASE_LINE_SUBT_SIZE;       // stop for trace baseline calculation
//    Double_t    f_bls_val = 0.; // value of trace baseline
//
//
//    //------------------------------------------------------------------------
//    // special function objects. only initialised if defined in header.
//
//    #ifdef BIBOX
//    Int_t   l_A1, l_A2;             // BIBOX filter counters
//    UInt_t  l_gap = BIBOX_N_GAP;   // BIBOX gap size
//    UInt_t  l_win = BIBOX_N_AVG;   // BIBOX window size
//
//    UInt_t  l_bibox_e_found  [MAX_SFP][MAX_SLAVE][N_CHA];   // array of y/n if BIBOX energy found
//    Int_t   l_bibox_e        [MAX_SFP][MAX_SLAVE][N_CHA];   // array of magnitude of BIBOX energy
//    #endif // BIBOX
//
//    #ifdef MWD
//    UInt_t  l_mwd_wind = MWD_WIND;
//    UInt_t  l_mwd_avg  = MWD_AVG;
//    //Double_t    f_rev_tau = 1. / (Double_t) MWD_TAU;  // i'm using an array of TAU's, so calling the macro directly
//
//    // because the preamps have different decay consts, we need to individually set the tau value
//    Double_t l_mwd_tau [MAX_SFP][MAX_SLAVE][N_CHA];
//    // I initialise the array with a non-zero standard value to prevent math errrors
//    for(int i=0; i<MAX_SFP; ++i) { for(int j=0; j<MAX_SLAVE; ++j) { std::fill(l_mwd_tau[i][j], l_mwd_tau[i][j] + N_CHA, 1538); } }
//    // now I initialise the specific values given by the exp decay fits
//    l_mwd_tau[1][0][1] = 1567.68;     l_mwd_tau[1][0][2] = 1472.25;     l_mwd_tau[1][0][9] = 1567.88;     l_mwd_tau[1][0][10] = 1558.63;
//    l_mwd_tau[1][1][1] = 1560.54;     l_mwd_tau[1][1][2] = 1559.62;     l_mwd_tau[1][1][9] = 1568.26;     l_mwd_tau[1][1][10] = 1565.76;
//    l_mwd_tau[1][2][1] = 1572.50;     l_mwd_tau[1][2][2] = 1565.66;     l_mwd_tau[1][3][1] = 1570.41;     l_mwd_tau[1][3][2]  = 1567.77;
//    l_mwd_tau[1][4][0] = 1101.10;     l_mwd_tau[1][4][1] = 1109.53;     l_mwd_tau[1][4][2] = 1117.12;     l_mwd_tau[1][4][3]  = 1111.86;
//    l_mwd_tau[1][4][4] = 1115.77;     l_mwd_tau[1][4][5] = 1111.08;     l_mwd_tau[1][4][12] = 2458.66;    l_mwd_tau[1][4][13] = 2484.18;
//
//    Int_t       l_diff_sum = 0;
//    Int_t       l_dint_sum = 0;
//    Double_t    l_cint_sum = 0.;
//
//    #ifdef USE_MBS_PARAM
//        Double_t    f_diff[MAX_TRACE_SIZE];
//        Double_t    f_corr[MAX_TRACE_SIZE];
//        Double_t    f_int[MAX_TRACE_SIZE];
//    #else
//        Double_t    f_diff[MAX_TRACE_SIZE];
//        Double_t    f_corr[MAX_TRACE_SIZE];
//        Double_t    f_int[MAX_TRACE_SIZE];
//    #endif // USE_MBS_PARAM
//
//    Int_t   l_mwd_e[MAX_SFP][MAX_SLAVE][N_CHA];   // array of magnitude of MWD energy
//
//    #endif // MWD


    //------------------------------------------------------------------------
    // MBS data stream readout. Here the function starts calling basic Go4 functions to read data words, etc.

    // MBS trigger counters are incremeneted, but also not used?
    l_trig_type_triva = source->GetTrigger();
    if (l_trig_type_triva == 1)
    {
        l_evt_ct_phys++;
    }
    l_evt_ct++;

    // MBS allows for multiple subevents. We don't use sub events, so machinery has been removed for brevity.
    // See TFeb3Full example or CsISiPHOS_FEBEX setup from N. Kurz for sub event machinery.
    TGo4MbsSubEvent* psubevt;

    source->ResetIterator();

    // JAM25: scan all subevents here:
     while (psubevt = source->NextSubEvent()) // subevent loop
     {

       //if( psubevt->GetSubcrate() == 0)

    // next we extract data word for subevent and prepare properties
    // GetDataField is pointer to subevent data. pl_tmp++ increments pointer to next word in stream.
    pl_se_dat = (uint32_t *)psubevt->GetDataField();
    l_dat_len = psubevt->GetDlen();
    l_dat_len_byte = (l_dat_len - 2) * 2;

    pl_tmp = pl_se_dat;

    // checks for error in data word
    if(pl_se_dat == (UInt_t*)0)
    {
        printf (" ERROR>> ");
        printf ("pl_se_dat: 0x%lx, ", (ULong_t) pl_se_dat);
        printf ("l_dat_len: 0x%x, ", (UInt_t)l_dat_len);
        printf ("l_trig_type_triva: 0x%x \n", (UInt_t)l_trig_type_triva); fflush (stdout);
        goto bad_event;
    }

    if( (*pl_tmp) == 0xbad00bad)
    {
        printf ("ERROR>> found bad event (0xbad00bad) \n");
        goto bad_event;
    }

    // checks WR time stamp. 5 first 32 bits must be white rabbit time stamp
    #ifdef WR_TIME_STAMP
    // JAM 27-03-25: get statistics of subsystems here
    fWR_SubsystemID = *pl_tmp++;
    h_wr_subsystemid->Fill(fWR_SubsystemID/0x100 -1 );


    // JAM 27-03-25: evaluate time stamp for possible checks later
    l_dat = (*pl_tmp) >> 16;
    if (l_dat != TS__ID_L16)
    {
      printf ("ERROR>> 2. data word does not contain low 16bit identifier:\n");
      printf ("should be: 0x%x, but is: 0x%x\n", TS__ID_L16, l_dat);
    }
    fWR_Timestamp = *pl_tmp++ & 0xFFFF;

    l_dat = (*pl_tmp) >> 16;
    if (l_dat != TS__ID_M16)
    {
      printf ("ERROR>> 3. data word does not contain mid 16bit identifier:\n");
      printf ("should be: 0x%x, but is: 0x%x\n", TS__ID_M16, l_dat);
    }
    fWR_Timestamp +=( (*pl_tmp++ & 0xFFFF) << 16);

    l_dat = (*pl_tmp) >> 16;
    if (l_dat != TS__ID_H16)
    {
      printf ("ERROR>> 4. data word does not contain high 16bit identifier:\n");
      printf ("should be: 0x%x, but is: 0x%x\n", TS__ID_H16, l_dat);
    }
    fWR_Timestamp +=((*pl_tmp++ & 0xFFFF)<< 32);

    l_dat = (*pl_tmp) >> 16;
    if (l_dat != TS__ID_X16)
        {
        printf ("ERROR>> 5. data word does not contain xhigh 16bit identifier:\n");
        printf ("should be: 0x%x, but is: 0x%x\n", TS__ID_X16, l_dat);
        printf ("WR_TIME_STAMP is activated, but timestamp is throwing errors. Perhaps WR_TIME_STAMP should be turned off?\n");
        }
    fWR_Timestamp +=((*pl_tmp++ & 0xFFFF)<< 48);
    if(fWR_Timestamp_prev)
            {
              fWR_delta_t=fWR_Timestamp - fWR_Timestamp_prev;
              h_wr_delta_t->Fill(fWR_delta_t);
            }
    fWR_Timestamp_prev=fWR_Timestamp;


    #endif // WR_TIME_STAMP


        TPLEIADESFebexProc* subproc = dynamic_cast< TPLEIADESFebexProc*>(fSubProcs[fWR_SubsystemID]);
        if(subproc) {


          //printf("WWWWWW using FEBEX subsystem id 0x%x, trigger:%d \n",fWR_SubsystemID, l_trig_type_triva); fflush (stdout);
          size_t offset= (char*)(pl_tmp)- (char*) (pl_se_dat);
          if(!subproc->BuildSubEvent(psubevt,offset, fOutEvent)) goto bad_event;
        }
        else
        {
          //printf("WWWWWW no unpacker for subsystem id 0x%x, trigger:%d \n",fWR_SubsystemID, l_trig_type_triva); fflush (stdout);
        }



 } // while subevents

      fOutEvent->SetValid(isValid);

    bad_event:



    return isValid;
}



//----------------------------END OF GO4 SOURCE FILE ---------------------
