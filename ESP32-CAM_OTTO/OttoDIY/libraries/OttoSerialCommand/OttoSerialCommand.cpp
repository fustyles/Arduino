/*
    Library modified from: "SerialCommand.h" by Steven Cogswell http://awtfy.com
     -- Removed portion of the original library to not interfere with interruptions 
     -- (disable SoftwareSerial support, and thus don't have to use "#include <SoftwareSerial.h>" in the sketches)
*/

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

#include "OttoSerialCommand.h"


#include <string.h>


// Constructor makes sure some things are set. 
OttoSerialCommand::OttoSerialCommand()
{
	strncpy(delim," ",MAXDELIMETER);  // strtok_r needs a null-terminated string
	term='\r';   // return character, default terminator for commands
	numCommand=0;    // Number of callback handlers installed
	clearBuffer(); 
}



//
// Initialize the command buffer being processed to all null characters
//
void OttoSerialCommand::clearBuffer()
{
	for (int i=0; i<SERIALCOMMANDBUFFER; i++) 
	{
		buffer[i]='\0';
	}
	bufPos=0; 
}

// Retrieve the next token ("word" or "argument") from the Command buffer.  
// returns a NULL if no more tokens exist.   
char *OttoSerialCommand::next() 
{
	char *nextToken;
	nextToken = strtok_r(NULL, delim, &last); 
	return nextToken; 
}

// This checks the Serial stream for characters, and assembles them into a buffer.  
// When the terminator character (default '\r') is seen, it starts parsing the 
// buffer for a prefix command, and calls handlers setup by addCommand() member
void OttoSerialCommand::readSerial() 
{
	bool onlyOneCommand = true;
	// If we're using the Hardware port, check it.   Otherwise check the user-created OttoSoftwareSerial Port
	while ((Serial.available() > 0)&&(onlyOneCommand==true))
	{
		int i; 
		boolean matched; 
		
			inChar=Serial.read();   // Read single available character, there may be more waiting
		
		if (inChar==term) {     // Check for the terminator (default '\r') meaning end of command

			onlyOneCommand=false; //
			
			bufPos=0;           // Reset to start of buffer
			token = strtok_r(buffer,delim,&last);   // Search for command at start of buffer
			if (token == NULL) return; 
			matched=false; 
			for (i=0; i<numCommand; i++) {
				
				// Compare the found command against the list of known commands for a match
				if (strncmp(token,CommandList[i].command,SERIALCOMMANDBUFFER) == 0) 
				{
					
					// Execute the stored handler function for the command
					(*CommandList[i].function)(); 
					clearBuffer(); 
					matched=true; 
					break; 
				}
			}
			if (matched==false) {
				(*defaultHandler)(); 
				clearBuffer(); 
			}

		}
		if (isprint(inChar))   // Only printable characters into the buffer
		{
			buffer[bufPos++]=inChar;   // Put character into buffer
			buffer[bufPos]='\0';  // Null terminate
			if (bufPos > SERIALCOMMANDBUFFER-1) bufPos=0; // wrap buffer around if full  
		}
	}
}

// Adds a "command" and a handler function to the list of available commands.  
// This is used for matching a found token in the buffer, and gives the pointer
// to the handler function to deal with it. 
void OttoSerialCommand::addCommand(const char *command, void (*function)())
{
	if (numCommand < MAXSERIALCOMMANDS) {
				
		strncpy(CommandList[numCommand].command,command,SERIALCOMMANDBUFFER); 
		CommandList[numCommand].function = function; 
		numCommand++; 
	} 
}

// This sets up a handler to be called in the event that the receveived command string
// isn't in the list of things with handlers.
void OttoSerialCommand::addDefaultHandler(void (*function)())
{
	defaultHandler = function;
}
