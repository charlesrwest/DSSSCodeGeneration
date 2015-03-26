#ifndef  DSSSCODEGENERATORHPP
#define DSSSCODEGENERATORHPP

#include<string>
#include<vector>
#include<cstdio>
#include<random>
#include<limits>

#include "SOMException.hpp"
#include "SOMScopeGuard.hpp"
#include "DSSSCodeGeneratorMessage.pb.h"


//If the checkpoint file is bigger than 100000, something is wrong and we should abort 
#define MAX_CHECKPOINT_FILE_SIZE 100000


/*
This class actively searches for DSSS codes that are unlikely to be confused with one another or a time delayed version of themselves.
*/
class DSSSCodeGenerator
{
public:
/*
This function intializes all of the required datastructures to start searching for good DSSS codes.  It checks the given file path to see if it contains an good DSSS code and uses that as a "best" value until it finds something better.
@param inputSequenceLength: The length of the DSSS code to optimize
@param inputCheckpointPath: The path to the checkpoint file (if empty, it will use "checkpointCodeLength" -> "checkpoint32" for a 32 length code)

@exceptions: This function can throw exceptions
*/
DSSSCodeGenerator(uint32_t inputSequenceLength, const std::string &inputCheckpointPath = "");

/*
This function tells the class to begin generating/testing DSSS codes.  It will run until the program terminates or it throws an exception, checkpointing every time it finds a code that is better than its current best code pair.
*/
void generateCodePairs();

std::string checkpointPath; //The path to store checkpoints in

std::vector<int32_t> currentBestCode0;
std::vector<int32_t> currentBestCode1;
uint64_t currentBestCodeCost;

//Current pair being tried
std::vector<int32_t> codeCandidate0;
std::vector<int32_t> codeCandidate1;

};



/*
This function attempts to read a DSSSCodeGeneratorMessage from the given file and place it into the given buffer.
@param inputFilePath: The path of the checkpoint file to read
@param inputMessageBuffer: The DSSSCodeGeneratorMessage to place the result in
@return: True if a DSSSCodeGeneratorMessage was successfully retrieved and false otherwise

@exceptions: This function can throw exceptions
*/
bool readCheckpoint(const std::string &inputFilePath, DSSSCodeGeneratorMessage &inputMessageBuffer);

/*
This function attempts to write a DSSSCodeGeneratorMessage to the given file.
@param inputFilePath: The path of the checkpoint file to write to
@param inputMessage: The DSSSCodeGeneratorMessage to write to the file
@return: True if a DSSSCodeGeneratorMessage was successfully written and false otherwise

@exceptions: This function can throw exceptions
*/
bool writeCheckpoint(const std::string &inputFilePath, const DSSSCodeGeneratorMessage &inputMessage);

/*
This function returns the maximum value from a sequence when it is mis-aligned with itself.  It starts by having one sample of overlap in the front, then moves down until there is one sample of overlap in the end.
@param inputCodeSequence: The sequence to evaluate.
@return: The max sum of a misaligned self sequence
*/
uint64_t getSequenceSelfCost(const std::vector<int32_t> &inputCodeSequence);

/*
This function returns the maximum value from one sequence when it colliding with the another.  It starts by having one sample of overlap in the front, then moves down until there is one sample of overlap in the end.
@param inputCodeSequence0: The first sequence to evaluate.
@param inputCodeSequence1: The second sequence to evaluate
@return: The max sum of one of the sequences colliding with the other sequence
*/
uint64_t getCrossSequenceCost(const std::vector<int32_t> &inputCodeSequence0, const std::vector<int32_t> &inputCodeSequence1);


#endif 
