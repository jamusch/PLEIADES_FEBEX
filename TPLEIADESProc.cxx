// N.Kurz, EE, GSI, 15-Jan-2010

//-------------------------------------------------------------
//        Go4 Release Package v3.03-05 (build 30305)
//                      05-June-2008
//---------------------------------------------------------------
//   The GSI Online Offline Object Oriented (Go4) Project
//   Experiment Data Processing at EE department, GSI
//---------------------------------------------------------------
//
//Copyright (C) 2000- Gesellschaft f. Schwerionenforschung, GSI
//                    Planckstr. 1, 64291 Darmstadt, Germany
//Contact:            http://go4.gsi.de
//----------------------------------------------------------------
//This software can be used under the license agreements as stated
//in Go4License.txt file which is part of the distribution.
//----------------------------------------------------------------
#include "TPLEIADESProc.h"
#include "stdint.h"

#include "Riostream.h"

using namespace std;

#include "TH1.h"
#include "TH2.h"
#include "TCutG.h"
#include "snprintf.h"

#include "TGo4MbsEvent.h"
#include "TGo4WinCond.h"
#include "TGo4PolyCond.h"
#include "TGo4CondArray.h"
#include "TGo4Picture.h"

#include "TGo4UserException.h"

#include "TPLEIADESRawEvent.h"
#include "TPLEIADESParam.h"


#ifdef USE_MBS_PARAM
 static UInt_t    l_tr_size = MAX_TRACE_SIZE;
 static UInt_t    l_tr    [MAX_TRACE_SIZE];
 static Double_t  f_tr_blr[MAX_TRACE_SIZE];
 static UInt_t    l_sfp_slaves  [MAX_SFP] = {MAX_SLAVE, MAX_SLAVE, MAX_SLAVE, MAX_SLAVE};
 static UInt_t    l_slaves=0;
 static UInt_t    l_trace=0;
 static UInt_t    l_e_filt=0;
 static UInt_t    l_pola        [MAX_SFP] = {0,0,0,0}; // all positive. not a real reset!
#else
 static UInt_t    l_tr_size = TRACE_SIZE;
 static UInt_t    l_tr    [TRACE_SIZE];
 static Double_t  f_tr_blr[TRACE_SIZE];
 static UInt_t    l_sfp_slaves  [MAX_SFP] = NR_SLAVES;
 static UInt_t    l_sfp_adc_type[MAX_SFP] = ADC_TYPE;
 static UInt_t    l_pola        [MAX_SFP] = POLARITY;
#endif //

static UInt_t    l_adc_type;
static UInt_t    l_more_1_hit_ct=0;

static UInt_t    l_first  = 0;
static UInt_t    l_first2 = 0;

#ifdef FFT_GSI
static UInt_t    l_pct;
static UInt_t    l_tr_fft_si; 
#endif

