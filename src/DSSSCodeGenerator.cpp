#include "DSSSCodeGenerator.hpp"

/*
This function intializes all of the required datastructures to start searching for good DSSS codes.  It checks the given file path to see if it contains an good DSSS code and uses that as a "best" value until it finds something better.
@param inputSequenceLength: The length of the DSSS code to optimize
@param inputCheckpointPath: The path to the checkpoint file (if empty, it will use "checkpointCodeLength" -> "checkpoint32" for a 32 length code)

@exceptions: This function can throw exceptions
*/
DSSSCodeGenerator::DSSSCodeGenerator(uint32_t inputSequenceLength, const std::string &inputCheckpointPath) 
{
if((inputSequenceLength % 2) != 0)
{
throw SOMException("Attempted to generate code of odd length\n", INVALID_FUNCTION_INPUT, __FILE__, __LINE__);
}

checkpointPath = inputCheckpointPath;

//Try to load default if no path given
if(inputCheckpointPath == "")
{
checkpointPath = std::string("checkpoint") + std::to_string(inputSequenceLength);
}



//Initialize vectors to be legal codes (-1, 1)
for(int i=0; i<inputSequenceLength; i++)
{
currentBestCode0.push_back((i % 2)*2 -1 ); //Alternating -1,1s
}
currentBestCode1 = currentBestCode0;
codeCandidate0 = currentBestCode0;
codeCandidate1 = currentBestCode0;
currentBestCodeCost = std::numeric_limits<uint64_t>::max();

DSSSCodeGeneratorMessage message;
SOM_TRY
if(readCheckpoint(checkpointPath, message) != true)
{
//Couldn't read checkpoint, so we are going to make our own checkpoint file
checkpointPath = std::string("checkpoint") + std::to_string(inputSequenceLength);
printf("Couldn't load check point, so now saving to: %s\n", checkpointPath.c_str());
return;
}
SOM_CATCH("Error reading checkpoint message\n")

if(message.optimal_code_candidate0().size() != inputSequenceLength || message.optimal_code_candidate1().size() != inputSequenceLength)
{
//We are trying to load an invalid checkpoint (or different size one), so we are going to call that an error
throw SOMException("Attempted to load wrong size or invalid checkpoint\n", INVALID_FUNCTION_INPUT, __FILE__, __LINE__);
}

//Got a message 
std::copy(message.optimal_code_candidate0().begin(), message.optimal_code_candidate0().end(), currentBestCode0.begin());
std::copy(message.optimal_code_candidate1().begin(), message.optimal_code_candidate1().end(), currentBestCode1.begin());
currentBestCodeCost = (uint64_t)  message.cost();


printf("(loaded) Best cost: %llu\n", (long long unsigned int)  currentBestCodeCost);
printf("(loaded) Vector 0: ");
for(int i=0; i<currentBestCode0.size(); i++)
{
printf("%d ", currentBestCode0[i]);
}
printf("\n");

printf("(loaded) Vector 1: ");
for(int i=0; i<currentBestCode1.size(); i++)
{
printf("%d ", currentBestCode1[i]);
}
printf("\n");
}

/*
This function tells the class to begin generating/testing DSSS codes.  It will run until the program terminates or it throws an exception, checkpointing every time it finds a code that is better than its current best code pair.
*/
void DSSSCodeGenerator::generateCodePairs()
{
//Try codes until stopped
while(true)
{
//Generate a new pair of codes to try by randomly permutating 
shuffle(codeCandidate0.begin(), codeCandidate0.end(), std::random_device());
shuffle(codeCandidate1.begin(), codeCandidate1.end(), std::random_device());

uint64_t maxSelfCost0 = getSequenceSelfCost(codeCandidate0);
uint64_t maxSelfCost1 = getSequenceSelfCost(codeCandidate1);
uint64_t maxCrossCost = getCrossSequenceCost(codeCandidate0, codeCandidate1);

//Find overall max
uint64_t overallMax = maxSelfCost0;
overallMax = overallMax < maxSelfCost1 ? maxSelfCost1 : overallMax;
overallMax = overallMax < maxCrossCost ? maxCrossCost : overallMax;

//printf("Costs: %lu %lu %lu\n", maxSelfCost0, maxSelfCost1, maxCrossCost);

if(overallMax < currentBestCodeCost)
{
//We have a winner
currentBestCode0 = codeCandidate0;
currentBestCode1 = codeCandidate1;
currentBestCodeCost = overallMax;

printf("Best cost: %llu\n", (long long unsigned int) currentBestCodeCost);
printf("Vector 0: ");
for(int i=0; i<currentBestCode0.size(); i++)
{
printf("%d ", currentBestCode0[i]);
}
printf("\n");

printf("Vector 1: ");
for(int i=0; i<currentBestCode1.size(); i++)
{
printf("%d ", currentBestCode1[i]);
}
printf("\n");


//Checkpoint
DSSSCodeGeneratorMessage message;
message.set_cost(currentBestCodeCost);

for(int i=0; i<currentBestCode0.size(); i++)
{
message.add_optimal_code_candidate0(currentBestCode0[i]);
}

for(int i=0; i<currentBestCode1.size(); i++)
{
message.add_optimal_code_candidate1(currentBestCode1[i]);
}

if(writeCheckpoint(checkpointPath, message) != true)
{
throw SOMException("Error writing checkpoint\n", FILE_SYSTEM_ERROR, __FILE__, __LINE__);
}
}

}

}

