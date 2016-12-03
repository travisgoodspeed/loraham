#ifndef UTILITIES_H_
#define UTILITIES_H_

// Returns the battery voltage as a float.
float voltage();

//! Uptime in seconds, correcting for rollover.
long int uptime();

// returns true if supplied packet should be retransmitted
bool shouldrt(uint8_t *buf);

#endif // UTILITIES_H_
