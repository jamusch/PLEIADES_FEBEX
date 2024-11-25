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


# TPLEIADES Go4 Analysis: full FEBEX readout and unpacking
V 1.0 - 24 November 2024: 
Guy Leckenby, TRIUMF Vancouver (gleckenby@triumf.ca) and </br>
Joern Adamczewski-Musch (JAM), EEL, GSI Darmstadt (j.adamczewski@gsi.de)

This software is a Go4 analysis library for the PLEIADES pocket detectors of the ILIMA collaboration. With the library, Go4 can be run as an online analysis tool to analyse the MBS data stream, or as an offline unpacker to prepare ROOT files for further analysis. We decided to use Go4 for our PLEIADES analysis to take advantage of the native interface between Go4 and MBS, and for the excellent support from the GSI EEL department.

## Analysis structure
Go4 user analyses feature a modular, step-by-step framework for maximum flexibility in the analysis. In particular, each step can run on any input, so the analysis can be run sequentially or in parts, as the user desires. The main analysis structure is set up in the constructor of class `TPLEIADESAnalysis`. Here the different analysis steps are defined by means of a `TGo4StepFactory`. More details on Go4 analysis steps can be found in the [Go4 manual](https://web-docs.gsi.de/%7Ego4/go4V06/manuals/Go4introV6.pdf).

The StepFactory takes a processor class that codes the analysis to be done and an event class that creates the structure of the physics event to be read. Note here that following the MBS structure, everything is processed on an event-by-event basis. The StepFactory tells the framework which processor class and event class shall be created from the user library with functions `DefEventProcessor()` and `DefOutputEvent()`. The analysis step is then created using this factory and registered by calling `AddAnalysisStep()`.

We have taken advantage of the modular structure by splitting the TPLEIADES analysis into three analysis steps:
1. `TPLEIADESRaw` - unpacks the raw data from the MBS steam or .lmd file,
2. `TPLEIADESDet` - formats the raw channel mappings into detector objects, and
3. `TPLEIADESPhys` - applies "physics" routines like signal filtering or pulse shape analysis.

The intention is that the `Raw` and `Det` step should only need to be run once for a given data set, and then some early data manipulation can be done with step `Phys`. For the June 2022 commissioning run, the compression from raw .lmd files to the output Phys ROOT file was a reduction factor of >2000, mostly from including trace analysis in the Phys step.

Note that all initial definitions of input/output file etc specified in the `TPLEIADESAnalysis` file can still be redefined at runtime by the `go4analysis` command line arguments, or by the analysis configuration GUI. However, the analysis steps and their classes must be defined here at compile time.


## First Step: `TPLEIADESRaw`
`TPLEIADESRaw` is the first Go4 analysis step and does the unpacking of the MBS data from the FEBEX ADCs and copies data to the output event structure of class `TPLEIADESRawEvent`. Currently `TPLEIADESRawProc` also creates some raw histograms as well, but this may be phased out in the next version. 

### Processor class `TPLEIADESRawProc`
`TPLEIADESRawProc` is primarily composed of Nikolaus Kurz's unpacker that he wrote whilst building the FEBEX DAQ. It unpacks the MBS using the structure created by the `f_user.C` file that sets up how the FEBEX ADCs record data. Unless you are very brave, I would not touch the core of Nik's code. The main function in the `TPLEIADESRawProc` class that is executed for each input event is BuildEvent(). The unpacking is done to various arrays, and then those arrays are copied out to the `TPLEIADESRawEvent` class for output.

### Event structure class `TPLEIADESRawEvent`
The output event object will keep the relevant data of one MBS input event. The MBS event number is stored class member `fSequenceNumber`.

The `TPLEIADESRawEvent` class emulates the FEBEX DAQ structure by featuring a series of `TPLEIADESFebBoard` objects (corresponding to FEBEX cards), each with 16 `TPLEIADESFebChannel` objects (corresponding to the individual channels). A `TPLEIADESFebBoard` will be initialised by defining the FEBEX board ID in `set_PLEIADESParam.C`. This uses `TPLEIADESParam` to tell `TPLEIADESRawEvent` to create a `TPLEIADESFebBoard` instance. The param member `fBoardID` has dimensions `[MAX_SFP][MAX_SLAVE]` set in `TPLEIADESRawEvent.h`, where SFP refers to the FEBEX crate and SLAVE refers to the individual FEBEX cards. Be sure to check that these numbers are calibrated for your DAQ setup.

The `TPLEIADESFebChannel`s store the data: e.g. `fRPolarity` (raw polarity), `fRTraceBLR` (raw trace baseline restored), etc. Traces are stored in `std::vector`s. The filling of the raw traces can be suppressed for performance reason by commenting this definition in `TPLEIADESRawEvent.h`:
> \#define TPLEIADES_FILL_TRACES 1

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

and calling "make clean;make;" for recompiling.

## FURTHER INFORMATION
All information about go4 are available at
>https://go4.gsi.de


Please have a look at the Go4 user manual available at
>http://web-docs.gsi.de/~go4/go4V06/manuals/Go4introV6.pdf

(or in the Go4 GUI Help menu ;-))

##### JAM 15-12-2023

