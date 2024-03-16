# CSE240A Branch Predictor Project

## Table of Contents
  * [Introduction](#introduction)
    - [Results](#results)
  * [Build and run the predictors](#build-and-run-the-predictors)
  * [Configurations](#configurations)
    - [Gshare](#gshare)
    - [Tournament](#tournament)
    - [Custom](#custom)

## Introduction
Branch prediction is critical to performance in modern processors.  An accurate branch predictor ensures that the front-end of the machine is capable of feeding the back-end with correct-path instructions.

In this project, our primary focus lies in the exploration of local and global branch prediction schemes, as well as their combination. We have implemented three branch predictors—Gshare, a modified Alpha 21264 Tournament predictor, and a local/Gshare tournament predictor—in C.

#### Results
Our custom predictor beats the Gshare and modified Alpha tournament predictors in all six traces. To run all three predictors in all six traces, simply run the following commands. 

```
  chmod +x run.sh
  ./run.sh
```

Please pay attention that the custom predictor has exactly same configurations as the tournament predictor i.e. --custom:<# ghistory>:<# lhistory>:<# index>. An example of running the custom predictor with 13 bits of global history, 13 bits of local history and 10 bits of PC index would be:   

`bunzip2 -kc ../traces/int_1.bz2 | ./predictor --custom:13:13:10`

## Build and run the predictors
A more detailed usage is provided below. 

In order to build the predictors you simply need to run `make` in the src/ directory of the project.  You can then run the program on an uncompressed trace as follows:   

`./predictor <options> [<trace>]`

If no trace file is provided then the predictor will read in input from STDIN. Some of the traces we provided are rather large when uncompressed so we have distributed them compressed with bzip2 (included in the Docker image).  If you want to run the predictor on a compressed trace, then you can do so by doing the following:

`bunzip2 -kc trace.bz2 | ./predictor <options>`

In either case the `<options>` that can be used to change the type of predictor
being run are as follows:

```
  --help       Print usage message
  --verbose    Outputs all predictions made by your
               mechanism. Will be used for correctness
               grading.
  --<type>     Branch prediction scheme. Available
               types are:
        static
        gshare:<# ghistory>
        tournament:<# ghistory>:<# lhistory>:<# index>
        custom:<# ghistory>:<# lhistory>:<# index>
```


## Configurations

#### Gshare

```
Configuration:
    ghistoryBits    // Indicates the length of Global History kept
```

#### Tournament
```
Configuration:
    ghistoryBits    // Indicates the length of Global History kept
    lhistoryBits    // Indicates the length of Local History kept in the PHT
    pcIndexBits     // Indicates the number of bits used to index the PHT
```

#### Custom

```
Configuration:
    ghistoryBits    // Indicates the length of Global History kept
    lhistoryBits    // Indicates the length of Local History kept in the PHT
    pcIndexBits     // Indicates the number of bits used to index the PHT
```

All history patterns are initialized to NOTTAKEN and all 2-bit predictors are initialized to WN (Weakly Not Taken). The Choice Predictor used to select which predictor to use is initialized to Weakly select the Global Predictor.

