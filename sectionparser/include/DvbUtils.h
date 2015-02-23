// Copyright (c) 2014 ARRIS Group, Inc. All rights reserved.
//
// This program is confidential and proprietary to ARRIS Group, Inc. (ARRIS),
// and may not be copied, reproduced, modified, disclosed to others, published
// or used, in whole or in part, without the express prior written permission
// of ARRIS.

#ifndef DVBUTILS_H_
#define DVBUTILS_H_

// C system includes
#include <stdint.h>

// C++ system includes
#include <string>

/**
 * Convert a binary coded decimal (byte) to decimal
 *
 * @param bcd binary coded decimal (byte)
 * @return decimal
 */
uint16_t BcdByteToDec(uint8_t bcd);

/**
 * Convert a binary coded decimal to decimal
 *
 * @param bcd binary coded decimal
 * @return decimal
 */
uint16_t BcdToDec(uint16_t bcd);

/**
 * Convert a binary coded decimal to time
 *
 * @param encodedTime encoded time
 * @param hour
 * @param min
 * @param sec
 */
void BcdToTime(int64_t encodedTime, uint32_t& hour, uint32_t& min, uint32_t& sec);

/**
 * Convert Modified Julian Date to date
 *
 * @param encodedTime
 * @return time_t
 */
time_t MjdToDate (int64_t encodedTime);

/**
 * Decode text information that is coded as described in ETSI EN 300 468 annex A
 *
 * @param str string
 * @param len length
 * @return decoded text in UTF-8
 */
std::string DecodeText(const unsigned char *str, size_t len);

#endif /* DVBUTILS_H_ */
