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


## Configuration with `TPLEIADESParam`
The class `TPLEIADESParam` is a `TGo4Parameter` that can be used to control the analysis without recompiling. We have also used it as a configuration script with which to configure the DAQ setup without needing to touch the inner workings of the code. **The DAQ setup needs to be correctly configured in `set_PLEIADESParam.C` before the analysis will work.** An example we have already seen is `fBoardID`, which is a member of `TPLEIADESParam` that stores the active FEBEX board positions.

An example of interactive control is the flag `fSlowMotion`, which, if true, than only one MBS event is processed before Go4 is stopped. This can be helpful to debug data by printout in analysis terminal. When pressing the "start" button in Go4 gu, the next event is processed. 

Like all Go4 parameters, one can inspect (and change) the current contents of `TPLEIADESParam` in the GUI
with the parameter editor.


## Second step: `TPLEIADESDet`
The second step is essentially an advanced channel mapping that takes the raw FEBEX like structure from step 1 and formats it into detector objects that emulate the detector. In most cases, it is intended that step 1 and 2 get run together and the 'raw' data (i.e. before trace analysis) is stored as the output of step 2. These steps were separated to help debugging and provide extra flexibility, but in 99% of cases you will want to implement the detector map.

The `TPLEIADESDetEvent` class has `TPLEIADESDetector` objects, which represent the individual detectors inside the PLEIADES telescope, and `TPLEIADESDetChan` objects, which are the various channels of those detectors. In the default setup, the output should feature objects for 6 Si pad detectors, 1 DSSD, and 1 crystal stopper.

The detector map is setup in `set_PLEIADESParam.C`. The following information must be given for each detector:
- the detector name: e.g. "slot1_MSPad_17A"
- the detector type: one of "SiPad", "DSSD", or "Crystal"
- the position of the first p-side: e.g. "0x100"
- the position of the n-side: e.g. "0x140"

The code used for the channel map is a 12 bit word: bit 9/8 are for SFP, bit 7--4 are for Board slot, bit 3--0 are for channel position.

## Third step: `TPLEIADESPhys`
The third step is where trace analysis and any other complex event construction should be done. The idea is that after this step, the traces will be discarded so that light, minimalist data files can be used for the real physics analysis. The `TPLEIADESPhysEvent` object features `TPLEIADESDetPhysics` objects that represent the detectors but only have a single p/n side energy values and a position for simplicity. The goal of this step is then to construct those single values.

In his code, Nik Kurz implements two filters to extract the pulse height from the traces: the bi-box filter (called `TRAPEZ` in his code) and a moving window deconvolution (called `MWD` in the code). These are basic energy filters and are implemented in step 1. Ideally they'd probably be done here instead, but moving that code is too much effort. Currently we use the `TRAPEZ` filter, but we should probably move to the MWD.

In the present state of the code (as finished for my thesis), `TPLEIADESPhysProc` features the following functions:
- `pStripSelect`: a function that searches for the active p strip and returns the hit location(s).
- `PulseShapeIntegration`: a function that attempts to integrate the backside of the pulse, currently through summing the bins.
- `stdSinSideEnergy`: the "standard" energy function for Si n side. Currently directly copies the FPGA energy.
- `stdDSSDEnergy`: a function that adds the DSSD channels to get p and n side energies. Currently also uses FPGA energy.
- `stdCrystalEnergy`: currently blank
- `FillClipStatsHists`: a function that created histograms on the clipping statistics of traces.
- `FillTOThreshHists`: a function that implemented 'time over threshold' for clipped traces.
- `ExpFitPHRecon`: a function that attempted to implement an exponential fit pulse height reconstruction.
- `ExpIntegPHRecon`: a function that implements an integration of the exponential tail of the trace.

I should probably clean some of these up for a future release...

## Building analysis and startup
To build it just enable the Go4 environment by calling 

> . $GO4SYS/go4login

(bash dot script; with GO4SYS the go4 installation directory)

**Note for GSI lxpool users: please use**
>. /cvmfs/eel.gsi.de/bin/go4login

The analysis is then built with a simple `make` call. Use `make clean; make;` for recompiling.

To start Go4 GUI just call
> go4

and launch analysis in local folder with `libGo4UserAnalysis.so`. Do not forget to create a Go4 hotstart script after this!

As usual, 
> go4analysis 

will offer a batch mode (please see go4analysis -h)

## Further information
More information on Go4 is available [here](https://www.gsi.de/en/work/research/experiment_electronics/data_processing/data_analysis/the_go4_home_page).

More information on the PLEIADES detector and FEBEX DAQ is available on the [PLEIADES Wiki](https://pleiades.wiki.triumf.ca/).

This code has been compiled and tested with Go4 v6.2.0 and root 6.26/04. 