//***********************************************************
TPLEIADESProc::TPLEIADESProc() : TGo4EventProcessor("Proc")
{
  cout << "**** TPLEIADESProc: Create instance " << endl;
}
//***********************************************************
TPLEIADESProc::~TPLEIADESProc()
{
  cout << "**** TPLEIADESProc: Delete instance " << endl;
  l_first  = 0; // JAM 13-Dec-2024: need to reset these flags to renew histogram handles when resubmitted from GUI!
  l_first2 = 0;
}
//***********************************************************
// this one is used in standard factory
TPLEIADESProc::TPLEIADESProc(const char* name) : TGo4EventProcessor(name)
{
  cout << "**** TPLEIADESProc: Create instance " << name << endl;
  fPar = dynamic_cast<TPLEIADESParam*>(MakeParameter("PLEIADESParam", "TPLEIADESParam", "set_PLEIADESParam.C"));
  //printf ("Histograms created \n");  fflush (stdout);
}
//-----------------------------------------------------------
// event function
Bool_t TPLEIADESProc::BuildEvent(TGo4EventElement* target)
{
  // called by framework for each mbs input event.


  fOutEvent= dynamic_cast<TPLEIADESRawEvent*>  (target);
  if(fOutEvent==0)  GO4_STOP_ANALYSIS_MESSAGE(
      "NEVER COME HERE: output event is not configured, wrong class!")

  fOutEvent->SetValid(kFALSE); // initialize next output as not filled, i.e.it is only stored when something is in



  UInt_t         l_i, l_j, l_k, l_l;
  uint32_t      *pl_se_dat;
  uint32_t      *pl_tmp;

  UInt_t         l_dat_len;  
  UInt_t         l_dat_len_byte;  

  UInt_t         l_dat;
  UInt_t         l_trig_type;
  UInt_t         l_trig_type_triva;
  UInt_t         l_sfp_id;
  UInt_t         l_feb_id;
  UInt_t         l_cha_id;
  UInt_t         l_n_hit;
  //UInt_t         l_hit_id;
  UInt_t         l_hit_cha_id;
  Long64_t       ll_time;
  Long64_t       ll_trg_time;
  //Long64_t       ll_hit_time;
  UInt_t         l_ch_hitpat   [MAX_SFP][MAX_SLAVE][N_CHA];  
  UInt_t         l_ch_hitpat_tr[MAX_SFP][MAX_SLAVE][N_CHA];  
  UInt_t         l_first_trace [MAX_SFP][MAX_SLAVE];

  UInt_t         l_cha_head;  
  UInt_t         l_cha_size;
  UInt_t         l_trace_head;
  UInt_t         l_trace_size;
  UInt_t         l_trace_trail;

  UInt_t         l_spec_head;
  UInt_t         l_spec_trail;
  UInt_t         l_n_hit_in_cha;
  UInt_t         l_only_one_hit_in_cha;
  UInt_t         l_more_than_1_hit_in_cha;  
  UInt_t         l_hit_time_sign;
   Int_t         l_hit_time;
  UInt_t         l_hit_cha_id2;
  UInt_t         l_fpga_energy_sign;
   Int_t         l_fpga_energy;

  UInt_t         l_trapez_e_found [MAX_SFP][MAX_SLAVE][N_CHA];  
  UInt_t         l_fpga_e_found   [MAX_SFP][MAX_SLAVE][N_CHA]; 
  UInt_t         l_trapez_e       [MAX_SFP][MAX_SLAVE][N_CHA];  
  UInt_t         l_fpga_e         [MAX_SFP][MAX_SLAVE][N_CHA]; 

  UInt_t         l_dat_fir;
  UInt_t         l_dat_sec;

  static UInt_t  l_evt_ct=0;
  static UInt_t  l_evt_ct_phys=0;

  UInt_t         l_pol = 0;

  UInt_t          l_bls_start = BASE_LINE_SUBT_START;
  UInt_t          l_bls_stop  = BASE_LINE_SUBT_START + BASE_LINE_SUBT_SIZE; // 
  Double_t       f_bls_val=0.;

  #ifdef IVAN
  Real_t r_ivan_f_prev; 
  Real_t r_ivan_f; 
  #endif // IVAN

  #ifdef FFT_GSI
   #ifdef USE_MBS_PARAM
    Double_t f_f_real[MAX_TRACE_SIZE];
    Double_t f_f_imag[MAX_TRACE_SIZE];
   #else
    Double_t f_f_real[TRACE_SIZE];
    Double_t f_f_imag[TRACE_SIZE];
   #endif // USE_MBS_PARAM  
  #endif // FFT_GSI
  #ifdef TEST_FFT_GSI
   #define TWOPI  Double_t (3.14159265 * 2.)
   #ifdef USE_MBS_PARAM
    Double_t f_test_real[MAX_TRACE_SIZE];
    Double_t f_test_imag[MAX_TRACE_SIZE];
   #else
    Double_t f_test_real[TRACE_SIZE];
    Double_t f_test_imag[TRACE_SIZE];
   #endif // USE_MBS_PARAM  
  #endif // TEST_FFT_GSI

  #ifdef APFEL_INT
  Double_t       f_sum_a;
  Double_t       f_sum_tr[TRACE_SIZE];
  #endif // APFEL_INT

  #ifdef TRAPEZ
   Int_t          l_A1;
   Int_t          l_A2;
   UInt_t          l_gap = TRAPEZ_N_GAP; // TRAPEZ gap    size
   UInt_t          l_win = TRAPEZ_N_AVG; // TRAPEZ window size
  #endif // TRAPEZ

  #ifdef MWD
   UInt_t          l_mwd_wind  = MWD_WIND;
   UInt_t          l_mwd_avg   = MWD_AVG;
   Double_t       f_rev_tau   = 1. / (Double_t) MWD_TAU;
   Int_t          l_tau_sum=0;
   //Double_t        f_tau_sum=0.;
   Double_t        f_sum=0.;
   Double_t        f_sum_e=0.;
   #ifdef USE_MBS_PARAM
    Double_t        f_mwd[MAX_TRACE_SIZE]; 
    Double_t        f_avg[MAX_TRACE_SIZE];
   #else
    Double_t        f_mwd[TRACE_SIZE]; 
    Double_t        f_avg[TRACE_SIZE];
   #endif // MWD 
  #endif // MWD

  #ifdef APFEL_INT
   Int_t          l_A1_a;
   Int_t          l_A2_a;
   UInt_t          l_gap_a = A_TRAPEZ_N_GAP; // TRAPEZ gap    size
   UInt_t          l_win_a = A_TRAPEZ_N_AVG; // TRAPEZ window size
  #endif // APFEL_INT

  Int_t       l_fpga_filt_on_off;
  //Int_t       l_fpga_filt_mode;
  Int_t       l_dat_trace;
  Int_t       l_dat_filt;
  Int_t       l_filt_sign;


#ifdef FFT_GSI
  Double_t d_re;
  Double_t d_im;
  Double_t d_am;
  Double_t d_ph;
#endif

  Double_t value=0;

  TGo4MbsSubEvent* psubevt;


  fInput = (TGo4MbsEvent* ) GetInputEvent();
  if(fInput == 0)
  {
    cout << "AnlProc: no input event !"<< endl;
    return kFALSE;
  }


  //  JAM 12-12-2023 take general event number from mbs event header. Note that subsystem sequence may differ:
  fOutEvent->fSequenceNumber = fInput->GetCount();


  l_trig_type_triva = fInput->GetTrigger();
  if (l_trig_type_triva == 1)
  {
     l_evt_ct_phys++;
  }

  //if(fInput->GetTrigger() > 11)
  //{
  //cout << "**** TPLEIADESProc: Skip trigger event"<<endl;
  //return kFALSE;
  //}
  // first we fill the arrays fCrate1,2 with data from MBS source
  // we have up to two subevents, crate 1 and 2
  // Note that one has to loop over all subevents and select them by
  // crate number:   psubevt->GetSubcrate(),
  // procid:         psubevt->GetProcid(),
  // and/or control: psubevt->GetControl()
  // here we use only crate number

  l_evt_ct++;

  fInput->ResetIterator();
  //while((psubevt = fInput->NextSubEvent()) != 0) // loop over subevents
  //{

  psubevt = fInput->NextSubEvent(); // only one subevent    
  
  //printf ("         psubevt: 0x%x \n", (UInt_t)psubevt); fflush (stdout);
  //printf ("-------------------------------next event-----------\n");
  //sleep (1);

  pl_se_dat = (uint32_t *)psubevt->GetDataField();
  l_dat_len = psubevt->GetDlen();
  l_dat_len_byte = (l_dat_len - 2) * 2; 
  //printf ("sub-event data size:         0x%x, %d \n", l_dat_len, l_dat_len);
  //printf ("sub-event data size (bytes): 0x%x, %d \n", l_dat_len_byte, l_dat_len_byte);
  //fflush (stdout);

  pl_tmp = pl_se_dat;

  if (pl_se_dat == (UInt_t*)0)
  {
    printf (" ERROR>> ");
    printf ("pl_se_dat: 0x%lx, ", (ULong_t) pl_se_dat);
    printf ("l_dat_len: 0x%x, ", (UInt_t)l_dat_len);
    printf ("l_trig_type_triva: 0x%x \n", (UInt_t)l_trig_type_triva); fflush (stdout);
    goto bad_event;  
  }

  if ( (*pl_tmp) == 0xbad00bad)
  {
    printf ("ERROR>> found bad event (0xbad00bad) \n");
    goto bad_event;
  }

  #ifdef WR_TIME_STAMP
  // 5 first 32 bits must be white rabbit time stamp
  l_dat = *pl_tmp++;
/*
  if (l_dat != SUB_SYSTEM_ID)
  {
    printf ("ERROR>> 1. data word is not sub-system id: %d \n");
    printf ("should be: 0x%x, but is: 0x%x\n", SUB_SYSTEM_ID, l_dat);
  }

  if (l_dat != SUB_SYSTEM_ID)
  {
    goto bad_event;
  }
*/

  l_dat = (*pl_tmp++) >> 16;
  if (l_dat != TS__ID_L16)
  {
    printf ("ERROR>> 2. data word does not contain 0-15 16bit identifier: \n");
    printf ("should be: 0x%x, but is: 0x%x\n", TS__ID_L16, l_dat);
  }
  l_dat = (*pl_tmp++) >> 16;
  if (l_dat != TS__ID_M16)
  {
    printf ("ERROR>> 3. data word does not contain 16-31 16bit identifier:  \n");
    printf ("should be: 0x%x, but is: 0x%x\n", TS__ID_M16, l_dat);
  }
  l_dat = (*pl_tmp++) >> 16;
  if (l_dat != TS__ID_H16)
  {
    printf ("ERROR>> 4. data word does not contain 32-47 16bit identifier:  \n");
    printf ("should be: 0x%x, but is: 0x%x\n", TS__ID_H16, l_dat);
  }
  l_dat = (*pl_tmp++) >> 16;
  if (l_dat != TS__ID_X16)
  {
    printf ("ERROR>> 4. data word does not contain 48-63 16bit identifier:  \n");
    printf ("should be: 0x%x, but is: 0x%x\n", TS__ID_X16, l_dat);
  }
  #endif // WR_TIME_STAMP

  // extract analysis parameters from MBS data 
  // ATTENTION:  these data is only present if WRITE_ANALYSIS_PARAM 
  //             is enabled in corresponding f_user.c 
  // WRITE_ANALYSIS_PARAM (in mbs) and USE_MBS_PARAM (in go4) must be used always together 

  #ifdef USE_MBS_PARAM
  l_slaves = *pl_tmp++;
  l_trace  = *pl_tmp++;
  l_e_filt = *pl_tmp++;
  for (l_i=0; l_i<MAX_SFP; l_i++)
  {
    l_pola[l_i] = *pl_tmp++;
  }
  #endif

  if (l_first == 0)
  {
    l_first = 1;
    #ifdef USE_MBS_PARAM
    printf ("debug: 0x%x, 0x%x, 0x%x \n", l_slaves, l_trace, l_e_filt);
    fflush (stdout);
    #endif
    f_make_histo (0);
  }

  #ifdef TEST_FFT_GSI
  h_test->Reset ("");
  h_test_fft_real->Reset ("");
  h_test_fft_imag->Reset ("");
  for (l_l=0; l_l<l_tr_size; l_l++)
  {
    f_test_real[l_l] = 0.;
    f_test_imag[l_l] = 0.;
  }  
  #endif // TEST_FFT_GSI

  for (l_i=0; l_i<MAX_SFP; l_i++)
  {
    if (l_sfp_slaves[l_i] != 0)
    {
      for (l_j=0; l_j<l_sfp_slaves[l_i]; l_j++)
      {
        for (l_k=0; l_k<N_CHA; l_k++)
        {
          h_trace       [l_i][l_j][l_k]->Reset ("");
          h_trace_blr   [l_i][l_j][l_k]->Reset ("");
          h_trapez_fpga [l_i][l_j][l_k]->Reset ("");
          l_ch_hitpat   [l_i][l_j][l_k] = 0;  
          l_ch_hitpat_tr[l_i][l_j][l_k] = 0;
          #ifdef IVAN
          h_ivan_f[l_i][l_j][l_k]->Reset ("");
          #endif // IVAN
          #ifdef FFT_GSI
          h_fft_real[l_i][l_j][l_k]->Reset ("");
          h_fft_imag[l_i][l_j][l_k]->Reset ("");
          h_fft_ampl[l_i][l_j][l_k]->Reset ("");
          h_fft_phas[l_i][l_j][l_k]->Reset ("");
          for (l_l=0; l_l<l_tr_size; l_l++)
          {
            f_f_real[l_l] = 0.;
            f_f_imag[l_l] = 0.;
          }  
          #endif // FFT_GSI
          #ifdef TRAPEZ
          h_trapez_f[l_i][l_j][l_k]->Reset ("");
          #endif // TRAPEZ
          #ifdef MWD
          h_mwd_f[l_i][l_j][l_k]->Reset ("");
          h_mwd_a[l_i][l_j][l_k]->Reset ("");
          for (l_l=0; l_l<l_tr_size; l_l++)
          {
            f_mwd[l_l] = 0.;
            f_avg[l_l] = 0.;
          }  
          #endif // MWD
          #ifdef APFEL_INT
          h_sum_trace [l_i][l_j][l_k]->Reset ("");
          h_apf_trap_f[l_i][l_j][l_k]->Reset ("");
          #endif // APFEL_INT
          l_trapez_e_found[l_i][l_j][l_k] = 0;
          l_fpga_e_found  [l_i][l_j][l_k] = 0;
          l_trapez_e      [l_i][l_j][l_k] = 0;
          l_fpga_e        [l_i][l_j][l_k] = 0;
        }
        h_hitpat   [l_i][l_j]->Fill (-2, 1);  
        h_hitpat_tr[l_i][l_j]->Fill (-2, 1);  
        l_first_trace[l_i][l_j] = 0;
      }
    }
  }
 
  while ( (pl_tmp - pl_se_dat) < (l_dat_len_byte/4) )
  {
    //sleep (1);
    //printf (" begin while loop \n");  fflush (stdout); 
    l_dat = *pl_tmp++;   // must be padding word or channel header
    //printf ("l_dat 0x%x \n", l_dat);
    if ( (l_dat & 0xfff00000) == 0xadd00000 ) // begin of padding 4 byte words
    {
      //printf ("padding found \n");
      l_dat = (l_dat & 0xff00) >> 8;
      pl_tmp += l_dat - 1;  // increment by pointer by nr. of padding  4byte words 
    }
    else if ( (l_dat & 0xff) == 0x34) //channel header
    {
      l_cha_head = l_dat;
      //printf ("l_cha_head: 0x%x \n", l_cha_head);

      l_trig_type = (l_cha_head & 0xf00)      >>  8;
      l_sfp_id    = (l_cha_head & 0xf000)     >> 12;
      l_feb_id    = (l_cha_head & 0xff0000)   >> 16;
      l_cha_id    = (l_cha_head & 0xff000000) >> 24;
 
      if ((l_sfp_id > (MAX_SFP-1)) || (l_sfp_id < 0))
      {
        printf ("ERROR>> l_spf_id: %d \n", l_sfp_id);  fflush (stdout);
        goto bad_event; 
      }
      if ((l_feb_id > (MAX_SLAVE-1)) || (l_feb_id < 0))
      {
        printf ("ERROR>> l_feb_id: %d \n", l_feb_id); fflush (stdout);
        goto bad_event; 
      }
      if ((l_cha_id > (N_CHA-1)) || (l_cha_id < 0))
      {
        if (l_cha_id != 0xff)
        { 
          printf ("ERROR>> l_cha_id: %d \n", l_cha_id); fflush (stdout);
          goto bad_event;
        }
      }

      l_pol = (l_pola[l_sfp_id] >> l_feb_id) & 0x1;

      if ( ((l_cha_head & 0xff) >> 0) != 0x34 )
      {
        printf ("ERROR>> channel header type is not 0x34 \n");
        goto bad_event;
      }

      if ( (l_cha_head & 0xff000000) == 0xff000000) // special channel 0xff for E,t from fpga 
      {
        //printf ("special channel \n");
        // special channel data size
        l_cha_size = *pl_tmp++;
        //printf ("l_cha_head: 0x%x \n", l_cha_head); sleep (1);
        //printf ("l_cha_size: 0x%x \n", l_cha_size);

        l_spec_head = *pl_tmp++;
        if ( (l_spec_head & 0xff000000) != 0xaf000000)
        {  
          printf ("ERROR>> E,t summary: wrong header is 0x%x, must be: 0x%x\n",
                                                 (l_spec_head & 0xff000000)>>24, 0xaf);              
          goto bad_event;
          //sleep (1); 
        }
        ll_trg_time  = (Long64_t)*pl_tmp++;
        ll_time      = (Long64_t)*pl_tmp++;
        ll_trg_time += ((ll_time & 0xffffff) << 32);

        l_n_hit = (l_cha_size - 16) >> 3;
        //printf ("#hits: %d \n", l_n_hit);

        if (l_trig_type_triva == 1) // physics event
        { 
          h_hitpat[l_sfp_id][l_feb_id]->Fill (-1, 1);

          for (l_i=0; l_i<l_n_hit; l_i++)
          {
            l_dat = *pl_tmp++;      // hit time from fpga (+ other info)
            l_hit_cha_id             = (l_dat & 0xf0000000) >> 28;
            l_n_hit_in_cha           = (l_dat & 0xf000000)  >> 24;

            l_more_than_1_hit_in_cha = (l_dat & 0x400000)   >> 22;
            l_only_one_hit_in_cha    = (l_dat & 0x100000)   >> 20;
 
            l_ch_hitpat[l_sfp_id][l_feb_id][l_hit_cha_id] = l_n_hit_in_cha;            
            
            if (l_more_than_1_hit_in_cha == 1)
            {
              l_more_1_hit_ct++;
              printf ("%d More than 1 hit found for SFP: %d FEBEX: %d CHA: %d:: %d \n",
                      l_more_1_hit_ct, l_sfp_id, l_feb_id, l_hit_cha_id, l_n_hit_in_cha);
              fflush (stdout);
            }
            

            if ((l_more_than_1_hit_in_cha == 1) && (l_only_one_hit_in_cha == 1))
            {
              printf ("ERROR>> haeh? \n"); fflush (stdout);
            }  

            if (l_only_one_hit_in_cha == 1)
            {
              l_hit_time_sign = (l_dat & 0x8000) >> 15;
              l_hit_time = l_dat & 0x7ff;     // positive := AFTER  trigger, relative to trigger time
              if (l_hit_time_sign == 1)       // negative sign
              {
                l_hit_time = l_hit_time * (-1); // negative := BEFORE trigger, relative to trigger time
              }
              //printf ("cha: %d, hit fpga time:  %d \n", l_hit_cha_id,  l_hit_time);
              h_trgti_hitti[l_sfp_id][l_feb_id][l_hit_cha_id]->Fill (l_hit_time);
            }
            h_hitpat[l_sfp_id][l_feb_id]->Fill (l_hit_cha_id, l_n_hit_in_cha);
            
            l_dat = *pl_tmp++;      // energy from fpga (+ other info)
            l_hit_cha_id2  = (l_dat & 0xf0000000) >> 28;
            if (l_hit_cha_id != l_hit_cha_id2)
            {
              printf ("ERROR>> hit channel ids differ in energy and time data word\n");
              goto bad_event;
            }
            if ((l_hit_cha_id > (N_CHA-1)) || (l_hit_cha_id < 0))
            {
              printf ("ERROR>> hit channel id: %d \n", l_hit_cha_id); fflush (stdout);
              goto bad_event;
            }

            if (l_only_one_hit_in_cha == 1)
            {
              l_fpga_energy_sign = (l_dat & 0x800000) >> 23;
              l_fpga_energy      =  l_dat & 0x7fffff;      // positiv
              if (l_fpga_energy_sign == 1)                // negative sign
              {
                l_fpga_energy = l_fpga_energy * (-1);     // negative
              }
              //printf ("cha: %d, hit fpga energy: %d \n", l_hit_cha_id2,  l_fpga_energy);
              h_fpga_e[l_sfp_id][l_feb_id][l_hit_cha_id]->Fill (l_fpga_energy);
              l_fpga_e_found [l_sfp_id][l_feb_id][l_hit_cha_id] = 1;
              l_fpga_e[l_sfp_id][l_feb_id][l_hit_cha_id] = l_fpga_energy; 
            }
          }
        }
        l_spec_trail = *pl_tmp++;
        if ( (l_spec_trail & 0xff000000) != 0xbf000000)
        {  
          printf ("ERROR>> E,t summary: wrong header is 0x%x, must be: 0x%x\n",
                                                 (l_spec_trail & 0xff000000)>>24, 0xbf);              
          goto bad_event;
          //sleep (1); 
        }
      }
      else // real channel 
      {
        //printf ("real channel \n");
        // channel data size
        l_cha_size = *pl_tmp++;

        // trace header
        l_trace_head = *pl_tmp++;
        //printf ("trace header \n");
        if ( ((l_trace_head & 0xff000000) >> 24) != 0xaa)
        {
          printf ("ERROR>> trace header id is not 0xaa \n");
          goto bad_event; 
        }

        l_fpga_filt_on_off = (l_trace_head & 0x80000) >> 19;
        //l_fpga_filt_mode   = (l_trace_head & 0x40000) >> 18;
        //printf ("fpga filter on bit: %d, fpga filter mode: %d \n", l_fpga_filt_on_off, l_fpga_filt_mode);
        //fflush (stdout);
        //sleep (1);

        if (l_trig_type == 1) // physics event
        {
          if (l_first_trace[l_sfp_id][l_feb_id] == 0)
          {
            l_first_trace[l_sfp_id][l_feb_id] = 1; 
            h_hitpat_tr[l_sfp_id][l_feb_id]->Fill (-1, 1);
          }
          h_hitpat_tr[l_sfp_id][l_feb_id]->Fill (l_cha_id, 1);
          l_ch_hitpat_tr[l_sfp_id][l_feb_id][l_cha_id]++;

          // now trace
          l_trace_size = (l_cha_size/4) - 2;     // in longs/32bit

          //das folgende kommentierte noch korrigieren!
          //falls trace + filter trace: cuttoff bei 2000 slices, da 4fache datenmenge!

          //if (l_trace_size != (TRACE_SIZE>>1))
          //{
          //  printf ("ERROR>> l_trace_size: %d \n", l_trace_size); fflush (stdout);
          //  goto bad_event;
          //}

          if (l_fpga_filt_on_off == 0) // only trace. no fpga filter trace data
          {
            for (l_l=0; l_l<l_trace_size; l_l++)   // loop over traces 
            {
              // disentangle data
              l_dat_fir = *pl_tmp++;
              l_dat_sec = l_dat_fir;

              #ifdef USE_MBS_PARAM
              l_adc_type = (l_trace_head & 0x800000) >> 23; 
              #else
              l_adc_type = (l_sfp_adc_type[l_sfp_id] >> l_feb_id) & 0x1;              
              #endif
            
              if (l_adc_type == 0) // 12 bit
              {
                l_dat_fir =  l_dat_fir        & 0xfff;
                l_dat_sec = (l_dat_sec >> 16) & 0xfff;
              }

              if (l_adc_type == 1)  // 14 bit
              { 
                l_dat_fir =  l_dat_fir        & 0x3fff;
                l_dat_sec = (l_dat_sec >> 16) & 0x3fff;
              } 

              h_trace[l_sfp_id][l_feb_id][l_cha_id]->SetBinContent (l_l*2  +1, l_dat_fir);
              h_trace[l_sfp_id][l_feb_id][l_cha_id]->SetBinContent (l_l*2+1+1, l_dat_sec);

              h_adc_spect[l_sfp_id][l_feb_id][l_cha_id]->Fill (l_dat_fir * ADC_RES - 1000.);
              h_adc_spect[l_sfp_id][l_feb_id][l_cha_id]->Fill (l_dat_sec * ADC_RES - 1000.);

              l_tr[l_l*2]   = l_dat_fir;
              l_tr[l_l*2+1] = l_dat_sec;
            }
            l_trace_size = l_trace_size * 2; //??? JAM
          }

          if (l_fpga_filt_on_off == 1) // trace AND fpga filter data
          {
            for (l_l=0; l_l<(l_trace_size>>1); l_l++)   // loop over traces 
            {
              // disentangle data
              l_dat_trace = *pl_tmp++;
              l_dat_filt  = *pl_tmp++;
              l_filt_sign  =  (l_dat_filt & 0x800000) >> 23;

              #ifdef USE_MBS_PARAM
              l_adc_type = (l_trace_head & 0x800000) >> 23; 
              #else
              l_adc_type = (l_sfp_adc_type[l_sfp_id] >> l_feb_id) & 0x1;              
              #endif
            
              if (l_adc_type == 0) // 12 bit
              {
                l_dat_trace = l_dat_trace  & 0xfff;
              }

              if (l_adc_type == 1)  // 14 bit
              {
                l_dat_trace = l_dat_trace  & 0x3fff;
              } 
              
              l_dat_filt  = l_dat_filt   & 0x7fffff;
              if (l_filt_sign == 1) {l_dat_filt = l_dat_filt * -1;}

              h_trace      [l_sfp_id][l_feb_id][l_cha_id]->SetBinContent (l_l+1, l_dat_trace);
              h_trapez_fpga[l_sfp_id][l_feb_id][l_cha_id]->SetBinContent (l_l+1, l_dat_filt);

              l_tr[l_l] = l_dat_trace;
            }
            l_trace_size = l_trace_size >> 1;
          }

          // find base line value of trace and correct it to baseline 0
          f_bls_val = 0.;
          for (l_l=l_bls_start; l_l<l_bls_stop; l_l++) 
          {
            f_bls_val += (Double_t)l_tr[l_l];
          }
          f_bls_val = f_bls_val / (Double_t)(l_bls_stop - l_bls_start); 
          for (l_l=0; l_l<l_trace_size; l_l++)   // create baseline restored trace 
          {
            f_tr_blr[l_l] =  (Double_t)l_tr[l_l] - f_bls_val;
            h_trace_blr[l_sfp_id][l_feb_id][l_cha_id]->Fill (l_l, f_tr_blr[l_l] * ADC_RES);
            //h_trace_blr[l_sfp_id][l_feb_id][l_cha_id]->SetBinContent (l_l+1, f_tr_blr[l_l]);
          }

          #ifdef IVAN
          r_ivan_f_prev = 0;
          for (l_l=1; l_l<l_trace_size; l_l++)   // loop over traces 
          {
            r_ivan_f = (C1 * l_tr[l_l]) - (C2 * l_tr[l_l-1]) + (C3 * r_ivan_f_prev);
            h_ivan_f[l_sfp_id][l_feb_id][l_cha_id]->Fill (l_l, r_ivan_f);
            r_ivan_f_prev = r_ivan_f;
          }
          #endif //IVAN

          #ifdef FFT_GSI
          for (l_l=0; l_l<l_tr_fft_si; l_l++)
          {
            f_f_real[l_l] = (Double_t)l_tr[l_l];
          }
          fft_gsi (1, l_pct, f_f_real, f_f_imag); 
          for (l_l=0; l_l<l_tr_fft_si; l_l++)
          {
            h_fft_real[l_sfp_id][l_feb_id][l_cha_id]->Fill (l_l, f_f_real[l_l]);
            h_fft_imag[l_sfp_id][l_feb_id][l_cha_id]->Fill (l_l, f_f_imag[l_l]);

            d_re = h_fft_real[l_sfp_id][l_feb_id][l_cha_id]->GetBinContent (l_l);
            d_im = h_fft_imag[l_sfp_id][l_feb_id][l_cha_id]->GetBinContent (l_l);
            d_am = sqrt (pow (d_re, 2) + pow (d_im, 2));
            h_fft_ampl[l_sfp_id][l_feb_id][l_cha_id]->Fill (l_l, d_am);
            d_ph = 0.;
            if (d_re != 0.)
            { 
              d_ph = atan (d_im/d_re);
            }
            h_fft_phas[l_sfp_id][l_feb_id][l_cha_id]->Fill (l_l, d_ph);
          }      
          #endif // FFT_GSI

          #ifdef TRAPEZ
          l_A1 = 0;
          l_A2 = 0;
          for (l_l=(l_gap+l_win); l_l<l_trace_size; l_l++)   // loop over traces 
          {
            if (l_l < (l_gap + (2*l_win)))
            {
              //l_A1 += l_tr[l_l] - l_tr[l_l-l_win];
              l_A1 += l_tr[l_l];
              l_A2 += l_tr[l_l-l_win-l_gap];
              //printf ("index: %d, A1: %d, A2 %d, A1-A2: %d, norm %d  \n", l_l, l_A1, l_A2, l_A1-l_A2, l_win);   
              //printf ("(A1-A2)/norm: %d, %f \n", (l_A1 - l_A2) / l_win, (Real_t)(l_A1 - l_A2) / (Real_t) l_win); 
            }
            else
            {
              l_A1 += l_tr[l_l]  - l_tr[l_l-l_win];
              l_A2 += l_tr[l_l-l_win-l_gap] - l_tr[l_l-(2*l_win)-l_gap]; 
        
              if (l_l < (l_trace_size -(2*l_win)-l_gap))
              {
                //h_trapez_f[l_sfp_id][l_feb_id][l_cha_id]->Fill (l_l, (Real_t)(l_A1 - l_A2) / (Real_t) l_win);
                h_trapez_f[l_sfp_id][l_feb_id][l_cha_id]->Fill (l_l, (Real_t)(l_A1 - l_A2));
              }
            }     
          }
          //printf ("next trace \n\n\n\n");
          //sleep (1);
          #endif // TRAPEZ

          // find peak and fill histogram
          h_peak  [l_sfp_id][l_feb_id][l_cha_id]->Fill (h_trace[l_sfp_id][l_feb_id][l_cha_id]->GetMaximum ());
          h_valley[l_sfp_id][l_feb_id][l_cha_id]->Fill (h_trace[l_sfp_id][l_feb_id][l_cha_id]->GetMinimum ());

          #ifdef TRAPEZ
          if (l_pol == 1) // negative signals
          { 
            h_trapez_e[l_sfp_id][l_feb_id][l_cha_id]->Fill (h_trapez_f[l_sfp_id][l_feb_id][l_cha_id]->GetMinimum ());
            l_trapez_e_found [l_sfp_id][l_feb_id][l_cha_id] = 1;
            l_trapez_e[l_sfp_id][l_feb_id][l_cha_id] = h_trapez_f[l_sfp_id][l_feb_id][l_cha_id]->GetMinimum ();
          }
          if (l_pol == 0) // positive signals
          { 
            h_trapez_e[l_sfp_id][l_feb_id][l_cha_id]->Fill (h_trapez_f[l_sfp_id][l_feb_id][l_cha_id]->GetMaximum ());
            l_trapez_e_found [l_sfp_id][l_feb_id][l_cha_id] = 1;
            l_trapez_e[l_sfp_id][l_feb_id][l_cha_id] = h_trapez_f[l_sfp_id][l_feb_id][l_cha_id]->GetMaximum ();
          }
          #endif // TRAPEZ

#ifdef MWD
          l_tau_sum = 0;
          for (l_l=1; l_l<l_mwd_wind; l_l++)
          {
            l_tau_sum += f_tr_blr[l_mwd_wind - l_l]; 
          } 

          f_sum = 0.;
          for (l_l=l_mwd_wind; l_l<l_trace_size; l_l++)   // mwd 
          {
            f_sum =  f_tr_blr[l_l] - f_tr_blr[l_l - l_mwd_wind];
            f_sum += (f_rev_tau * (Double_t)l_tau_sum);
            f_mwd[l_l] = f_sum;

            h_mwd_f[l_sfp_id][l_feb_id][l_cha_id]->Fill (l_l, f_sum);

            // correct f_tau_sum for next index
            l_tau_sum += f_tr_blr[l_l] - f_tr_blr[l_l - l_mwd_wind];
          }

          f_sum_e = 0.;
          for (l_l=l_mwd_wind; l_l<l_trace_size; l_l++) // averaging
          {
            f_sum_e += f_mwd[l_l] - f_mwd[l_l - l_mwd_avg];
            if (l_l>(l_mwd_wind+l_mwd_avg)) 
            {
              f_avg[l_l] = f_sum_e/(Double_t)l_mwd_avg;
              h_mwd_a[l_sfp_id][l_feb_id][l_cha_id]->Fill (l_l, f_avg[l_l]);
            }
          }

          if (l_pol == 1) // negative signals
          { 
            h_mwd_e[l_sfp_id][l_feb_id][l_cha_id]
             ->Fill (h_mwd_a[l_sfp_id][l_feb_id][l_cha_id]->GetMinimum ());
          }
          if (l_pol == 0) // positive signals
          { 
            h_mwd_e[l_sfp_id][l_feb_id][l_cha_id]
             ->Fill (h_mwd_a[l_sfp_id][l_feb_id][l_cha_id]->GetMaximum ());
          }
          #endif // MWD

          #ifdef APFEL
          if (l_pol == 1) // negative signals
          { 
            h_apfel_e[l_sfp_id][l_feb_id][l_cha_id]->Fill (h_trace_blr[l_sfp_id][l_feb_id][l_cha_id]->GetMinimum ());
          }
          if (l_pol == 0) // positive signals
          { 
            h_apfel_e[l_sfp_id][l_feb_id][l_cha_id]->Fill (h_trace_blr[l_sfp_id][l_feb_id][l_cha_id]->GetMaximum ());
          }

          #ifdef APFEL_INT
          f_sum_a = 0.; 
          for (l_l=0; l_l<l_trace_size; l_l++)
          {
            //f_sum_a += (Double_t)h_trace_blr[l_sfp_id][l_feb_id][l_cha_id]->GetBinContent (l_l+1);
            f_sum_a += f_tr_blr[l_l]; 
            f_sum_tr[l_l] = f_sum_a;
            h_sum_trace[l_sfp_id][l_feb_id][l_cha_id]->Fill (l_l, f_sum_a);
          }

          l_A1_a = 0;
          l_A2_a = 0;
          for (l_l=(l_gap_a+l_win_a); l_l<l_trace_size; l_l++)   // loop over traces 
          {
            if (l_l < (l_gap_a + (2*l_win_a)))
            {
              l_A1_a += f_sum_tr[l_l];
              l_A2_a += f_sum_tr[l_l-l_win_a-l_gap_a];
            }
            else
            {
              l_A1_a += f_sum_tr[l_l]  - f_sum_tr[l_l-l_win_a];
              l_A2_a += f_sum_tr[l_l-l_win_a-l_gap_a] - f_sum_tr[l_l-(2*l_win_a)-l_gap_a]; 
        
              if (l_l < (l_trace_size -(2*l_win_a)-l_gap_a))
              {
                h_apf_trap_f[l_sfp_id][l_feb_id][l_cha_id]->Fill (l_l, (Real_t)(l_A1_a - l_A2_a) / (Real_t) l_win_a);
              }
            }     
          }
          if (l_pol == 1) // negative signals
          { 
            //h_apf_trap_e[l_sfp_id][l_feb_id][l_cha_id]->Fill (h_apf_trap_f[l_sfp_id][l_feb_id][l_cha_id]->GetMinimum ());
            h_apf_trap_e[l_sfp_id][l_feb_id][l_cha_id]->Fill (h_apf_trap_f[l_sfp_id][l_feb_id][l_cha_id]->GetBinContent (700));
          }
          if (l_pol == 0) // positive signals
          { 
            //h_apf_trap_e[l_sfp_id][l_feb_id][l_cha_id]->Fill (h_apf_trap_f[l_sfp_id][l_feb_id][l_cha_id]->GetMaximum ());
            h_apf_trap_e[l_sfp_id][l_feb_id][l_cha_id]->Fill (h_apf_trap_f[l_sfp_id][l_feb_id][l_cha_id]->GetBinContent (700));
          }
          #endif // APFEL_INT

          #endif // APFEL
        }

        // jump over trace
        //pl_tmp += (l_cha_size >> 2) - 2;          
            
        // trace trailer
        //printf ("trace trailer \n");
        l_trace_trail = *pl_tmp++;
        if ( ((l_trace_trail & 0xff000000) >> 24) != 0xbb)
        {
          printf ("ERROR>> trace trailer id is not 0xbb, ");
          printf ("SFP: %d, FEB: %d, CHA: %d \n", l_sfp_id, l_feb_id, l_cha_id); 
          goto bad_event; 
        }
      }
    }
    else
    {
      printf ("ERROR>> data word neither channel header nor padding word \n");
    }       
  }


  for (l_i=0; l_i<MAX_SFP; l_i++)
  {
    if (l_sfp_slaves[l_i] != 0)
    {
      for (l_j=0; l_j<l_sfp_slaves[l_i]; l_j++)
      {
        for (l_k=0; l_k<N_CHA; l_k++)
        {
          h_ch_hitpat   [l_i][l_j][l_k]->Fill (l_ch_hitpat   [l_i][l_j][l_k]);  
          h_ch_hitpat_tr[l_i][l_j][l_k]->Fill (l_ch_hitpat_tr[l_i][l_j][l_k]);  

          if ( (l_trapez_e_found[l_i][l_j][l_k] == 1) && (l_fpga_e_found[l_i][l_j][l_k] == 1))
          {
            value=0;
            if(l_trapez_e[l_i][l_j][l_k])
              value=(Double_t)l_fpga_e[l_i][l_j][l_k] / (Double_t)l_trapez_e[l_i][l_j][l_k];
            h_corr_e_fpga_trapez[l_i][l_j][l_k]->Fill(value);
            //printf ("Energy FPGA / Energy Trace: %1.10f \n",value);

            // JAM 12-12-2023: EXAMPLE -put evaluated energy for each channel to output event here:
            fOutEvent->fE_FPGA_Trapez[l_i][l_j][l_k]=value;
            fOutEvent->SetValid(kTRUE);
          }
        }
      }
    }
  }

  #ifdef TEST_FFT_GSI
  for (l_l=1; l_l<l_tr_fft_si; l_l++) 
  {
    
    if (l_l < 950 || l_l > 1000)
    {
      f_test_real[l_l] = 0.;
    }
    else
    {
      f_test_real[l_l] = 100.;
    }
 

    //f_test_real[l_l] = cos (4 * TWOPI * ((Double_t)l_l/(Double_t)l_tr_fft_si));
    f_test_imag[l_l] = 0.;
    h_test->Fill (l_l, f_test_real[l_l]);
  }
  fft_gsi (1, 11, f_test_real, f_test_imag); 
  for (l_l=1; l_l<l_tr_fft_si; l_l++)  
  { 
    h_test_fft_real->Fill (l_l, f_test_real[l_l]);
    h_test_fft_imag->Fill (l_l, f_test_imag[l_l]);
  }      
  #endif // TEST_FFT_GSI


  // JAM 12/2023: copy here values to output event for optional ROOT tree storage
#ifdef TPLEIADES_FILL_TRACES

  for (l_i=0; l_i<MAX_SFP; l_i++)
   {
     if (l_sfp_slaves[l_i] != 0)
     {
       for (l_j=0; l_j<l_sfp_slaves[l_i]; l_j++)
       {
         for (l_k=0; l_k<N_CHA; l_k++)
         {

             for(int bin=1; bin<h_trace[l_i][l_j][l_k]->GetNbinsX(); ++bin)
             {
                 value=h_trace[l_i][l_j][l_k]->GetBinContent(bin);
                 fOutEvent->fTrace[l_i][l_j][l_k].push_back(value);
             }

             for(int bin=1; bin<h_trace_blr[l_i][l_j][l_k]->GetNbinsX(); ++bin)
             {
               value=h_trace_blr[l_i][l_j][l_k]->GetBinContent(bin);
               fOutEvent->fTraceBLR[l_i][l_j][l_k].push_back(value);
             }

             for(int bin=1; bin<h_trapez_fpga[l_i][l_j][l_k]->GetNbinsX(); ++bin)
             {
               value=h_trapez_fpga[l_i][l_j][l_k]->GetBinContent(bin);
               fOutEvent->fTrapezFPGA[l_i][l_j][l_k].push_back(value);
             }
         }// l_k
       }// l_j
     }// if (l_sfp_slaves
   } // l_i
  fOutEvent->SetValid(kTRUE);
#endif
  //printf ("check next event \n"); sleep (1);

bad_event:

// JAM 12-Dec-2023: Added slow motion (eventwise step mode) as example how to use parameter container flag
if (fPar->fSlowMotion)
{
  Int_t evnum = fInput->GetCount();
    GO4_STOP_ANALYSIS_MESSAGE(
      "Stopped for slow motion mode after MBS event count %d. Click green arrow for next event. please.", evnum);
}



  return kTRUE;
}

