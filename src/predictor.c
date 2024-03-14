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
const char *studentName = "Xuanyu Chen & Ruichun Yang";
const char *studentID   = "A59025159 & A59023739";
const char *email       = "xuc020@ucsd.edu & ruy015@ucsd.edu";

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

/* -------- Gshare ---------- */
uint32_t brHistoryReg; // branch history register
uint8_t* pHistoryTable; // pattern history table


/* -------- Tournament ---------- */
uint32_t gHistory;
uint32_t* lHistoryTable;

uint8_t* gPredictTable;
uint8_t* lPredictTable;
uint8_t* choiceTable;

// Store results from two predictors
// reuse for training
uint8_t lPredict;
uint8_t gPredict;


/* -------- Custom ---------- */
// a combination of gshare and tournament
uint16_t gHistoryCustom;
uint16_t* lHistoryTableCustom;

uint8_t* gPredictTableCustom;
uint8_t* lPredictTableCustom;
uint8_t* choiceTableCustom;

uint8_t lPredictCustom;
uint8_t gPredictCustom;


//------------------------------------//
//           Helper Functions         //
//------------------------------------//

// shift 2-bit predictors in a given entry based on outcome
/*      
        T(1)    T       T
      ----->  ----->  ----->
    SN(0)   WN      WT      ST(3)
      <-----  <-----  <-----
        NT(0)   NT      NT
 */
void pred_shift(uint8_t outcome, uint8_t* pred)
{
  if((outcome == TAKEN) && (*pred != ST)){
    (*pred)++;
  }else if((outcome == NOTTAKEN) && (*pred != SN)){
    (*pred)--;
  }
}

// shift predictors for custom predictor
void pred_shift_custom(uint8_t outcome, uint16_t idx, uint8_t* predTable)
{
  uint32_t entryIdx = ((uint32_t)idx * PREDSIZE) / ENTRYSIZE;
  uint32_t entryOffset = ((uint32_t)idx * PREDSIZE) % ENTRYSIZE;

  uint8_t pred = (predTable[entryIdx] >> entryOffset) & ((1 << PREDSIZE) - 1);

  if((outcome == TAKEN) && (pred != ST)){
    pred++;
  }else if((outcome == NOTTAKEN) && (pred != SN)){
    pred--;
  }

  predTable[entryIdx] &= ~(((1 << PREDSIZE) - 1) << entryOffset);
  predTable[entryIdx] |= (pred << entryOffset);
}

// retrieve 2-bit predictors in a predictor table
// for Custom predictor
uint8_t pred_retrieve(uint16_t idx, uint8_t* predTable)
{
  uint32_t entryIdx = (idx * PREDSIZE) / ENTRYSIZE;
  uint32_t entryOffset = (idx * PREDSIZE) % ENTRYSIZE;

  uint8_t pred = (predTable[entryIdx] >> entryOffset) & ((1 << PREDSIZE) - 1);

  return pred;
}

//------------------------------------//
//        Predictor Functions         //
//------------------------------------//

/* -------- Gshare ---------- */

// init for Gshare
void init_gshare()
{
  // init branch history reg
  brHistoryReg = 0;

  // init patten history table to weak not taken
  int phtSize = (1 << ghistoryBits);
  pHistoryTable = (uint8_t*) malloc(phtSize * sizeof(uint8_t));

  for(int i = 0; i < phtSize; ++i){
    pHistoryTable[i] = WN;
  }
}

// free up tables for Gshare
void destroy_gshare()
{
  free(pHistoryTable);
}

// make prediction for Gshare
uint8_t pred_gshare(uint32_t pc)
{
  uint32_t mask = (1 << ghistoryBits) - 1;
  uint32_t pattern = (pc ^ brHistoryReg) & mask;
  uint8_t pred = pHistoryTable[pattern];

  if(verbose){
    printf("GSHARE: Get pred %u from pht %u, pc: %u, br his: %u\n", pred, pattern, pc, brHistoryReg);
  }
  
  if (pred == SN || pred == WN){
    return NOTTAKEN;
  } else if(pred == WT || pred == ST){
    return TAKEN;
  } else{
    printf("ERROR: undefined 2-bit predictors %u for gshare\n", pred);
    return NOTTAKEN;
  }
}

// update Gshare tables and history
void train_gshare(uint32_t pc, uint8_t outcome)
{
  uint32_t mask = (1 << ghistoryBits) - 1;
  uint32_t pattern = (pc ^ brHistoryReg) & mask;
  pred_shift(outcome, &(pHistoryTable[pattern]));

  brHistoryReg = ((brHistoryReg << 1) & mask) | outcome;
}

/* -------- Tournament ---------- */

