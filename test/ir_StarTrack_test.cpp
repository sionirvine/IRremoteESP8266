// Copyright 2017 David Conran

#include "IRsend.h"
#include "IRsend_test.h"
#include "gtest/gtest.h"

// Tests for sendStarTrack().
// Test sending typical data only.
TEST(TestSendStarTrack, SendDataOnly) {
  IRsendTest irsend(4);
  irsend.begin();

  irsend.reset();
  irsend.sendStarTrack(0xD5F2A);  // Star Track TV Power Off.
  EXPECT_EQ("m4000s4000"
            "m500s2000m500s2000m500s2000m500s2000m500s1000m500s1000m500s2000"
            "m500s1000m500s2000m500s1000m500s2000m500s1000m500s1000m500s1000"
            "m500s1000m500s1000m500s2000m500s2000m500s1000m500s2000m500s1000"
            "m500s2000m500s1000m500s2000m500s8500",
            irsend.outputStr());

  irsend.reset();
}

// Test sending with different repeats.
TEST(TestSendStarTrack, SendWithRepeats) {
  IRsendTest irsend(4);
  irsend.begin();

  irsend.reset();
  irsend.sendStarTrack(0xD5F2A, STARTRACK_BITS, 1);  // 1 repeat.
  EXPECT_EQ("m4000s4000"
            "m500s2000m500s2000m500s2000m500s2000m500s1000m500s1000m500s2000"
            "m500s1000m500s2000m500s1000m500s2000m500s1000m500s1000m500s1000"
            "m500s1000m500s1000m500s2000m500s2000m500s1000m500s2000m500s1000"
            "m500s2000m500s1000m500s2000m500s8500"
            "m4000s4000"
            "m500s2000m500s2000m500s2000m500s2000m500s1000m500s1000m500s2000"
            "m500s1000m500s2000m500s1000m500s2000m500s1000m500s1000m500s1000"
            "m500s1000m500s1000m500s2000m500s2000m500s1000m500s2000m500s1000"
            "m500s2000m500s1000m500s2000m500s8500",
            irsend.outputStr());
  irsend.sendStarTrack(0xD5F2A, STARTRACK_BITS, 2);  // 2 repeat.
  EXPECT_EQ("m4000s4000"
            "m500s2000m500s2000m500s2000m500s2000m500s1000m500s1000m500s2000"
            "m500s1000m500s2000m500s1000m500s2000m500s1000m500s1000m500s1000"
            "m500s1000m500s1000m500s2000m500s2000m500s1000m500s2000m500s1000"
            "m500s2000m500s1000m500s2000m500s8500"
            "m4000s4000"
            "m500s2000m500s2000m500s2000m500s2000m500s1000m500s1000m500s2000"
            "m500s1000m500s2000m500s1000m500s2000m500s1000m500s1000m500s1000"
            "m500s1000m500s1000m500s2000m500s2000m500s1000m500s2000m500s1000"
            "m500s2000m500s1000m500s2000m500s8500"
            "m4000s4000"
            "m500s2000m500s2000m500s2000m500s2000m500s1000m500s1000m500s2000"
            "m500s1000m500s2000m500s1000m500s2000m500s1000m500s1000m500s1000"
            "m500s1000m500s1000m500s2000m500s2000m500s1000m500s2000m500s1000"
            "m500s2000m500s1000m500s2000m500s8500",
            irsend.outputStr());
}

// Tests for decodeStarTrack().

// Decode normal Star Track messages.
TEST(TestDecodeStarTrack, NormalDecodeWithStrict) {
  IRsendTest irsend(4);
  IRrecv irrecv(4);
  irsend.begin();

  // Normal Star Track 24-bit message.
  irsend.reset();
  irsend.sendStarTrack(0x123456);
  irsend.makeDecodeResult();
  ASSERT_TRUE(irrecv.decodeStarTrack(&irsend.capture, STARTRACK_BITS, true));
  EXPECT_EQ(STARTRACK, irsend.capture.decode_type);
  EXPECT_EQ(STARTRACK_BITS, irsend.capture.bits);
  EXPECT_EQ(0x123456, irsend.capture.value);

  irsend.reset();
  irsend.sendStarTrack(0x101);
  irsend.makeDecodeResult();
  ASSERT_TRUE(irrecv.decodeStarTrack(&irsend.capture, STARTRACK_BITS, true));
  EXPECT_EQ(STARTRACK, irsend.capture.decode_type);
  EXPECT_EQ(STARTRACK_BITS, irsend.capture.bits);
  EXPECT_EQ(0x101, irsend.capture.value);
}

// Decode normal repeated Star Track messages.
TEST(TestDecodeStarTrack, NormalDecodeWithRepeatAndStrict) {
  IRsendTest irsend(4);
  IRrecv irrecv(4);
  irsend.begin();

  // Normal Star Track 24-bit message.
  irsend.reset();
  irsend.sendStarTrack(0xD5F2A, STARTRACK_BITS, 2);
  irsend.makeDecodeResult();
  ASSERT_TRUE(irrecv.decodeStarTrack(&irsend.capture, STARTRACK_BITS, true));
  EXPECT_EQ(STARTRACK, irsend.capture.decode_type);
  EXPECT_EQ(STARTRACK_BITS, irsend.capture.bits);
  EXPECT_EQ(0xD5F2A, irsend.capture.value);
}

