#include<stdio.h>
#include<unistd.h>
#include<string>

#include "DSSSCodeGenerator.hpp"

int main(int argc, char **argv)
{
if(argc < 2)
{
printf("Error, argument required.\nUsage: %s numberOfBitsInSequence optionalFileToSaveTo\n", argv[0]);
}

//Get path argument
std::string stringToSaveTo = "";
if(argc >= 3)
{
stringToSaveTo = std::string(argv[2]);
}

int sequenceSize;
try
{
sequenceSize = std::stoi(std::string(argv[1]));
}
catch(const std::exception &inputException)
{
fprintf(stderr, "Unable to convert argument for sequence size to integer\n");
return 1;
}

DSSSCodeGenerator myGenerator(sequenceSize, stringToSaveTo);

myGenerator.generateCodePairs();


return 0;
}
