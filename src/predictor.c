//========================================================//
//  predictor.c                                           //
//  Source file for the Branch Predictor                  //
//                                                        //
//  Implement the various branch predictors below as      //
//  described in the README                               //
//========================================================//
#include <stdio.h>
#include "predictor.h"

//
// TODO:Student Information
//
const char *studentName = "NAME";
const char *studentID   = "PID";
const char *email       = "EMAIL";

//------------------------------------//
//      Predictor Configuration       //
//------------------------------------//

// Handy Global for use in output routines
const char *bpName[4] = { "Static", "Gshare",
                          "Tournament", "Custom" };

int ghistoryBits; // Number of bits used for Global History
int lhistoryBits; // Number of bits used for Local History
int pcIndexBits;  // Number of bits used for PC index
int bpType;       // Branch Prediction Type
int verbose;

//------------------------------------//
//      Predictor Data Structures     //
//------------------------------------//

//
//TODO: Add your own Branch Predictor data structures here
//
#define PC_SIZE 32

uint8_t* gHistoryTable;
uint8_t* gPredictTable;
uint8_t* lHistoryTable;
uint8_t* lPredictTable;
uint8_t* choiceTable;


//------------------------------------//
//        Predictor Functions         //
//------------------------------------//

// Initialize the predictor
//
void
init_predictor()
{
  //
  //TODO: Initialize Branch Predictor Data Structures
  //
  if (bpType == GSHARE) {
    size_t size = (size_t)(1 << ghistoryBits) / 4;
    gPredictTable = (uint8_t*)malloc(size);
    gHistoryTable = (uint8_t*)malloc(ghistoryBits / 8);
  }
  else if (bpType == TOURNAMENT) {
    size_t size = (size_t) (1 << pcIndexBits) / 4;
    choiceTable = (uint8_t*)malloc(size);

    gHistoryTable = (uint8_t*)malloc(ghistoryBits / 8);

    size = (size_t) (1 << ghistoryBits) / 4;
    gPredictTable = (uint8_t*)malloc(size);

    size = (size_t) (1 << pcIndexBits) * lhistoryBits / 8;
    lHistoryTable = (uint8_t*)malloc(size);

    size = (size_t) (1 << lhistoryBits) / 4;
    lPredictTable = (uint8_t*)malloc(size);
  }
}

// Make a prediction for conditional branch instruction at PC 'pc'
// Returning TAKEN indicates a prediction of taken; returning NOTTAKEN
// indicates a prediction of not taken
//
uint8_t
make_prediction(uint32_t pc)
{
  //
  //TODO: Implement prediction scheme
  //

  // Make a prediction based on the bpType
  switch (bpType) {
    case STATIC:
      return TAKEN;
    case GSHARE:
    case TOURNAMENT:
    case CUSTOM:
    default:
      break;
  }

  // If there is not a compatable bpType then return NOTTAKEN
  return NOTTAKEN;
}

// Train the predictor the last executed branch at PC 'pc' and with
// outcome 'outcome' (true indicates that the branch was taken, false
// indicates that the branch was not taken)
//
void
train_predictor(uint32_t pc, uint8_t outcome)
{
  //
  //TODO: Implement Predictor training
  //
  // update global history table
  *gHistoryTable = (*gHistoryTable << 1) | outcome;
  if (bpType == GSHARE) {
    // update global prediction table
    
  }
}