/*
This function attempts to read a DSSSCodeGeneratorMessage from the given file and place it into the given buffer.
@param inputFilePath: The path of the checkpoint file to read
@param inputMessageBuffer: The DSSSCodeGeneratorMessage to place the result in
@return: True if a DSSSCodeGeneratorMessage was successfully retrieved and false otherwise

@exceptions: This function can throw exceptions
*/
bool readCheckpoint(const std::string &inputFilePath, DSSSCodeGeneratorMessage &inputMessageBuffer)
{

//Open file and retrieve previous best code if it is there
FILE *checkpointFile = fopen(inputFilePath.c_str(), "rb");
if(checkpointFile == nullptr)
{
return false;
}
//Ensure file will be closed however function exits
SOMScopeGuard checkpointFileGuard([&](){fclose(checkpointFile);});

std::string serializedMessage;
char readBuffer[1024];
while(true) //Read file
{
int numberOfBytesRead = fread((void *) readBuffer, sizeof(char), 1024, checkpointFile);

//Check for file read error
if(numberOfBytesRead < 1024 && ferror(checkpointFile)) 
{
throw SOMException("Error reading checkpoint file\n", FILE_SYSTEM_ERROR, __FILE__, __LINE__);
}

//Check for file too big
if(serializedMessage.size() + numberOfBytesRead > MAX_CHECKPOINT_FILE_SIZE)
{
throw SOMException("Error reading checkpoint file (file too big)\n", AN_ASSUMPTION_WAS_VIOLATED_ERROR, __FILE__, __LINE__);
}

//Actually save the data
serializedMessage.append(readBuffer, numberOfBytesRead);

if(numberOfBytesRead < 1024) //Check if we reached the end of the file
{
break; //Read whole file, so exit loop
}
}

//Attempt to deserialize message
inputMessageBuffer.ParseFromString(serializedMessage);
if(!inputMessageBuffer.IsInitialized())
{
return false;
}

return true;
}

/*
This function attempts to write a DSSSCodeGeneratorMessage to the given file.
@param inputFilePath: The path of the checkpoint file to write to
@param inputMessage: The DSSSCodeGeneratorMessage to write to the file
@return: True if a DSSSCodeGeneratorMessage was successfully written and false otherwise

@exceptions: This function can throw exceptions
*/
bool writeCheckpoint(const std::string &inputFilePath, const DSSSCodeGeneratorMessage &inputMessage)
{
//Open file for writing
FILE *checkpointFile = fopen(inputFilePath.c_str(), "wb");
if(checkpointFile == nullptr)
{
return false;
}
//Ensure file will be closed however function exits
SOMScopeGuard checkpointFileGuard([&](){fclose(checkpointFile);});

//Serialize message
std::string serializedMessage;
inputMessage.SerializeToString(&serializedMessage);

//Write out
return fwrite(serializedMessage.c_str(), serializedMessage.size(), 1, checkpointFile) == 1;  //Return if write was successful
}

/*
This function returns the maximum value from a sequence when it is mis-aligned with itself.
@param inputCodeSequence: The sequence to evaluate.
@return: The max sum of a misaligned self sequence
*/
uint64_t getSequenceSelfCost(const std::vector<int32_t> &inputCodeSequence)
{
int maxSelfCost = 0;

for(int i=1; i<inputCodeSequence.size(); i++) //Start at 1 to prevent perfect alignment
{
int sum = 0;
for(int ii=0; ii < inputCodeSequence.size(); ii++)
{
sum += inputCodeSequence[(i+ii) % inputCodeSequence.size()]*inputCodeSequence[ii];
}

if(maxSelfCost < sum)
{
maxSelfCost = sum;
}
}

return maxSelfCost;
}

/*
This function returns the maximum value from one sequence when it colliding with the another (cyclical collisions).  
@param inputCodeSequence0: The first sequence to evaluate.
@param inputCodeSequence1: The second sequence to evaluate
@return: The max sum of one of the sequences colliding with the other sequence
*/
uint64_t getCrossSequenceCost(const std::vector<int32_t> &inputCodeSequence0, const std::vector<int32_t> &inputCodeSequence1)
{
int maxCrossCost = 0;
for(int i=0; i<inputCodeSequence0.size(); i++)
{
int sum = 0;
for(int ii=0; ii < inputCodeSequence1.size(); ii++)
{
sum += inputCodeSequence1[(i+ii)%inputCodeSequence1.size()]*inputCodeSequence0[ii];
}

if(maxCrossCost < sum)
{
maxCrossCost = sum;
}
}

return maxCrossCost;
}