//--------------------------------------------------------------------------------

/*
   This computes an in-place complex-to-complex FFT_GSI 
   x and y are the real and imaginary arrays of 2^m points.
   dir =  1 gives forward transform
   dir = -1 gives reverse transform 
*/

void TPLEIADESProc:: fft_gsi (Int_t dir, Int_t m, Double_t *x, Double_t *y)
{
  long n,i,i1,j,k,i2,l,l1,l2;
  double c1,c2,tx,ty,t1,t2,u1,u2,z;

  /* Calculate the number of points */
  n = 1;
  for (i=0;i<m;i++) 
    n *= 2;

  /* Do the bit reversal */
  i2 = n >> 1;
  j = 0;
  for (i=0;i<n-1;i++) {
    if (i < j) {
      tx = x[i];
      ty = y[i];
      x[i] = x[j];
      y[i] = y[j];
      x[j] = tx;
      y[j] = ty;
    }
    k = i2;
    while (k <= j) {
      j -= k;
      k >>= 1;
    }
    j += k;
  }

  /* Compute the FFT_GSI */
  c1 = -1.0; 
  c2 = 0.0;
  l2 = 1;
  for (l=0;l<m;l++) {
    l1 = l2;
    l2 <<= 1;
    u1 = 1.0; 
    u2 = 0.0;
    for (j=0;j<l1;j++) {
      for (i=j;i<n;i+=l2) {
        i1 = i + l1;
        t1 = u1 * x[i1] - u2 * y[i1];
        t2 = u1 * y[i1] + u2 * x[i1];
        x[i1] = x[i] - t1; 
        y[i1] = y[i] - t2;
        x[i] += t1;
        y[i] += t2;
      }
      z =  u1 * c1 - u2 * c2;
      u2 = u1 * c2 + u2 * c1;
      u1 = z;
    }
    c2 = sqrt((1.0 - c1) / 2.0);
    if (dir == 1) 
      c2 = -c2;
    c1 = sqrt((1.0 + c1) / 2.0);
  }

  /* Scaling for forward transform */
  if (dir == 1) {
    for (i=0;i<n;i++) {
      x[i] /= n;
      y[i] /= n;
    }
  }
   
  //return(1);
}


