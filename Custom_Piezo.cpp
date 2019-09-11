#include "Custom_Piezo.h"

const int startUpNotes[5] = {NOTE_F5, NOTE_C5, NOTE_A5, NOTE_G5, NOTE_A5}; 
const int startUpDuration[5] = {4, 8, 4, 8, 1};								//4 is quarter notes, 8 is eighth notes

const int successfulConnection[3] = {NOTE_A6, NOTE_B6, NOTE_C7}	;
const int successfulConnectionDuration[3] = {10, 10, 8};
	
const int waiting[3] = {NOTE_F5, NOTE_F5, NOTE_F5};
const int waitingDuration[3] = {3, 3, 3};

void playNote(int duration, int note, int pin){
	tone(pin, note, duration);
	delay(duration * 1.3);
}

void playStartUpMelody(int pin){
	for(int i = 0; i < sizeof(startUpNotes)/sizeof(startUpNotes[0]); i++){
		playNote(1000 / startUpDuration[i], startUpNotes[i], pin);
	}
}

void playSuccessfulConnection(int pin){
	for(int i = 0; i < sizeof(successfulConnection)/sizeof(successfulConnection[0]); i++){
		playNote(1000 / successfulConnectionDuration[i], successfulConnection[i], pin);
	}
}

void playWaiting(int pin, int delayAfter){								//Use 750 for delayAfter, good default
	for(int i = 0; i < sizeof(waiting)/sizeof(waiting[0]); i++){
		playNote(1000 / waitingDuration[i], waiting[i], pin);
	}
	
	delay(delayAfter);
}

