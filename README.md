```
----------------------------------------------------------------------
       The GSI Online Offline Object Oriented (Go4) Project
         Experiment Data Processing at CSEE department, GSI
-----------------------------------------------------------------------
 Copyright (C) 2000- GSI Helmholtzzentrum f. Schwerionenforschung GmbH
                     Planckstr. 1, 64291 Darmstadt, Germany
 Contact:            http://go4.gsi.de
-----------------------------------------------------------------------
 This software can be used under the license agreements as stated
 in Go4License.txt file which is part of the distribution.
-----------------------------------------------------------------------

```


# Febex full readout with output event for PLEIADES

V 0.1 13-Dec-2023
Joern Adamczewski-Musch (JAM), EEL, GSI Darmstadt (j.adamczewski@gsi.de)

---------------------------------------------

## BUILDING and STARTUP:

To build it just enable go4 environment by calling 

> . $GO4SYS/go4login

(bash dot script; with GO4SYS the go4 installation directory)

**Note for GSI lxpool users: please use**
>. /cvmfs/eel.gsi.de/bin/go4login

Then call
> make 

To start Go4 GUI just call
> go4

and launch analysis in local folder with libGo4UserAnalysis.so.
Do not forget to create a Go4 hotstart script after this!

As usual, 
> go4analysis 

will offer a batch mode (please see go4analysis -h)

This code has been compiled and tested with  Go4 v6.1.2 and root 6.22/08. 


## USER ANALYSIS:

The main analysis structure is set up in constructor of class TPLEIADESAnalysis.
Here different subsequent "analysis steps" can be defined by means of a TGo4StepFactory. This
stepfactory will tell the framework which processor class and event class shall be created from the user
library with functions DefEventProcessor() and DefOutputEvent(). The analysis step is then created using
this factory and registered by calling AddAnalysisStep().
Currently there is only one analysis step provided for the raw unpacker, but others may be added later.

Please note that all initial definitions of input/output file etc specified here can still be redefined at runtime
by go4analysis command line argumens, or by the analysis configuration GUI. However,
analysis steps and their classes must be defined here at compile time.




## FIRST ANALYSIS STEP:


### Event processor class TPLEIADESProc:
The first Go4 Analysis step  with class TPLEIADESRawProc does unpacking of MBS data from Febex, fills histograms and
copies data to the output event structure of class TPLEIADESRawEvent. The main function which is executed for each input event is
BuildEvent(). 


### Event structure class TPLEIADESRawEvent:
The output event object will keep the relevant data of one MBS input event, 

The MBS event number  is stored as 
>fSequenceNumber.

For each channel of the frontend slave (indices for SFP, SLAVE, CHA) the raw traces are kept in 
>std::vector<Double_t> , i.e. fTrace, fTraceBLR, and fTrapezFPGA.

**NOTE: Filling of the raw traces can be suppressed for performance reason by commenting this definition in TPLEIADESRawEvent.h:**
> \#define TPLEIADES_FILL_TRACES 1

and calling "make clean;make;" for recompiling.


Additionally, there is the corrected energy from the feature extraction 
>fE_FPGA_Trapez

This event structure may be saved as a ROOT tree, or can be used
for further analysis in additional analysis steps, e.g. to map the electronic channels into
displays of detector components (to be developed).


###Configuration and control with TPLEIADESRawParam:
The class TPLEIADESRawParam is a TGo4Parameter that may defines the setup of the unpacker 
and may offer additional run control flags.

The following flag is an example of interactive control:

>Bool_t fSlowMotion; 
<- if true than only process one MBS event and stop. 

This can be helpful to debug data by printout in analysis terminal. When pressing the "start" button in Go4 gui,
the next event is processed. 

Like all Go4 parameters, one can inspect (and change) the current contents of TPLEIADESRawParam in the GUI
with the parameter editor.


### Configuration script set_PLEIADESRawParam.C:

By default, the parameter values as specified in the source code of TPLEIADESRawParam.cxx are used, so one could change the setup
here and recompile the user analysis.
By means of the interactive parameter editor and the autosave file, one could override this though (however, old
histgrams of deactivated modules will still be kept in the autosave file then)

The script set_PLEIADESRawParam.C offers the possibility to change the setup without recompiling or using the parameter editor. 
If existing in the working directory, it will override both previous setups.
To use it, please edit the values assigned to the parameter members in the script 
Then just "submit and start" again the analysis from gui, or rerun go4analysis batch job.


### Checking the tree contents with ROOT macro

To verify that event contents were written to the output tree, subfolder macros contains an example ROOT macro 
>MyTreeAnalysis.C

This will read first TTree of Go4 produced event store file and display the contents of fTrace vector for the 16 channels
of one single frontend board (indices sfp=1, slave=1).

To use this macro:

>root [0] .L MyTreeAnalysis.C++

>...

>root [1] MyTreeAnalysis ana(0,"/u/adamczew/go4work/go4-app/PLEIADES/Plejades.root")

(please adjust example filename in second constructor argument)
>...

>root [2] ana.Loop();

The contents of the febex traces are displayed for each event in a divided TCanvas.

## FURTHER INFORMATION
All information about go4 are available at
>https://go4.gsi.de


Please have a look at the Go4 user manual available at
>http://web-docs.gsi.de/~go4/go4V06/manuals/Go4introV6.pdf

(or in the Go4 GUI Help menu ;-))

##### JAM 15-12-2023

