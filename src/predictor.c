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
// initialize the prediction table to WEAK TAKEN
void init_prediction_table(uint8_t *predictTable, size_t size) {
    // Check if memory allocation was successful
    if (predictTable == NULL) {
        return;
    }

    // Set each 2-bit block in the prediction table to WEAK TAKEN
    for (size_t i = 0; i < size; i++) {
        // 0x55 = 0b01010101
        predictTable[i] = 0x55;
    }
}

void
init_predictor()
{
  //
  //TODO: Initialize Branch Predictor Data Structures
  //
  if (bpType == GSHARE) {
    size_t size = (size_t)(1 << ghistoryBits) / 4;
    gPredictTable = (uint8_t*)malloc(size);
    init_prediction_table(gPredictTable, size); // intial the 2-bit predictors to WN

    gHistoryTable = (uint8_t*)malloc(ghistoryBits / 8);
  }
  else if (bpType == TOURNAMENT) {
    size_t size = (size_t) (1 << pcIndexBits) / 4;
    choiceTable = (uint8_t*)malloc(size);

    gHistoryTable = (uint8_t*)malloc(ghistoryBits / 8);

    size = (size_t) (1 << ghistoryBits) / 4;
    gPredictTable = (uint8_t*)malloc(size);
    init_prediction_table(gPredictTable, size); // intial the 2-bit predictors to WN

    size = (size_t) (1 << pcIndexBits) * lhistoryBits / 8;
    lHistoryTable = (uint8_t*)malloc(size);

    size = (size_t) (1 << lhistoryBits) / 4;
    lPredictTable = (uint8_t*)malloc(size);
    init_prediction_table(lPredictTable, size);
  }
}

// Make a prediction for conditional branch instruction at PC 'pc'
// Returning TAKEN indicates a prediction of taken; returning NOTTAKEN
// indicates a prediction of not taken
//
uint32_t calculate_xor(uint32_t pc, uint32_t history) {
    uint32_t m = ghistoryBits;
    // Create a bitmask to extract the lowest m bits
    uint32_t bitmask = (1 << m) - 1;
    
    // Extract the lowest m bits of the PC
    uint32_t lowest_m_bits = pc & bitmask;
    
    // Calculate the XOR of the lowest m bits
    uint32_t result = lowest_m_bits ^ history;
    return result;
}

uint8_t get_pred_block(uint32_t index) {
    // Calculate the starting bit position of the 2-bit block
    uint32_t bit_position = index * 2;

    // Calculate the byte index and the bit offset within the byte
    uint32_t byte_index = bit_position / 8;
    uint32_t bit_offset = bit_position % 8;

    // Extract the 2-bit block from the global prediction table
    uint8_t block = (gPredictTable[byte_index] >> bit_offset) & 0x3;
    // SN:00 WN:01 WT:10 ST:11
    return block;
}

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
    case GSHARE: {
      uint32_t xor_value = calculate_xor(pc, *gHistoryTable);
      uint8_t predict = get_pred_block(xor_value);
      // return predict;
      if (predict == 0 || predict == 1) return NOTTAKEN;
      else if (predict == 2 || predict == 3) return TAKEN;
      else break;
    }
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
  if (bpType == GSHARE) {
    // update global history table
    *gHistoryTable = (*gHistoryTable << 1) | outcome;
    // update global prediction table
    uint32_t xor_index = calculate_xor(pc, *gHistoryTable);
    uint32_t bit_position = xor_index * 2;
    uint32_t byte_index = bit_position / 8;
    uint32_t bit_offset = bit_position % 8;

    // Extract the 2-bit block from the global prediction table
    uint8_t block = (gPredictTable[byte_index] >> bit_offset) & 0x3;

    // Update the 2-bit block based on the outcome
    if (outcome == 1 && block < 3) {
        block++;
    } else if (outcome == 0 && block > 0) {
        block--;
    }

    // Store the updated 2-bit block back into the global prediction table
    gPredictTable[byte_index] &= ~(0x3 << bit_offset); // Clear the 2-bit block
    gPredictTable[byte_index] |= (block << bit_offset); // Set the updated 2-bit block
  }
}
