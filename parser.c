#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser.h"

#define headerLength 3
#define checksumLength 1
#define validMessageLength 13
#define validMessageLength2 8
#define validOutput 10
#define validOutput2 5

char *parser(char fullMessage[], int MessageLen){

	char *err = malloc(1);
	err[0] = 0x00;

	if(fullMessage == NULL){
	  return err;
	}
	else{
	        int strippedLength = MessageLen - headerLength - checksumLength;
	        if((strippedLength != validMessageLength)&&(strippedLength != validMessageLength2)){
	        	return err;
	        }
	        else if((fullMessage[3] != 0x77) || (fullMessage[4] != 0x00) || (fullMessage[5] != 0x0A)){
	        	return err;
	        }

	        else if(strippedLength == validMessageLength){
	                char *output = malloc(validOutput);
	                memcpy(output, &fullMessage[6], validOutput);
	                return output;
	        }

	        else if (strippedLength == validMessageLength2){
	                char *output = malloc(validOutput2);
	                memcpy(output, &fullMessage[6], validOutput2);
	                return output;
	        }
	}
}


