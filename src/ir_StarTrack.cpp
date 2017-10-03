// Copyright 2009 Ken Shirriff
// Copyright 2017 David Conran

#include <algorithm>
#include "IRrecv.h"
#include "IRsend.h"
#include "IRtimer.h"
#include "IRutils.h"

//  SSSSS  TTTTTTT   AAA   RRRRRR   TTTTTTT RRRRRR    AAA    CCCCC  KK  KK
// SS        TTT    AAAAA  RR   RR    TTT   RR   RR  AAAAA  CC    C KK KK
//  SSSSS    TTT   AA   AA RRRRRR     TTT   RRRRRR  AA   AA CC      KKKK
//      SS   TTT   AAAAAAA RR  RR     TTT   RR  RR  AAAAAAA CC    C KK KK
//  SSSSS    TTT   AA   AA RR   RR    TTT   RR   RR AA   AA  CCCCC  KK  KK

// Constants
// Ref:
//   https://github.com/markszabo/IRremoteESP8266/issues/309
#define STARTRACK_TICK             500U
#define STARTRACK_HDR_MARK_TICKS     8U
#define STARTRACK_HDR_MARK         (STARTRACK_HDR_MARK_TICKS * STARTRACK_TICK)
#define STARTRACK_HDR_SPACE_TICKS    8U
#define STARTRACK_HDR_SPACE        (STARTRACK_HDR_SPACE_TICKS * STARTRACK_TICK)
#define STARTRACK_BIT_MARK_TICKS     1U
#define STARTRACK_BIT_MARK         (STARTRACK_BIT_MARK_TICKS * STARTRACK_TICK)
#define STARTRACK_ONE_SPACE_TICKS    2U
#define STARTRACK_ONE_SPACE        (STARTRACK_ONE_SPACE_TICKS * STARTRACK_TICK)
#define STARTRACK_ZERO_SPACE_TICKS   4U
#define STARTRACK_ZERO_SPACE       (STARTRACK_ZERO_SPACE_TICKS * STARTRACK_TICK)
#define STARTRACK_MIN_GAP_TICKS     17U
#define STARTRACK_MIN_GAP          (STARTRACK_MIN_GAP_TICKS * STARTRACK_TICK)


#if SEND_STARTRACK
// Send a Star Track SR 9090PLUS formatted message.
//
// Args:
//   data:   The message to be sent.
//   nbits:  The bit size of the message being sent. typically STARTRACK_BITS.
//   repeat: The number of times the message is to be repeated.
//
// Status: ALPHA / Untested.
//
// Ref: https://github.com/markszabo/IRremoteESP8266/issues/309
void IRsend::sendStarTrack(uint64_t data, uint16_t nbits, uint16_t repeat) {
  // Set 38kHz IR carrier frequency & a 1/3 (33%) duty cycle.
  enableIROut(38, 33);
  // We always send a message, even for repeat=0, hence '<= repeat'.
  for (uint16_t i=0; i <= repeat; i++) {
    // Header
    mark(STARTRACK_HDR_MARK);
    space(STARTRACK_HDR_SPACE);
    // Data
    sendData(STARTRACK_BIT_MARK, STARTRACK_ONE_SPACE, STARTRACK_BIT_MARK,
             STARTRACK_ZERO_SPACE, data, nbits, true);
    // Footer
    mark(STARTRACK_BIT_MARK);
    space(STARTRACK_MIN_GAP);
  }
}
#endif

#if DECODE_STARTRACK
// Decode the supplied Star Track message.
//
// Args:
//   results: Ptr to the data to decode and where to store the decode result.
//   nbits:   Nr. of bits to expect in the data portion.
//            Typically STARTRACK_BITS.
//   strict:  Flag to indicate if we strictly adhere to the specification.
// Returns:
//   boolean: True if it can decode it, false if it can't.
//
// Status: ALPHA / Untested.
//
bool IRrecv::decodeStarTrack(decode_results *results, uint16_t nbits,
                             bool strict) {
  if (results->rawlen < 2 * nbits + HEADER + FOOTER - 1)
    return false;  // Can't possibly be a valid Star Track message.
  if (strict && nbits != STARTRACK_BITS)
    return false;  // We expect Star Track to be 32 bits of message.

  uint64_t data = 0;
  uint16_t offset = OFFSET_START;

  // Header
  if (!matchMark(results->rawbuf[offset], STARTRACK_HDR_MARK)) return false;
  // Calculate how long the common tick time is based on the header mark.
  uint32_t m_tick = results->rawbuf[offset++] * RAWTICK /
      STARTRACK_HDR_MARK_TICKS;
  if (!matchSpace(results->rawbuf[offset], STARTRACK_HDR_SPACE)) return false;
  // Calculate how long the common tick time is based on the header space.
  uint32_t s_tick = results->rawbuf[offset++] * RAWTICK /
      STARTRACK_HDR_SPACE_TICKS;
  // Data
  match_result_t data_result = matchData(&(results->rawbuf[offset]), nbits,
                                         STARTRACK_BIT_MARK_TICKS * m_tick,
                                         STARTRACK_ONE_SPACE_TICKS * s_tick,
                                         STARTRACK_BIT_MARK_TICKS * m_tick,
                                         STARTRACK_ZERO_SPACE_TICKS * s_tick);
  if (data_result.success == false) return false;
  data = data_result.data;
  offset += data_result.used;
  // Footer
  if (!matchMark(results->rawbuf[offset++], STARTRACK_BIT_MARK_TICKS * m_tick))
    return false;
  if (offset < results->rawlen &&
      !matchAtLeast(results->rawbuf[offset], STARTRACK_MIN_GAP_TICKS * s_tick))
    return false;

  // Compliance

  // Success
  results->bits = nbits;
  results->value = data;
  results->decode_type = STARTRACK;
  results->command = 0;
  results->address = 0;
  return true;
}
#endif