//--------------------------------------------------------------------------------------------------------

void TPLEIADESProc:: f_make_histo (Int_t l_mode)
{
  Text_t chis[256];
  Text_t chead[256];
  UInt_t l_i, l_j, l_k;
  UInt_t l_tra_size;
  UInt_t l_trap_n_avg;
  //UInt_t l_left;
  //UInt_t l_right;

  #ifdef USE_MBS_PARAM
  l_tra_size   = l_trace & 0xffff;
  l_trap_n_avg = l_e_filt >> 21;
  printf ("f_make_histo: trace size: %d, avg size %d \n", l_tra_size, l_trap_n_avg);
  fflush (stdout);
  l_sfp_slaves[0] =  l_slaves & 0xff;   
  l_sfp_slaves[1] = (l_slaves & 0xff00)     >>  8;
  l_sfp_slaves[2] = (l_slaves & 0xff0000)   >> 16;
  l_sfp_slaves[3] = (l_slaves & 0xff000000) >> 24;
  printf ("f_make_histo: # of sfp slaves: 3:%d, 2:%d, 1: %d, 0: %d \n",
          l_sfp_slaves[3], l_sfp_slaves[2], l_sfp_slaves[1], l_sfp_slaves[0]);
  fflush (stdout);
  #else
  l_tra_size   = TRACE_SIZE;
  l_trap_n_avg = TRAPEZ_N_AVG;
  #endif // USE_MBS_PARAM      

  if (l_first2 == 0)
  {
    l_first2 = 1;

    #ifdef FFT_GSI
    l_pct = 0;
    l_tr_fft_si = l_tra_size;

    printf ("trace lenght: %d \n", l_tr_fft_si); fflush (stdout);  
    while (1)
    {
      if (l_tr_fft_si > 0)
      {
        l_tr_fft_si = l_tr_fft_si/2;
        l_pct++;
      }
      else
      {
        l_pct--;
        l_tr_fft_si = pow (2,l_pct);
        printf ("FFT trace length: %d / power: %d \n", l_tr_fft_si, l_pct); fflush (stdout);
        break;     
      }  
    }
    #endif // FFT_GSI
  }

  for (l_i=0; l_i<MAX_SFP; l_i++)
  {
    if (l_sfp_slaves[l_i] != 0)
    {
      for (l_j=0; l_j<l_sfp_slaves[l_i]; l_j++)
      {
 
        for (l_k=0; l_k<N_CHA; l_k++)
        {
          sprintf(chis,"Traces/TRACE  SFP: %2d FEBEX: %2d CHAN: %2d", l_i, l_j, l_k);
          sprintf(chead,"Trace");
          h_trace[l_i][l_j][l_k] = MakeTH1('I', chis,chead,l_tra_size,0,l_tra_size);
        }

        for (l_k=0; l_k<N_CHA; l_k++)
        {
          sprintf(chis,"Traces BLR/TRACE, base line restored (ordinate in mV) SFP: %2d FEBEX: %2d CHAN: %2d", l_i, l_j, l_k);
          sprintf(chead,"Trace, base line restored");
          h_trace_blr[l_i][l_j][l_k] = MakeTH1('F', chis,chead,l_tra_size,0,l_tra_size);
        }

        #ifdef APFEL
        for (l_k=0; l_k<N_CHA; l_k++)
        {
          sprintf(chis,"APFEL Energy/APFEL Energy   SFP: %2d FEBEX: %2d CHAN: %2d", l_i, l_j, l_k);
          sprintf(chead,"Energy");
          h_apfel_e[l_i][l_j][l_k] = MakeTH1('F', chis,chead,0x2000,-0x1000,0x1000);
        }
        
        #ifdef APFEL_INT
        for (l_k=0; l_k<N_CHA; l_k++)
        {
          sprintf(chis,"Sum Traces/Sum Traces  SFP: %2d FEBEX: %2d CHAN: %2d", l_i, l_j, l_k);
          sprintf(chead,"SUM Trace from base line restored trace");
          h_sum_trace[l_i][l_j][l_k] = MakeTH1('F', chis,chead,l_tra_size,0,l_tra_size);
        }

        for (l_k=0; l_k<N_CHA; l_k++)
        {
          sprintf(chis,"TRAPEZ from Sum/Sum TRAPEZ Filter  SFP: %2d FEBEX: %2d CHAN: %2d", l_i, l_j, l_k);
          sprintf(chead,"TRAPEZ Filter from Sum of APFEL trace");
          h_apf_trap_f[l_i][l_j][l_k] = MakeTH1('F', chis,chead,l_tra_size,0,l_tra_size);
        }

        for (l_k=0; l_k<N_CHA; l_k++)
        {
          sprintf(chis,"APFEL Trapez Energy/APFEL Trapez Energy   SFP: %2d FEBEX: %2d CHAN: %2d", l_i, l_j, l_k);
          sprintf(chead,"Energy from Sum Trace and trapezoidal filter");
          h_apf_trap_e[l_i][l_j][l_k] = MakeTH1('F', chis,chead,0x2000,-0x18000,0x18000);
        }
        #endif // APFEL_INT
        #endif // APFEL

        #ifdef TRAPEZ 
        for (l_k=0; l_k<N_CHA; l_k++)
        {
          sprintf(chis,"TRAPEZ/TRAPEZ Filter  SFP: %2d FEBEX: %2d CHAN: %2d", l_i, l_j, l_k);
          sprintf(chead,"TRAPEZ Filter");
          h_trapez_f[l_i][l_j][l_k] = MakeTH1('F', chis,chead,l_tra_size,0,l_tra_size);
        }

        for (l_k=0; l_k<N_CHA; l_k++)
        {
          sprintf(chis,"TRAPEZ Energy/TRAPEZ Energy   SFP: %2d FEBEX: %2d CHAN: %2d", l_i, l_j, l_k);
          sprintf(chead,"Energy");
          //h_trapez_e[l_i][l_j][l_k] = MakeTH1('F', chis,chead,0x8000,(-1)*0x1000*TRAPEZ_N_AVG,0x1000*TRAPEZ_N_AVG);
          //h_trapez_e[l_i][l_j][l_k] = MakeTH1('F', chis,chead,0x20000,-2000000,2000000);
          h_trapez_e[l_i][l_j][l_k] = MakeTH1('F', chis,chead,0x20000,-200000,200000);
        }
        #endif // TRAPEZ

        #ifdef MWD 
        for (l_k=0; l_k<N_CHA; l_k++)
        {
          sprintf(chis,"MWD Filter/MWD Filter  SFP: %2d FEBEX: %2d CHAN: %2d", l_i, l_j, l_k);
          sprintf(chead,"MWD Filter");
          h_mwd_f[l_i][l_j][l_k] = MakeTH1('F', chis,chead,l_tra_size,0,l_tra_size);
        }

        for (l_k=0; l_k<N_CHA; l_k++)
        {
          sprintf(chis,"MWD Average/MWD Average  SFP: %2d FEBEX: %2d CHAN: %2d", l_i, l_j, l_k);
          sprintf(chead,"MWD Average");
          h_mwd_a[l_i][l_j][l_k] = MakeTH1('F', chis,chead,l_tra_size,0,l_tra_size);
        }

        for (l_k=0; l_k<N_CHA; l_k++)
        {
          sprintf(chis,"MWD Energy/MWD Energy   SFP: %2d FEBEX: %2d CHAN: %2d", l_i, l_j, l_k);
          sprintf(chead,"Energy");
          h_mwd_e[l_i][l_j][l_k] = MakeTH1('F', chis,chead,0x20000,-0x1000,0x1000);
        }
        #endif // MWD

        #ifdef IVAN 
        for (l_k=0; l_k<N_CHA; l_k++)
        {
          sprintf(chis,"IVAN/Ivan Filter  SFP: %2d FEBEX: %2d CHAN: %2d", l_i, l_j, l_k);
          sprintf(chead,"IVAN Filter");
          h_ivan_f[l_i][l_j][l_k] = MakeTH1('F', chis,chead,l_tra_size,0,l_tra_size);
        }
        #endif // IVAN

        #ifdef TEST_FFT_GSI 
        sprintf(chis,"FFT_Test/Test function");
        sprintf(chead,"FFT Test func");
        h_test = MakeTH1('F', chis,chead,l_tr_fft_si,0,l_tr_fft_si);
        sprintf(chis,"FFT_Test/FFT real Test function");
        sprintf(chead,"FFT real Test func");
        h_test_fft_real = MakeTH1('F', chis,chead,l_tr_fft_si,0,l_tr_fft_si);
        sprintf(chis,"FFT_Test/FFT imag Test function");
        sprintf(chead,"FFT imag Test func");
        h_test_fft_imag = MakeTH1('F', chis,chead,l_tr_fft_si,0,l_tr_fft_si);
        #endif // TEST_FFT_GSI       

        #ifdef FFT_GSI
        for (l_k=0; l_k<N_CHA; l_k++)
        {
          sprintf(chis,"FFTreal/Frequency spectrum (real part)  SFP: %2d FEBEX: %2d CHAN: %2d", l_i, l_j, l_k);
          sprintf(chead,"FFT Filter (real part)");
          h_fft_real[l_i][l_j][l_k] = MakeTH1('F', chis,chead,l_tr_fft_si,0,l_tr_fft_si);
        }
        for (l_k=0; l_k<N_CHA; l_k++)
        {
          sprintf(chis,"FFTimag/Frequency spectrum (imag part)  SFP: %2d FEBEX: %2d CHAN: %2d", l_i, l_j, l_k);
          sprintf(chead,"FFT Filter (imag part)");
          h_fft_imag[l_i][l_j][l_k] = MakeTH1('F', chis,chead,l_tr_fft_si,0,l_tr_fft_si);
        }
        for (l_k=0; l_k<N_CHA; l_k++)
        {
          sprintf(chis,"FFTamplitude/Frequency spectrum amplitude  SFP: %2d FEBEX: %2d CHAN: %2d", l_i, l_j, l_k);
          sprintf(chead,"FFT Filter (amplitude)");
          h_fft_ampl[l_i][l_j][l_k] = MakeTH1('F', chis,chead,l_tr_fft_si,0,l_tr_fft_si);
        }
        for (l_k=0; l_k<N_CHA; l_k++)
        {
          sprintf(chis,"FFTphase/Frequency spectrum phase  SFP: %2d FEBEX: %2d CHAN: %2d", l_i, l_j, l_k);
          sprintf(chead,"FFT Filter (phase)");
          h_fft_phas[l_i][l_j][l_k] = MakeTH1('F', chis,chead,l_tr_fft_si,0,l_tr_fft_si);
        }
        #endif // FFT_GSI

        for (l_k=0; l_k<N_CHA; l_k++)
        {
          sprintf(chis,"FPGA/FPGA Trapez   SFP: %2d FEBEX: %2d CHAN: %2d", l_i, l_j, l_k);
          sprintf(chead,"FPGA Trapez");
          h_trapez_fpga[l_i][l_j][l_k] = MakeTH1('F', chis,chead,l_tra_size,0,l_tra_size);
        }
        for (l_k=0; l_k<N_CHA; l_k++)
        {
          sprintf(chis,"FPGA/FPGA Energy(hitlist)  SFP: %2d FEBEX: %2d CHAN: %2d", l_i, l_j, l_k);
          sprintf(chead,"FPGA Energy");
          //h_fpga_e[l_i][l_j][l_k] = MakeTH1('F', chis,chead,0x8000,(-1)*0x1000*TRAPEZ_N_AVG,0x1000*TRAPEZ_N_AVG);
          h_fpga_e[l_i][l_j][l_k] = MakeTH1('F', chis,chead,0x10000,-2000000,2000000);
        }
        for (l_k=0; l_k<N_CHA; l_k++)
        {
          sprintf(chis,"E_CORR/Energy(hitlist) vs TRAPEZ Energy  SFP: %2d FEBEX: %2d CHAN: %2d", l_i, l_j, l_k);
          sprintf(chead,"(FPGA Energy)/(TRAPEZ Energy)");
          h_corr_e_fpga_trapez[l_i][l_j][l_k] = MakeTH1('F', chis,chead,2000,0.999,1.001);
        }

        for (l_k=0; l_k<N_CHA; l_k++)
        {
          sprintf(chis,"Peaks/PEAK   SFP: %2d FEBEX: %2d CHAN: %2d", l_i, l_j, l_k);
          sprintf(chead,"Peak");
          h_peak[l_i][l_j][l_k] = MakeTH1('I', chis,chead,0x1000,0,0x4000);
        }
        for (l_k=0; l_k<N_CHA; l_k++)
        {
          sprintf(chis,"Valleys/VALLEY   SFP: %2d FEBEX: %2d CHAN: %2d", l_i, l_j, l_k);
          sprintf(chead,"Valley");
          h_valley[l_i][l_j][l_k] = MakeTH1('I', chis,chead,0x1000,0,0x4000);
        }
        for (l_k=0; l_k<N_CHA; l_k++)
        {
          sprintf(chis,"Timediff/Trigger time - Hit time   SFP: %2d FEBEX: %2d CHAN: %2d", l_i, l_j, l_k);
          sprintf(chead,"TRIG-HIT");
          h_trgti_hitti[l_i][l_j][l_k] = MakeTH1('I', chis,chead,2000,-1000,1000);
        }
        for (l_k=0; l_k<N_CHA; l_k++)
        {
          sprintf(chis,"Hitpat_Cha_List/Channel hit pattern per event (list)  SFP: %2d FEBEX: %2d CHAN: %2d", l_i, l_j, l_k);
          sprintf(chead,"HITPAT_Cha_List");
          h_ch_hitpat[l_i][l_j][l_k] = MakeTH1('I', chis,chead,11,-1,10);
        }

        sprintf(chis,"Hitpat_Feb_List/Hit Pattern (list)  SFP: %2d FEBEX: %2d", l_i, l_j);
        sprintf(chead,"Hitpat_List");
        h_hitpat[l_i][l_j] = MakeTH1('I', chis,chead,20,-2,18);

        for (l_k=0; l_k<N_CHA; l_k++)
        {
          sprintf(chis,"Hitpat_Cha_Trace/Channel hit pattern per event (trace)  SFP: %2d FEBEX: %2d CHAN: %2d", l_i, l_j, l_k);
          sprintf(chead,"HITPAT_Cha_Trace");
          h_ch_hitpat_tr[l_i][l_j][l_k] = MakeTH1('I', chis,chead,11,-1,10);
        }
        sprintf(chis,"Hitpat_Feb_Trace/Hit Pattern (trace)  SFP: %2d FEBEX: %2d", l_i, l_j);
        sprintf(chead,"Hitpat_Trace");
        h_hitpat_tr[l_i][l_j] = MakeTH1('I', chis,chead,20,-2,18);

        for (l_k=0; l_k<N_CHA; l_k++)
        {
          sprintf(chis,"Hitpat_Cha_Diff/Channel hit pattern per event (diff)  SFP: %2d FEBEX: %2d CHAN: %2d", l_i, l_j, l_k);
          sprintf(chead,"HITPAT_Cha_Diff");
          h_ch_hitpat_di[l_i][l_j][l_k] = MakeTH1('I', chis,chead,11,-1,10);
        }
        sprintf(chis,"Hitpat_Feb_Diff/Hit Pattern (diff)  SFP: %2d FEBEX: %2d", l_i, l_j);
        sprintf(chead,"Hitpat_Trace");
        h_hitpat_di[l_i][l_j] = MakeTH1('I', chis,chead,20,-2,18);

        for (l_k=0; l_k<N_CHA; l_k++)
        {
          sprintf(chis,"ADC_Spectra/ADC Spectrum [mV]  SFP: %2d FEBEX: %2d CHAN: %2d", l_i, l_j, l_k);
          sprintf(chead,"ADC Spectrum");
          h_adc_spect[l_i][l_j][l_k] = MakeTH1('D', chis,chead,16384,-1000,1000);
        }
      }
    }
  }

  printf ("Leave f_make_histo, mode: %d \n", l_mode);
  fflush (stdout);
}



//----------------------------END OF GO4 SOURCE FILE ---------------------