//init for Tournament
void init_tournament()
{
  // init local history table and global history to 0
  gHistory = 0;
  int lhtSize = (1 << pcIndexBits);
  lHistoryTable = (uint32_t*) calloc(lhtSize, sizeof(uint32_t));

  // init local/global prediction, choice table to WN
  int lptSize = (1 << lhistoryBits);
  int gptSize = (1 << ghistoryBits);
  lPredictTable = (uint8_t*) malloc(lptSize * sizeof(uint8_t));
  gPredictTable = (uint8_t*) malloc(gptSize * sizeof(uint8_t));
  choiceTable = (uint8_t*) malloc(gptSize * sizeof(uint8_t));

  for(int i = 0; i < lptSize; ++i){
    lPredictTable[i] = WN;
  }

  for(int i = 0; i < gptSize; ++i){
    gPredictTable[i] = WN;
    choiceTable[i] = WN;  // weak global
  }
}

// free up tables for Tournament
void destroy_tournament()
{
  free(lHistoryTable);
  free(lPredictTable);
  free(gPredictTable);
  free(choiceTable);
}

// make prediction for Tournament
uint8_t pred_tournament(uint32_t pc)
{
  // get local prediction
  uint32_t lIndex = pc & ((1 << pcIndexBits) - 1);
  uint32_t lPattern = lHistoryTable[lIndex];
  uint8_t lpred = lPredictTable[lPattern]; // 2-bit predictor
  if(lpred == SN || lpred == WN){ // choose global
    lPredict = NOTTAKEN;
  }else if(lpred == WT || lpred == ST){ // choose local
    lPredict = TAKEN;
  }else{
    printf("ERROR: undefined local predictor %u for tournament\n", lpred);
    lPredict = NOTTAKEN;
  }

  // get global prediction
  uint32_t gIndex = gHistory & ((1 << ghistoryBits) - 1);
  uint8_t gpred = gPredictTable[gIndex];
  if(gpred == SN || gpred == WN){ // choose global
    gPredict = NOTTAKEN;
  }else if(gpred == WT || gpred == ST){ // choose local
    gPredict = TAKEN;
  }else{
    printf("ERROR: undefined global predictor %u for tournament\n", gpred);
    gPredict = NOTTAKEN;
  }

  // get choice
  uint8_t choice = choiceTable[gIndex];

  if(verbose){
    printf("TOURNAMENT: Local pred %u, global pred %u, choice %u, pc: %u, lHis: %u, gHis: %u\n", lPredict, gPredict, choice, pc, lPattern, gHistory);
  }
  
  if(choice == SN || choice == WN){ // choose global
    return gPredict;
  }else if(choice == WT || choice == ST){ // choose local
    return lPredict;
  }else{
    printf("ERROR: undefined choice %u for tournament\n", choice);
    return NOTTAKEN;
  }

}

// update tournament tables and history
void train_tournament(uint32_t pc, uint8_t outcome)
{
  // update choice table
  uint32_t gMask = (1 << ghistoryBits) - 1;
  uint32_t gIndex = gHistory & gMask;

  if(gPredict != lPredict){
    if(gPredict == outcome){ // decrease choice (NT) -> move toward global
      pred_shift(NOTTAKEN, &(choiceTable[gIndex]));
    }else{ // increase choice (T) -> move toward local
      pred_shift(TAKEN, &(choiceTable[gIndex]));
    }
  }

  // update global
  pred_shift(outcome, &(gPredictTable[gIndex]));
  gHistory = ((gHistory << 1) & gMask) | outcome;

  // update local
  uint32_t lIndex = pc & ((1 << pcIndexBits) - 1);
  uint32_t lPattern = lHistoryTable[lIndex];
  pred_shift(outcome, &(lPredictTable[lPattern]));

  lPattern = ((lPattern << 1) & ((1 << lhistoryBits) - 1)) | outcome;
  lHistoryTable[lIndex] = lPattern;
  
}

/* -------- Custom ---------- */

//init for Custom
void init_custom()
{
  // init local history table and global history to 0
  gHistoryCustom = NOTTAKEN;
  int lhtSize = (1 << pcIndexBits);
  lHistoryTableCustom = (uint16_t*) calloc(lhtSize, sizeof(uint16_t));

  // init local/global prediction. choice table
  // note that each entry in this table will contain 4 2-bit predictors
  // all 4 predictors will be iniated to WN
  // thus each entry is 01010101 -> 0x55;
  int lptSize = (1 << lhistoryBits) / 4;
  int gptSize = (1 << ghistoryBits) / 4;
  lPredictTableCustom = (uint8_t*) malloc(lptSize * sizeof(uint8_t));
  gPredictTableCustom = (uint8_t*) malloc(gptSize * sizeof(uint8_t));
  choiceTableCustom = (uint8_t*) malloc(gptSize * sizeof(uint8_t));

  for(int i = 0; i < lptSize; ++i){
    lPredictTableCustom[i] = WN4;
  }

  for(int i = 0; i < gptSize; ++i){
    gPredictTableCustom[i] = WN4;
    choiceTableCustom[i] = WN4;  // weak global
  }
}

