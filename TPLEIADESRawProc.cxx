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
#endif // USE_MBS_PARAM

static UInt_t    l_adc_type;
static UInt_t    l_more_1_hit_ct=0;

static UInt_t    l_first  = 0;
static UInt_t    l_first2 = 0;

//------------------------------------------------------------------------
TPLEIADESRawProc::TPLEIADESRawProc() : TGo4EventProcessor("Proc")
{
    cout << "**** TPLEIADESRawProc: Create instance " << endl;
}

//------------------------------------------------------------------------
TPLEIADESRawProc::~TPLEIADESRawProc()
{
    cout << "**** TPLEIADESRawProc: Delete instance " << endl;
    l_first  = 0; // JAM 13-Dec-2024: need to reset these flags to renew histogram handles when resubmitted from GUI!
    l_first2 = 0;
}

//------------------------------------------------------------------------
// this one is used in standard factory
TPLEIADESRawProc::TPLEIADESRawProc(const char* name) : TGo4EventProcessor(name)
{
    cout << "**** TPLEIADESRawProc: Create instance " << name << endl;
    fPar = dynamic_cast<TPLEIADESParam*>(MakeParameter("PLEIADESParam", "TPLEIADESParam", "set_PLEIADESParam.C"));
    if(fPar)
    {
        fPar->SetConfigBoards();
    }
    //printf ("Histograms created \n");  fflush (stdout);
}