TEST(TestDecodeStarTrack, NormalDecodeWithNonStrict) {
  IRsendTest irsend(4);
  IRrecv irrecv(4);
  irsend.begin();

  // Illegal under length (16-bit) message
  irsend.reset();
  irsend.sendStarTrack(0x0, 16);
  irsend.makeDecodeResult();
  // Should fail with strict on.
  ASSERT_FALSE(irrecv.decodeStarTrack(&irsend.capture, STARTRACK_BITS, true));
  // And it should fail when we expect more bits.
  ASSERT_FALSE(irrecv.decodeStarTrack(&irsend.capture, STARTRACK_BITS, false));

  // Should pass if strict off if we ask for correct nr. of bits sent.
  ASSERT_TRUE(irrecv.decodeStarTrack(&irsend.capture, 16, false));
  EXPECT_EQ(STARTRACK, irsend.capture.decode_type);
  EXPECT_EQ(16, irsend.capture.bits);
  EXPECT_EQ(0x0, irsend.capture.value);

  // Should fail as we are expecting less bits than there are.
  ASSERT_FALSE(irrecv.decodeStarTrack(&irsend.capture, 12, false));
}

// Decode (non-standard) 64-bit messages.
// Decode unsupported Star Track messages.
TEST(TestDecodeStarTrack, Decode64BitMessages) {
  IRsendTest irsend(4);
  IRrecv irrecv(4);
  irsend.begin();

  irsend.reset();
  // Illegal size Star Track 64-bit message.
  irsend.sendStarTrack(0xFFFFFFFFFFFFFFFF, 64);
  irsend.makeDecodeResult();
  ASSERT_FALSE(irrecv.decodeStarTrack(&irsend.capture, STARTRACK_BITS, true));
  // Should work with a 'normal' match (not strict)
  ASSERT_TRUE(irrecv.decodeStarTrack(&irsend.capture, 64, false));
  EXPECT_EQ(STARTRACK, irsend.capture.decode_type);
  EXPECT_EQ(64, irsend.capture.bits);
  EXPECT_EQ(0xFFFFFFFFFFFFFFFF, irsend.capture.value);
}

// Decode real example via Issue #309
TEST(TestDecodeStarTrack, DecodeExamples) {
  IRsendTest irsend(4);
  IRrecv irrecv(4);
  irsend.begin();

  irsend.reset();
  // Star Track SR Power Off from Issue #309
  uint16_t rawdata_off[100] = {4060, 3918,
      508, 2004, 508, 2002, 510, 2002, 508, 2004, 506, 1050, 508, 1048,
      510, 2004, 508, 1048, 508, 2002, 510, 1050, 508, 2004, 510, 1048,
      508, 1050, 508, 1048, 508, 1050, 508, 1050, 508, 2004, 508, 2002,
      510, 1048, 508, 2004, 508, 1050, 506, 2004, 508, 1048, 510, 2002,
      456, 8446,
      3956, 3998,
      508, 2004, 508, 2002, 508, 2004, 508, 1978, 532, 1050, 508, 1050,
      508, 2002, 508, 1050, 508, 2004, 508, 1050, 508, 2002, 510, 1050,
      508, 1050, 508, 1048, 508, 1050, 508, 1050, 508, 2002, 510, 2002,
      508, 1050, 508, 2002, 510, 1050, 508, 2002, 508};
  irsend.sendRaw(rawdata_off, 100, 38);
  irsend.makeDecodeResult();

  ASSERT_TRUE(irrecv.decode(&irsend.capture));
  EXPECT_EQ(STARTRACK, irsend.capture.decode_type);
  EXPECT_EQ(STARTRACK_BITS, irsend.capture.bits);
  EXPECT_EQ(0xD5F2A, irsend.capture.value);

  // Star Track SR Volume Up from Issue #309
  uint16_t rawdata_volup[52] = {3972, 4002,
      504, 1982, 526, 2010, 502, 2010, 502, 2010, 500, 1056, 502, 1056,
      502, 2010, 500, 1056, 502, 2010, 502, 2010, 500, 2010, 502, 2010,
      502, 1056, 502, 1056, 502, 1056, 500, 1056, 502, 2010, 502, 2010,
      500, 1056, 502, 2008, 502, 1054, 504, 1054, 504, 1054, 500, 1056,
      450};

  irsend.reset();
  irsend.sendRaw(rawdata_volup, 52, 38);
  irsend.makeDecodeResult();

  ASSERT_TRUE(irrecv.decode(&irsend.capture));
  EXPECT_EQ(STARTRACK, irsend.capture.decode_type);
  EXPECT_EQ(STARTRACK_BITS, irsend.capture.bits);
  EXPECT_EQ(0xD0F2F, irsend.capture.value);
}

// Fail to decode a non-Star Track example via GlobalCache
TEST(TestDecodeStarTrack, FailToDecodeNonStarTrackExample) {
  IRsendTest irsend(4);
  IRrecv irrecv(4);
  irsend.begin();

  irsend.reset();
  uint16_t gc_test[71] = {38000, 1, 1, 172, 172, 22, 64, 22, 64, 22, 64, 22, 21,
                          22, 21, 22, 21, 22, 11, 22, 21, 22, 128, 22, 64, 22,
                          64, 22, 21, 22, 21, 22, 21, 22, 21, 22, 21, 22, 64,
                          22, 21, 22, 21, 22, 64, 22, 64, 22, 21, 22, 21, 22,
                          64, 22, 21, 22, 64, 22, 64, 22, 21, 22, 21, 22, 64,
                          22, 64, 22, 21, 22, 1820};
  irsend.sendGC(gc_test, 71);
  irsend.makeDecodeResult();

  ASSERT_FALSE(irrecv.decodeStarTrack(&irsend.capture));
  ASSERT_FALSE(irrecv.decodeStarTrack(&irsend.capture, STARTRACK_BITS, false));
}