// free up tables for Custom
void destroy_custom()
{
  free(lHistoryTableCustom);
  free(lPredictTableCustom);
  free(gPredictTableCustom);
  free(choiceTableCustom);
}

// make prediction for Custom
uint8_t pred_custom(uint32_t pc)
{
  // get local prediction
  uint16_t lIndex = pc & ((1 << pcIndexBits) - 1);
  uint16_t lPattern = lHistoryTableCustom[lIndex] & ((1 << lhistoryBits) - 1);
  uint8_t lpred = pred_retrieve(lPattern, lPredictTableCustom);
  if(lpred == SN || lpred == WN){ // choose global
    lPredictCustom = NOTTAKEN;
  }else if(lpred == WT || lpred == ST){ // choose local
    lPredictCustom = TAKEN;
  }else{
    printf("ERROR: undefined local predictor %u for custom\n", lpred);
    lPredictCustom = NOTTAKEN;
  }

  // get global prediction
  uint16_t mask = ((1 << ghistoryBits) - 1);
  uint16_t gIndex = (pc & mask) ^ (gHistoryCustom & mask);
  uint8_t gpred = pred_retrieve(gIndex, gPredictTableCustom);
  if(gpred == SN || gpred == WN){ // choose global
    gPredictCustom = NOTTAKEN;
  }else if(gpred == WT || gpred == ST){ // choose local
    gPredictCustom = TAKEN;
  }else{
    printf("ERROR: undefined global predictor %u for custom\n", gpred);
    gPredictCustom = NOTTAKEN;
  }

  // get choice
  uint16_t choiceIndex = gHistoryCustom & mask;
  uint8_t choice = pred_retrieve(choiceIndex, choiceTableCustom);

  if(verbose){
    printf("CUSTOM: Local pred %u, global pred %u, choice %u, pc: %u, lHis: %u, gHis: %u\n", lPredictCustom, gPredictCustom, choice, pc, lPattern, gHistoryCustom);
  }

  if(choice == SN || choice == WN){ // choose global
    return gPredictCustom;
  }else if(choice == WT || choice == ST){ // choose local
    return lPredictCustom;
  }else{
    printf("ERROR: undefined choice %u for custom\n", choice);
    return NOTTAKEN;
  }

}

// update Custom tables and history
void train_custom(uint32_t pc, uint8_t outcome)
{
  // update choice table
  uint16_t gmask = ((1 << ghistoryBits) - 1);
  uint16_t choiceIndex = gHistoryCustom & gmask;

  if(gPredictCustom != lPredictCustom){
    if(gPredictCustom == outcome){ // decrease choice (NT) -> move toward global
      pred_shift_custom(NOTTAKEN, choiceIndex, choiceTableCustom);
    }else{
      pred_shift_custom(TAKEN, choiceIndex, choiceTableCustom);
    }
  }

  // update global
  uint16_t gIndex = (pc & gmask) ^ (gHistoryCustom & gmask);
  pred_shift_custom(outcome, gIndex, gPredictTableCustom);
  gHistoryCustom = ((gHistoryCustom << 1) & gmask) | outcome;

  // update local
  uint16_t lIndex = pc & ((1 << pcIndexBits) - 1);
  uint16_t lPattern = lHistoryTableCustom[lIndex];
  pred_shift_custom(outcome, lPattern, lPredictTableCustom);

  lPattern = ((lPattern << 1) & ((1 << lhistoryBits) - 1)) | outcome;
  lHistoryTableCustom[lIndex] = lPattern;

}


//------------------------------------//
//              Wrap-up               //
//------------------------------------//

void
init_predictor()
{
  //
  //TODO: Initialize Branch Predictor Data Structures
  //

  // Initialization based on the bpType
  switch (bpType) {
    case STATIC:
      break;
    case GSHARE:
      init_gshare();
      break;
    case TOURNAMENT:
      init_tournament();
      break;
    case CUSTOM:
      init_custom();
      break;
    default:
      break;
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
      return pred_gshare(pc);
    case TOURNAMENT:
      return pred_tournament(pc);
    case CUSTOM:
      return pred_custom(pc);
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

  switch (bpType) {
    case STATIC:
      break;
    case GSHARE:
      train_gshare(pc, outcome);
      break;
    case TOURNAMENT:
      train_tournament(pc, outcome);
      break;
    case CUSTOM:
      train_custom(pc, outcome);
      break;
    default:
      break;
  }
}

// free up dynamically allocated tables
void destroy()
{
  switch (bpType) {
    case STATIC:
      break;
    case GSHARE:
      destroy_gshare();
      break;
    case TOURNAMENT:
      destroy_tournament();
      break;
    case CUSTOM:
      destroy_custom();
      break;
    default:
      break;
  }
}