//------------------------------------------------------------------------
// Build event is the unpacker function, it reads the MBS event and does stuff with it.
// Histograms are erased with the next word, so are mostly used for online analysis.
Bool_t TPLEIADESRawProc::BuildEvent(TGo4EventElement* target)
{
    // called by framework for each mbs input event.
    fOutEvent= dynamic_cast<TPLEIADESRawEvent*>  (target);
    if(fOutEvent==0) { GO4_STOP_ANALYSIS_MESSAGE("NEVER COME HERE: output event is not configured, wrong class!") }

    fOutEvent->SetValid(kFALSE); // initialize next output as not filled, i.e.it is only stored when something is in


    //------------------------------------------------------------------------
    // object definitions. these objects will be used in reading the data stream.

    UInt_t  l_i, l_j, l_k, l_l; // counters
    Double_t    l_value=0;      // placeholder for filling histograms

    uint32_t    *pl_se_dat;     // pointer to data word from MBS sub event
    uint32_t    *pl_tmp;        // temporary pointer for manipulation

    UInt_t  l_dat_len;          // data length
    UInt_t  l_dat_len_byte;     // data length in bytes

    UInt_t  l_dat;              // data word
    UInt_t  l_trig_type;        // trigger type. 1 = physics event. We don't use other trigger types.
    UInt_t  l_trig_type_triva;  // another trigger type, not sure why we need two??
    UInt_t  l_sfp_id;           // SFP ID (ie FEBEX crate)
    UInt_t  l_feb_id;           // FEBEX card ID
    UInt_t  l_cha_id;           // FEBEX Channel ID
    UInt_t  l_n_hit;            // number of hits for subevent
    //UInt_t  l_hit_id;
    UInt_t  l_hit_cha_id;       // hit channel id
    Long64_t    ll_time;        // hit time from special channel
    Long64_t    ll_trg_time;    // trigger time from special channel
    //Long64_t  ll_hit_time;
    UInt_t  l_ch_hitpat   [MAX_SFP][MAX_SLAVE][N_CHA];  // channel hit pattern float array for filling histograms
    UInt_t  l_ch_hitpat_tr[MAX_SFP][MAX_SLAVE][N_CHA];  // channel hit pattern for traces
    UInt_t  l_first_trace [MAX_SFP][MAX_SLAVE];         // tracks first trace in FEBEX to fill -1 bin in hist

    UInt_t  l_cha_head;         // 32bit word for channel header
    UInt_t  l_cha_size;         // 32bit word for channel size (ie length in words)
    UInt_t  l_trace_head;       // 32bit word for trace header
    UInt_t  l_trace_size;       // 32bit word for trace size
    UInt_t  l_trace_trail;      // 32bit word for checking trace trail

    UInt_t  l_spec_head;        // 32bit word for checking special channel header
    UInt_t  l_spec_trail;       // 32bit word for checking special channel trace trail
    UInt_t  l_n_hit_in_cha;     // number of hits for specific channel
    UInt_t  l_only_one_hit_in_cha;      // y/n for just one hit in channel
    UInt_t  l_more_than_1_hit_in_cha;   // y/n for more than one hit
    UInt_t  l_hit_time_sign;    // bit for polarity of hit time relative to trigger
     Int_t  l_hit_time;         // magnitude of hit time
    UInt_t  l_hit_cha_id2;      // hit channel for FPGA energy
    UInt_t  l_fpga_energy_sign; // bit for polarity of FPGA energy
     Int_t  l_fpga_energy;      // magnitude of FPGA energy

    UInt_t  l_trapez_e_found [MAX_SFP][MAX_SLAVE][N_CHA];   // array of y/n if TRAPEZ energy found
    UInt_t  l_fpga_e_found   [MAX_SFP][MAX_SLAVE][N_CHA];   // array of y/n if FPGA energy found
    UInt_t  l_trapez_e       [MAX_SFP][MAX_SLAVE][N_CHA];   // array of magnitude of TRAPEZ energy
    UInt_t  l_fpga_e         [MAX_SFP][MAX_SLAVE][N_CHA];   // array of magnitude of FPGA energy
    UInt_t  l_fpga_hitti     [MAX_SFP][MAX_SLAVE][N_CHA];   // array of hit time values from FPGA

    UInt_t  l_dat_fir;          // word for first half of trace
    UInt_t  l_dat_sec;          // word for second half of trace

    static UInt_t   l_evt_ct = 0;         // MBS event counter
    static UInt_t   l_evt_ct_phys = 0;  // physics event counter

    UInt_t  l_pol = 0;          // polarity of FEBEX cards

    UInt_t  l_bls_start = BASE_LINE_SUBT_START;     // start for trace baseline calculation
    UInt_t  l_bls_stop  = BASE_LINE_SUBT_START + BASE_LINE_SUBT_SIZE;       // stop for trace baseline calculation
    Double_t    f_bls_val = 0.; // value of trace baseline


    //------------------------------------------------------------------------
    // special function objects. only initialised if defined in header.

    #ifdef TRAPEZ
    Int_t   l_A1, l_A2;             // TRAPEZ filter counters
    UInt_t  l_gap = TRAPEZ_N_GAP;   // TRAPEZ gap size
    UInt_t  l_win = TRAPEZ_N_AVG;   // TRAPEZ window size
    #endif // TRAPEZ

    #ifdef MWD
    UInt_t  l_mwd_wind = MWD_WIND;
    UInt_t  l_mwd_avg  = MWD_AVG;
    Double_t    f_rev_tau = 1. / (Double_t) MWD_TAU;
    Int_t   l_tau_sum=0;
    //Double_t  f_tau_sum=0.;
    Double_t    f_sum=0.;
    Double_t    f_sum_e=0.;
    #ifdef USE_MBS_PARAM
        Double_t    f_mwd[MAX_TRACE_SIZE];
        Double_t    f_avg[MAX_TRACE_SIZE];
    #else
        Double_t    f_mwd[TRACE_SIZE];
        Double_t    f_avg[TRACE_SIZE];
    #endif // USE_MBS_PARAM
    #endif // MWD

    Int_t   l_fpga_filt_on_off; // 4bit for checking if FPGA filter mode is on
    //Int_t l_fpga_filt_mode;   // not used?
    Int_t   l_dat_trace;        // word for storing trace used in FGPA filter
    Int_t   l_dat_filt;         // word for storing FGPA filter
    Int_t   l_filt_sign;        // polarity of FPGA filter for correction


    //------------------------------------------------------------------------
    // MBS data stream readout. Here the function starts calling basic Go4 functions to read data words, etc.

    fInput = (TGo4MbsEvent* ) GetInputEvent();	// event is called
    if(fInput == 0)
    {
        cout << "AnlProc: no input event !"<< endl;
        return kFALSE;
    }

    //  JAM 12-12-2023 take general event number from mbs event header. Note that subsystem sequence may differ:
    fOutEvent->fSequenceNumber = fInput->GetCount();

    // MBS trigger counters are incremeneted, but also not used?
    l_trig_type_triva = fInput->GetTrigger();
    if (l_trig_type_triva == 1)
    {
        l_evt_ct_phys++;
    }
    l_evt_ct++;

    // MBS allows for multiple subevents. We don't use sub events, so machinery has been removed for brevity.
    // See TFeb3Full example or CsISiPHOS_FEBEX setup from N. Kurz for sub event machinery.
    TGo4MbsSubEvent* psubevt;

    fInput->ResetIterator();
    psubevt = fInput->NextSubEvent(); // only one subevent

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
    l_dat = *pl_tmp++;
    // sub system ID commented because timesorter means we have 3 IDs
    /*
    if(l_dat != SUB_SYSTEM_ID)
    {
        printf ("ERROR>> 1. data word is not sub-system id: %d \n");
        printf ("should be: 0x%x, but is: 0x%x\n", SUB_SYSTEM_ID, l_dat);
    }

    if(l_dat != SUB_SYSTEM_ID)
    {
        goto bad_event;
    }
    */
    l_dat = (*pl_tmp++) >> 16;
    if(l_dat != TS__ID_L16)
    {
        printf ("ERROR>> 2. data word does not contain 0-15 16bit identifier: \n");
        printf ("should be: 0x%x, but is: 0x%x\n", TS__ID_L16, l_dat);
    }
    l_dat = (*pl_tmp++) >> 16;
    if(l_dat != TS__ID_M16)
    {
        printf ("ERROR>> 3. data word does not contain 16-31 16bit identifier:  \n");
        printf ("should be: 0x%x, but is: 0x%x\n", TS__ID_M16, l_dat);
    }
    l_dat = (*pl_tmp++) >> 16;
    if(l_dat != TS__ID_H16)
    {
        printf ("ERROR>> 4. data word does not contain 32-47 16bit identifier:  \n");
        printf ("should be: 0x%x, but is: 0x%x\n", TS__ID_H16, l_dat);
    }
    l_dat = (*pl_tmp++) >> 16;
    if(l_dat != TS__ID_X16)
    {
        printf ("ERROR>> 5. data word does not contain 48-63 16bit identifier:  \n");
        printf ("should be: 0x%x, but is: 0x%x\n", TS__ID_X16, l_dat);
        printf ("WR_TIME_STAMP is activated, but timestamp is throwing errors. Perhaps WR_TIME_STAMP should be turned off?\n");
    }
    #endif // WR_TIME_STAMP


    //------------------------------------------------------------------------
    // extract analysis parameters from MBS data
    // ATTENTION:  these data is only present if WRITE_ANALYSIS_PARAM is enabled in corresponding f_user.c
    // WRITE_ANALYSIS_PARAM (in mbs) and USE_MBS_PARAM (in go4) must be used always together

    #ifdef USE_MBS_PARAM
    l_slaves = *pl_tmp++;
    l_trace  = *pl_tmp++;
    l_e_filt = *pl_tmp++;
    for(l_i=0; l_i<MAX_SFP; l_i++)
    {
        l_pola[l_i] = *pl_tmp++;
    }
    #endif // USE_MBS_PARAM

    //------------------------------------------------------------------------
    // for first event, make histograms
    if(l_first == 0)	// l_first defined as zero, so triggered if first event
    {
        l_first = 1;
        #ifdef USE_MBS_PARAM
        printf ("debug: 0x%x, 0x%x, 0x%x \n", l_slaves, l_trace, l_e_filt);
        fflush (stdout);
        #endif
        f_make_histo (0);	// calls make histograms function
    }

    //------------------------------------------------------------------------
    // reset arrays before processing new data

    for(l_i=0; l_i<MAX_SFP; l_i++)
    {
        if(l_sfp_slaves[l_i] != 0)
        {
            for(l_j=0; l_j<l_sfp_slaves[l_i]; l_j++)
            {
                for(l_k=0; l_k<N_CHA; l_k++)
                {
                    h_trace       [l_i][l_j][l_k]->Reset ("");
                    h_trace_blr   [l_i][l_j][l_k]->Reset ("");
                    h_trapez_fpga [l_i][l_j][l_k]->Reset ("");
                    l_ch_hitpat   [l_i][l_j][l_k] = 0;
                    l_ch_hitpat_tr[l_i][l_j][l_k] = 0;

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

    //------------------------------------------------------------------------
    // while loops reads words of subevent and processes information
    //------------------------------------------------------------------------
    while( (pl_tmp - pl_se_dat) < (l_dat_len_byte/4) )
    {
        l_dat = *pl_tmp++;	// data word must be padding word or channel header
        if( (l_dat & 0xfff00000) == 0xadd00000 )	// check if beginning of padding 4 byte words
        {
              l_dat = (l_dat & 0xff00) >> 8;
              pl_tmp += l_dat - 1;  // increment by pointer by nr. of padding  4byte words
        }
        else if( (l_dat & 0xff) == 0x34) //if not padding, check for channel header
        {
            // reads header to extract channel identifiers
            l_cha_head = l_dat;

            l_trig_type = (l_cha_head & 0xf00)      >>  8;
            l_sfp_id    = (l_cha_head & 0xf000)     >> 12;
            l_feb_id    = (l_cha_head & 0xff0000)   >> 16;
            l_cha_id    = (l_cha_head & 0xff000000) >> 24;

            // check SFP, FEBEX card, and channel IDs are within expected range
            if((l_sfp_id > (MAX_SFP-1)) || (l_sfp_id < 0))
            {
                printf ("ERROR>> SFP out of range - l_sfp_id: %d \n", l_sfp_id);  fflush (stdout);
                goto bad_event;
            }
            if((l_feb_id > (MAX_SLAVE-1)) || (l_feb_id < 0))
            {
                printf ("ERROR>> FEBEX out of range - l_feb_id: %d \n", l_feb_id); fflush (stdout);
                goto bad_event;
            }
            if((l_cha_id > (N_CHA-1)) || (l_cha_id < 0))
            {
                if(l_cha_id != 0xff)
                {
                    printf ("ERROR>> Channel out of range - l_cha_id: %d \n", l_cha_id); fflush (stdout);
                    goto bad_event;
                }
            }

            // check polarity of FEBEX card
            l_pol = (l_pola[l_sfp_id] >> l_feb_id) & 0x1;

            //------------------------------------------------------------------------
            // reads out special channel 0xff for E,t from FPGA
            if( (l_cha_head & 0xff000000) == 0xff000000)
            {
                // special channel data size
                l_cha_size = *pl_tmp++;

                l_spec_head = *pl_tmp++;
                if( (l_spec_head & 0xff000000) != 0xaf000000)
                {
                    printf ("ERROR>> E,t summary: wrong header is 0x%x, must be: 0x%x\n", (l_spec_head & 0xff000000)>>24, 0xaf);
                    goto bad_event;
                    //sleep (1);
                }
                ll_trg_time  = (Long64_t)*pl_tmp++;
                ll_time      = (Long64_t)*pl_tmp++;
                ll_trg_time += ((ll_time & 0xffffff) << 32);

                l_n_hit = (l_cha_size - 16) >> 3;
                //printf ("#hits: %d \n", l_n_hit);

                if(l_trig_type_triva == 1) // physics event
                {
                    h_hitpat[l_sfp_id][l_feb_id]->Fill (-1, 1);

                    for(l_i=0; l_i<l_n_hit; l_i++)  // stuff happens based on n_hits
                    {
                        l_dat = *pl_tmp++;          // hit time from fpga (+ other info)
                        l_hit_cha_id             = (l_dat & 0xf0000000) >> 28;
                        l_n_hit_in_cha           = (l_dat & 0xf000000)  >> 24;

                        l_more_than_1_hit_in_cha = (l_dat & 0x400000)   >> 22;
                        l_only_one_hit_in_cha    = (l_dat & 0x100000)   >> 20;

                        l_ch_hitpat[l_sfp_id][l_feb_id][l_hit_cha_id] = l_n_hit_in_cha;

                        if(l_more_than_1_hit_in_cha == 1)   // more than 1 hit, print alert but don't read data
                        {
                            l_more_1_hit_ct++;
                            printf ("%d More than 1 hit found for SFP: %d FEBEX: %d CHA: %d:: %d \n", l_more_1_hit_ct, l_sfp_id, l_feb_id, l_hit_cha_id, l_n_hit_in_cha);
                            fflush (stdout);
                        }

                        if((l_more_than_1_hit_in_cha == 1) && (l_only_one_hit_in_cha == 1)) // nonsense edge case
                        {
                            printf ("ERROR>> haeh? \n"); fflush (stdout);
                        }

                        if(l_only_one_hit_in_cha == 1)      // if just 1 hit, then fill histograms with FPGA info
                        {
                            l_hit_time_sign = (l_dat & 0x8000) >> 15;
                            l_hit_time = l_dat & 0x7ff;     // positive := AFTER  trigger, relative to trigger time
                            if(l_hit_time_sign == 1)        // negative sign
                            {
                                l_hit_time = l_hit_time * (-1); // negative := BEFORE trigger, relative to trigger time
                            }
                            //printf ("cha: %d, hit fpga time:  %d \n", l_hit_cha_id,  l_hit_time);
                            h_trgti_hitti[l_sfp_id][l_feb_id][l_hit_cha_id]->Fill (l_hit_time);
                            l_fpga_hitti[l_sfp_id][l_feb_id][l_hit_cha_id] = l_hit_time;
                        }
                        h_hitpat[l_sfp_id][l_feb_id]->Fill (l_hit_cha_id, l_n_hit_in_cha);	// fill hit pattern histogram

                        l_dat = *pl_tmp++;      // energy from fpga (+ other info)
                        l_hit_cha_id2  = (l_dat & 0xf0000000) >> 28;
                        if (l_hit_cha_id != l_hit_cha_id2)
                        {
                            printf ("ERROR>> hit channel ids differ in energy and time data word\n");
                            goto bad_event;
                        }
                        if((l_hit_cha_id > (N_CHA-1)) || (l_hit_cha_id < 0))
                        {
                            printf ("ERROR>> hit channel id: %d \n", l_hit_cha_id); fflush (stdout);
                            goto bad_event;
                        }

                        if(l_only_one_hit_in_cha == 1)	// again, only read out if single hit
                        {
                            l_fpga_energy_sign = (l_dat & 0x800000) >> 23;
                            l_fpga_energy      =  l_dat & 0x7fffff;      // positiv
                            if(l_fpga_energy_sign == 1)                // negative sign
                            {
                                l_fpga_energy = l_fpga_energy * (-1);     // negative
                            }
                            //printf ("cha: %d, hit fpga energy: %d \n", l_hit_cha_id2,  l_fpga_energy);
                            h_fpga_e[l_sfp_id][l_feb_id][l_hit_cha_id]->Fill (l_fpga_energy);	// fill temp energy hist for scope
                            l_fpga_e_found [l_sfp_id][l_feb_id][l_hit_cha_id] = 1;		// fill if FPGA energy found
                            l_fpga_e[l_sfp_id][l_feb_id][l_hit_cha_id] = l_fpga_energy; 	// fill value for use later
                        }
                    }
                }
                l_spec_trail = *pl_tmp++;   // checks for final trailing word to close subevent
                if( (l_spec_trail & 0xff000000) != 0xbf000000)
                {
                    printf ("ERROR>> E,t summary: wrong header is 0x%x, must be: 0x%x\n", (l_spec_trail & 0xff000000)>>24, 0xbf);
                    goto bad_event;
                    //sleep (1);
                }
            }
            //------------------------------------------------------------------------
            else // if not special, must be real channel
            {
                //printf ("real channel \n");
                // channel data size
                l_cha_size = *pl_tmp++;

                // trace header
                l_trace_head = *pl_tmp++;
                //printf ("trace header \n");
                if( ((l_trace_head & 0xff000000) >> 24) != 0xaa)
                {
                    printf ("ERROR>> trace header id is not 0xaa \n");
                    goto bad_event;
                }

                // FGPA filter is a mode that records the filter output of the FPGA, presumably set through f_user and
                // used for debugging the FPGA filter setup
                l_fpga_filt_on_off = (l_trace_head & 0x80000) >> 19;
                //l_fpga_filt_mode   = (l_trace_head & 0x40000) >> 18;
                //printf ("fpga filter on bit: %d, fpga filter mode: %d \n", l_fpga_filt_on_off, l_fpga_filt_mode);
                //fflush (stdout);
                //sleep (1);

                if(l_trig_type == 1) // physics event
                {
                    if(l_first_trace[l_sfp_id][l_feb_id] == 0)
                    {
                        l_first_trace[l_sfp_id][l_feb_id] = 1;
                        h_hitpat_tr[l_sfp_id][l_feb_id]->Fill (-1, 1);
                    }
                    h_hitpat_tr[l_sfp_id][l_feb_id]->Fill (l_cha_id, 1);
                    l_ch_hitpat_tr[l_sfp_id][l_feb_id][l_cha_id]++;

                    // now trace
                    l_trace_size = (l_cha_size/4) - 2;     // in longs/32bit

                    //correct the following comments!
                    //falls trace + filter trace: cuttoff at 2000 slices, since 4 times the amount of data!

                    //if (l_trace_size != (TRACE_SIZE>>1))
                    //{
                    //  printf ("ERROR>> l_trace_size: %d \n", l_trace_size); fflush (stdout);
                    //  goto bad_event;
                    //}

                    //------------------------------------------------------------------------
                    // read out traces and fill histograms
                    if(l_fpga_filt_on_off == 0) // only trace. no fpga filter trace data
                    {
                        for(l_l=0; l_l<l_trace_size; l_l++)   // loop over traces
                        {
                            // disentangle data. for some reason, the trace is split in half and then filled separately
                            l_dat_fir = *pl_tmp++;
                            l_dat_sec = l_dat_fir;

                            #ifdef USE_MBS_PARAM
                            l_adc_type = (l_trace_head & 0x800000) >> 23;
                            #else
                            l_adc_type = (l_sfp_adc_type[l_sfp_id] >> l_feb_id) & 0x1;
                            #endif // USE_MBS_PARAM

                            if(l_adc_type == 0) // 12 bit
                            {
                                l_dat_fir =  l_dat_fir        & 0xfff;
                                l_dat_sec = (l_dat_sec >> 16) & 0xfff;
                            }

                            if(l_adc_type == 1)  // 14 bit
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

                        l_trace_size = l_trace_size * 2; // trace size needs to be doubled as we worked with halves
                    }

                    if(l_fpga_filt_on_off == 1) // trace AND fpga filter data
                    {
                        for(l_l=0; l_l<(l_trace_size>>1); l_l++)   // loop over traces
                        {
                            // disentangle data
                            l_dat_trace = *pl_tmp++;
                            l_dat_filt  = *pl_tmp++;
                            l_filt_sign  =  (l_dat_filt & 0x800000) >> 23;

                            #ifdef USE_MBS_PARAM
                            l_adc_type = (l_trace_head & 0x800000) >> 23;
                            #else
                            l_adc_type = (l_sfp_adc_type[l_sfp_id] >> l_feb_id) & 0x1;
                            #endif // USE_MBS_PARAM

                            if(l_adc_type == 0) // 12 bit
                            {
                                l_dat_trace = l_dat_trace  & 0xfff;
                            }

                            if(l_adc_type == 1)  // 14 bit
                            {
                                l_dat_trace = l_dat_trace  & 0x3fff;
                            }

                            l_dat_filt  = l_dat_filt   & 0x7fffff;
                            if(l_filt_sign == 1) {l_dat_filt = l_dat_filt * -1;}

                            // both trace and FGPA filter output are recorded this time
                            h_trace      [l_sfp_id][l_feb_id][l_cha_id]->SetBinContent (l_l+1, l_dat_trace);
                            h_trapez_fpga[l_sfp_id][l_feb_id][l_cha_id]->SetBinContent (l_l+1, l_dat_filt);

                            l_tr[l_l] = l_dat_trace;
                        }
                        l_trace_size = l_trace_size >> 1;
                    }

                    //------------------------------------------------------------------------
                    // find base line value of trace and correct it to baseline 0
                    f_bls_val = 0.;
                    for(l_l=l_bls_start; l_l<l_bls_stop; l_l++)
                    {
                        f_bls_val += (Double_t)l_tr[l_l];
                    }
                    f_bls_val = f_bls_val / (Double_t)(l_bls_stop - l_bls_start);
                    for(l_l=0; l_l<l_trace_size; l_l++)   // create baseline restored trace
                    {
                        f_tr_blr[l_l] =  (Double_t)l_tr[l_l] - f_bls_val;
                        h_trace_blr[l_sfp_id][l_feb_id][l_cha_id]->Fill (l_l, f_tr_blr[l_l] * ADC_RES);
                        //h_trace_blr[l_sfp_id][l_feb_id][l_cha_id]->SetBinContent (l_l+1, f_tr_blr[l_l]);
                    }

                    //------------------------------------------------------------------------
                    // run TRAPEZ filter on trace and fill histograms
                    #ifdef TRAPEZ
                    l_A1 = 0;
                    l_A2 = 0;
                    for(l_l=(l_gap+l_win); l_l<l_trace_size; l_l++)   // loop over traces
                    {
                        if(l_l < (l_gap + (2*l_win)))	// if pos before full window on screen, don't fill hist yet
                        {
                            //l_A1 += l_tr[l_l] - l_tr[l_l-l_win];
                            l_A1 += l_tr[l_l];
                            l_A2 += l_tr[l_l-l_win-l_gap];
                            //printf ("index: %d, A1: %d, A2 %d, A1-A2: %d, norm %d  \n", l_l, l_A1, l_A2, l_A1-l_A2, l_win);
                            //printf ("(A1-A2)/norm: %d, %f \n", (l_A1 - l_A2) / l_win, (Real_t)(l_A1 - l_A2) / (Real_t) l_win);
                        }
                        else    // now full window on screen, fill evaluate TRAPEZ and fill hist
                        {
                            l_A1 += l_tr[l_l]  - l_tr[l_l-l_win];
                            l_A2 += l_tr[l_l-l_win-l_gap] - l_tr[l_l-(2*l_win)-l_gap];

                            if(l_l < (l_trace_size -(2*l_win)-l_gap))
                            {
                                //h_trapez_f[l_sfp_id][l_feb_id][l_cha_id]->Fill (l_l, (Real_t)(l_A1 - l_A2) / (Real_t) l_win);
                                h_trapez_f[l_sfp_id][l_feb_id][l_cha_id]->Fill (l_l, (Real_t)(l_A1 - l_A2));
                            }
                        }
                    }
                    //printf ("next trace \n\n\n\n");
                    //sleep (1);
                    #endif // TRAPEZ

                    // find peaks and valleys in trace and fill histogram
                    h_peak  [l_sfp_id][l_feb_id][l_cha_id]->Fill (h_trace[l_sfp_id][l_feb_id][l_cha_id]->GetMaximum ());
                    h_valley[l_sfp_id][l_feb_id][l_cha_id]->Fill (h_trace[l_sfp_id][l_feb_id][l_cha_id]->GetMinimum ());

                    #ifdef TRAPEZ       // determine energy and fill histograms from TRAPEZ filter
                    if(l_pol == 1) // negative signals
                    {
                        h_trapez_e[l_sfp_id][l_feb_id][l_cha_id]->Fill (h_trapez_f[l_sfp_id][l_feb_id][l_cha_id]->GetMinimum ());
                        l_trapez_e_found [l_sfp_id][l_feb_id][l_cha_id] = 1;
                        l_trapez_e[l_sfp_id][l_feb_id][l_cha_id] = h_trapez_f[l_sfp_id][l_feb_id][l_cha_id]->GetMinimum ();
                    }
                    if(l_pol == 0) // positive signals
                    {
                        h_trapez_e[l_sfp_id][l_feb_id][l_cha_id]->Fill (h_trapez_f[l_sfp_id][l_feb_id][l_cha_id]->GetMaximum ());
                        l_trapez_e_found [l_sfp_id][l_feb_id][l_cha_id] = 1;
                        l_trapez_e[l_sfp_id][l_feb_id][l_cha_id] = h_trapez_f[l_sfp_id][l_feb_id][l_cha_id]->GetMaximum ();
                    }
                    #endif // TRAPEZ

                    //------------------------------------------------------------------------
                    // run MWD filter on trace and fill histograms
                    #ifdef MWD
                    l_tau_sum = 0;
                    for(l_l=1; l_l<l_mwd_wind; l_l++)
                    {
                        l_tau_sum += f_tr_blr[l_mwd_wind - l_l];
                    }

                    f_sum = 0.;
                    for(l_l=l_mwd_wind; l_l<l_trace_size; l_l++)   // mwd
                    {
                        f_sum =  f_tr_blr[l_l] - f_tr_blr[l_l - l_mwd_wind];
                        f_sum += (f_rev_tau * (Double_t)l_tau_sum);
                        f_mwd[l_l] = f_sum;

                        h_mwd_f[l_sfp_id][l_feb_id][l_cha_id]->Fill (l_l, f_sum);

                        // correct f_tau_sum for next index
                        l_tau_sum += f_tr_blr[l_l] - f_tr_blr[l_l - l_mwd_wind];
                    }

                    f_sum_e = 0.;
                    for(l_l=l_mwd_wind; l_l<l_trace_size; l_l++) // averaging
                    {
                        f_sum_e += f_mwd[l_l] - f_mwd[l_l - l_mwd_avg];
                        if(l_l>(l_mwd_wind+l_mwd_avg))
                        {
                            f_avg[l_l] = f_sum_e/(Double_t)l_mwd_avg;
                            h_mwd_a[l_sfp_id][l_feb_id][l_cha_id]->Fill (l_l, f_avg[l_l]);
                        }
                    }

                    // fill energy histograms for MWD from peaks/valleys
                    if(l_pol == 1) // negative signals
                    {
                        h_mwd_e[l_sfp_id][l_feb_id][l_cha_id]->Fill(h_mwd_a[l_sfp_id][l_feb_id][l_cha_id]->GetMinimum ());
                    }
                    if(l_pol == 0) // positive signals
                    {
                        h_mwd_e[l_sfp_id][l_feb_id][l_cha_id]->Fill(h_mwd_a[l_sfp_id][l_feb_id][l_cha_id]->GetMaximum ());
                    }
                    #endif // MWD
                }

                // jump over trace
                //pl_tmp += (l_cha_size >> 2) - 2;

                // check trace trailer to exit subevent correctly
                //printf ("trace trailer \n");
                l_trace_trail = *pl_tmp++;
                if( ((l_trace_trail & 0xff000000) >> 24) != 0xbb)
                {
                    printf ("ERROR>> trace trailer id is not 0xbb, ");
                    printf ("SFP: %d, FEB: %d, CHA: %d \n", l_sfp_id, l_feb_id, l_cha_id);
                    goto bad_event;
                }
            }
        }
        else    // if data word not padding or channel header, throw error
        {
            printf("ERROR>> data word neither channel header nor padding word \n");
        }
    }
    //------------------------------------------------------------------------
    // end of subevent data word while loop. all traces and histograms now filled for this event.
    //------------------------------------------------------------------------


    //------------------------------------------------------------------------
    // array to hist filling -- values stored in float arrays are filled into 1D hists for each channel
    //------------------------------------------------------------------------
    for(l_i=0; l_i<MAX_SFP; l_i++)
    {
        if(l_sfp_slaves[l_i] != 0)
        {
            for(l_j=0; l_j<l_sfp_slaves[l_i]; l_j++)
            {
                for(l_k=0; l_k<N_CHA; l_k++)
                {
                    h_ch_hitpat   [l_i][l_j][l_k]->Fill (l_ch_hitpat   [l_i][l_j][l_k]);
                    h_ch_hitpat_tr[l_i][l_j][l_k]->Fill (l_ch_hitpat_tr[l_i][l_j][l_k]);

                    if( (l_trapez_e_found[l_i][l_j][l_k] == 1) && (l_fpga_e_found[l_i][l_j][l_k] == 1))
                    {
                        l_value=0;
                        if(l_trapez_e[l_i][l_j][l_k])
                        {
                            l_value=(Double_t)l_fpga_e[l_i][l_j][l_k] / (Double_t)l_trapez_e[l_i][l_j][l_k];
                        }
                        h_corr_e_fpga_trapez[l_i][l_j][l_k]->Fill(l_value);
                        //printf ("Energy FPGA / Energy Trace: %1.10f \n",l_value);
                    }
                }
            }
        }
    }


    //------------------------------------------------------------------------
    // filling output events
    //------------------------------------------------------------------------
    for(l_i=0; l_i<MAX_SFP; ++l_i)
    {
        if(l_sfp_slaves[l_i] != 0)
        {
            for(l_j=0; l_j<l_sfp_slaves[l_i]; l_j++)
            {
                //get board ID at array position from Go4 parameter fBoardID
                UInt_t brdID = fPar->fBoardID[l_i][l_j];
                //printf("board ID fetched for sfp %i, slv %i was %d\n", l_i, l_j, brdID);

                TPLEIADESFebBoard* theBoard = fOutEvent->GetBoard(brdID);

                UInt_t nChan = theBoard->getNElements();
                if (nChan != N_CHA)
                {
                    GO4_SKIP_EVENT_MESSAGE("Config error: board NElements not equal to N_CHA")
                    return kFALSE;
                }
                for(l_k=0; l_k<nChan; l_k++)
                {
                    TPLEIADESFebChannel* theChannel = theBoard->GetChannel(l_k);

                    theChannel->fFPGAEnergy = l_fpga_e[l_i][l_j][l_k];
                    theChannel->fFGPAHitTime = l_fpga_hitti[l_i][l_j][l_k];

                    #ifdef TPLEIADES_FILL_TRACES
                    for(int bin=1; bin<h_trace[l_i][l_j][l_k]->GetNbinsX(); ++bin)
                    {
                        l_value=h_trace[l_i][l_j][l_k]->GetBinContent(bin);
                        theChannel->fTrace.push_back(l_value);
                    }

                    for(int bin=1; bin<h_trace_blr[l_i][l_j][l_k]->GetNbinsX(); ++bin)
                    {
                        l_value=h_trace_blr[l_i][l_j][l_k]->GetBinContent(bin);
                        theChannel->fTraceBLR.push_back(l_value);
                    }

                    for(int bin=1; bin<h_trapez_f[l_i][l_j][l_k]->GetNbinsX(); ++bin)
                    {
                        l_value=h_trapez_f[l_i][l_j][l_k]->GetBinContent(bin);
                        theChannel->fTraceTRAPEZ.push_back(l_value);
                    }

                    theChannel->fTrapezEnergy = l_trapez_e[l_i][l_j][l_k];

                    for(int bin=1; bin<h_trapez_fpga[l_i][l_j][l_k]->GetNbinsX(); ++bin)
                    {
                        l_value=h_trapez_fpga[l_i][l_j][l_k]->GetBinContent(bin);
                        theChannel->fFPGATRAPEZ.push_back(l_value);
                    }
                    #endif // TPLEIADES_FILL_TRACES
                }
            }
        }
    }
    fOutEvent->SetValid(kTRUE);

    bad_event:

    //------------------------------------------------------------------------
    // JAM 12-Dec-2023: Added slow motion (eventwise step mode) as example how to use parameter container flag
    //------------------------------------------------------------------------
    if(fPar->fSlowMotion)
    {
        Int_t evnum = fInput->GetCount();
        GO4_STOP_ANALYSIS_MESSAGE("Stopped for slow motion mode after MBS event count %d. Click green arrow for next event. please.", evnum);
    }



    return kTRUE;
}

//------------------------------------------------------------------------
// f_make_histo assembles the histograms used in BuilEvent()

void TPLEIADESRawProc:: f_make_histo (Int_t l_mode)
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
    printf ("f_make_histo: # of sfp slaves: 3:%d, 2:%d, 1: %d, 0: %d \n", l_sfp_slaves[3], l_sfp_slaves[2], l_sfp_slaves[1], l_sfp_slaves[0]);
    fflush (stdout);
    #else
    l_tra_size   = TRACE_SIZE;
    l_trap_n_avg = TRAPEZ_N_AVG;
    #endif // USE_MBS_PARAM

    if(l_first2 == 0)
    {
        l_first2 = 1;
    }

    for(l_i=0; l_i<MAX_SFP; l_i++)
    {
        if(l_sfp_slaves[l_i] != 0)
        {
            for(l_j=0; l_j<l_sfp_slaves[l_i]; l_j++)
            {

                for(l_k=0; l_k<N_CHA; l_k++)
                {
                    sprintf(chis,"Traces/TRACE  SFP: %2d FEBEX: %2d CHAN: %2d", l_i, l_j, l_k);
                    sprintf(chead,"Trace");
                    h_trace[l_i][l_j][l_k] = MakeTH1('I', chis,chead,l_tra_size,0,l_tra_size);
                }

                for(l_k=0; l_k<N_CHA; l_k++)
                {
                    sprintf(chis,"Traces BLR/TRACE, base line restored (ordinate in mV) SFP: %2d FEBEX: %2d CHAN: %2d", l_i, l_j, l_k);
                    sprintf(chead,"Trace, base line restored");
                    h_trace_blr[l_i][l_j][l_k] = MakeTH1('F', chis,chead,l_tra_size,0,l_tra_size);
                }

                #ifdef TRAPEZ
                for(l_k=0; l_k<N_CHA; l_k++)
                {
                    sprintf(chis,"TRAPEZ/TRAPEZ Filter  SFP: %2d FEBEX: %2d CHAN: %2d", l_i, l_j, l_k);
                    sprintf(chead,"TRAPEZ Filter");
                    h_trapez_f[l_i][l_j][l_k] = MakeTH1('F', chis,chead,l_tra_size,0,l_tra_size);
                }

                for(l_k=0; l_k<N_CHA; l_k++)
                {
                    sprintf(chis,"TRAPEZ Energy/TRAPEZ Energy   SFP: %2d FEBEX: %2d CHAN: %2d", l_i, l_j, l_k);
                    sprintf(chead,"Energy");
                    //h_trapez_e[l_i][l_j][l_k] = MakeTH1('F', chis,chead,0x8000,(-1)*0x1000*TRAPEZ_N_AVG,0x1000*TRAPEZ_N_AVG);
                    //h_trapez_e[l_i][l_j][l_k] = MakeTH1('F', chis,chead,0x20000,-2000000,2000000);
                    h_trapez_e[l_i][l_j][l_k] = MakeTH1('F', chis,chead,0x20000,-200000,200000);
                }
                #endif // TRAPEZ

                #ifdef MWD
                for(l_k=0; l_k<N_CHA; l_k++)
                {
                    sprintf(chis,"MWD Filter/MWD Filter  SFP: %2d FEBEX: %2d CHAN: %2d", l_i, l_j, l_k);
                    sprintf(chead,"MWD Filter");
                    h_mwd_f[l_i][l_j][l_k] = MakeTH1('F', chis,chead,l_tra_size,0,l_tra_size);
                }

                for(l_k=0; l_k<N_CHA; l_k++)
                {
                    sprintf(chis,"MWD Average/MWD Average  SFP: %2d FEBEX: %2d CHAN: %2d", l_i, l_j, l_k);
                    sprintf(chead,"MWD Average");
                    h_mwd_a[l_i][l_j][l_k] = MakeTH1('F', chis,chead,l_tra_size,0,l_tra_size);
                }

                for(l_k=0; l_k<N_CHA; l_k++)
                {
                    sprintf(chis,"MWD Energy/MWD Energy   SFP: %2d FEBEX: %2d CHAN: %2d", l_i, l_j, l_k);
                    sprintf(chead,"Energy");
                    h_mwd_e[l_i][l_j][l_k] = MakeTH1('F', chis,chead,0x20000,-0x1000,0x1000);
                }
                #endif // MWD

                for(l_k=0; l_k<N_CHA; l_k++)
                {
                    sprintf(chis,"FPGA/FPGA Trapez   SFP: %2d FEBEX: %2d CHAN: %2d", l_i, l_j, l_k);
                    sprintf(chead,"FPGA Trapez");
                    h_trapez_fpga[l_i][l_j][l_k] = MakeTH1('F', chis,chead,l_tra_size,0,l_tra_size);
                }

                for(l_k=0; l_k<N_CHA; l_k++)
                {
                    sprintf(chis,"FPGA/FPGA Energy(hitlist)  SFP: %2d FEBEX: %2d CHAN: %2d", l_i, l_j, l_k);
                    sprintf(chead,"FPGA Energy");
                    //h_fpga_e[l_i][l_j][l_k] = MakeTH1('F', chis,chead,0x8000,(-1)*0x1000*TRAPEZ_N_AVG,0x1000*TRAPEZ_N_AVG);
                    h_fpga_e[l_i][l_j][l_k] = MakeTH1('F', chis,chead,0x10000,-2000000,2000000);
                }

                for(l_k=0; l_k<N_CHA; l_k++)
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

                for(l_k=0; l_k<N_CHA; l_k++)
                {
                    sprintf(chis,"Valleys/VALLEY   SFP: %2d FEBEX: %2d CHAN: %2d", l_i, l_j, l_k);
                    sprintf(chead,"Valley");
                    h_valley[l_i][l_j][l_k] = MakeTH1('I', chis,chead,0x1000,0,0x4000);
                }

                for(l_k=0; l_k<N_CHA; l_k++)
                {
                    sprintf(chis,"Timediff/Trigger time - Hit time   SFP: %2d FEBEX: %2d CHAN: %2d", l_i, l_j, l_k);
                    sprintf(chead,"TRIG-HIT");
                    h_trgti_hitti[l_i][l_j][l_k] = MakeTH1('I', chis,chead,2000,-1000,1000);
                }

                for(l_k=0; l_k<N_CHA; l_k++)
                {
                    sprintf(chis,"Hitpat_Cha_List/Channel hit pattern per event (list)  SFP: %2d FEBEX: %2d CHAN: %2d", l_i, l_j, l_k);
                    sprintf(chead,"HITPAT_Cha_List");
                    h_ch_hitpat[l_i][l_j][l_k] = MakeTH1('I', chis,chead,11,-1,10);
                }

                sprintf(chis,"Hitpat_Feb_List/Hit Pattern (list)  SFP: %2d FEBEX: %2d", l_i, l_j);
                sprintf(chead,"Hitpat_List");
                h_hitpat[l_i][l_j] = MakeTH1('I', chis,chead,20,-2,18);

                for(l_k=0; l_k<N_CHA; l_k++)
                {
                    sprintf(chis,"Hitpat_Cha_Trace/Channel hit pattern per event (trace)  SFP: %2d FEBEX: %2d CHAN: %2d", l_i, l_j, l_k);
                    sprintf(chead,"HITPAT_Cha_Trace");
                    h_ch_hitpat_tr[l_i][l_j][l_k] = MakeTH1('I', chis,chead,11,-1,10);
                }

                sprintf(chis,"Hitpat_Feb_Trace/Hit Pattern (trace)  SFP: %2d FEBEX: %2d", l_i, l_j);
                sprintf(chead,"Hitpat_Trace");
                h_hitpat_tr[l_i][l_j] = MakeTH1('I', chis,chead,20,-2,18);

                for(l_k=0; l_k<N_CHA; l_k++)
                {
                    sprintf(chis,"Hitpat_Cha_Diff/Channel hit pattern per event (diff)  SFP: %2d FEBEX: %2d CHAN: %2d", l_i, l_j, l_k);
                    sprintf(chead,"HITPAT_Cha_Diff");
                    h_ch_hitpat_di[l_i][l_j][l_k] = MakeTH1('I', chis,chead,11,-1,10);
                }

                sprintf(chis,"Hitpat_Feb_Diff/Hit Pattern (diff)  SFP: %2d FEBEX: %2d", l_i, l_j);
                sprintf(chead,"Hitpat_Trace");
                h_hitpat_di[l_i][l_j] = MakeTH1('I', chis,chead,20,-2,18);

                for(l_k=0; l_k<N_CHA; l_k++)
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
