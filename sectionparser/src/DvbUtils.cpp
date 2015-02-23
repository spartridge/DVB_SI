// Copyright (c) 2014 ARRIS Group, Inc. All rights reserved.
//
// This program is confidential and proprietary to ARRIS Group, Inc. (ARRIS),
// and may not be copied, reproduced, modified, disclosed to others, published
// or used, in whole or in part, without the express prior written permission
// of ARRIS.

// C system includes
#define __STDC_FORMAT_MACROS
#include <inttypes.h>
#include <time.h>
#include <sys/time.h>
#include <errno.h>

// Other libraries' includes

// Project's includes
#include "oswrap.h"
#include "DvbUtils.h"

using std::string;

/**
 * Convert a binary coded decimal (byte) to decimal
 *
 * @param bcd binary coded decimal (byte)
 * @return decimal
 */
uint16_t BcdByteToDec(uint8_t bcd)
{
    return (bcd >> 4)*10 + (bcd & 0x0f);
}

/**
 * Convert a binary coded decimal to decimal
 *
 * @param bcd binary coded decimal
 * @return decimal
 */
uint16_t BcdToDec(uint16_t bcd)
{
    uint16_t res = 0;
    uint16_t multiplier = 1;
    uint16_t digit = 0;

    // Decoding the binary coded decimal
    // Each decimal digit is represented by a four-bit binary value
    while (bcd > 0)
    {
        // Get the digit
        digit = bcd & 0x0F;

        // Shift to the next one
        bcd >>= 4;

        // Adding the digit to the result
        res += multiplier * digit;

        // Shift the multiplier
        multiplier *= 10;
    }

    return res;
}

/**
 * Convert a binary coded decimal to time
 *
 * @param encodedTime encoded time
 * @param hour
 * @param min
 * @param sec
 */
void BcdToTime(int64_t encodedTime, uint32_t& hour, uint32_t& min, uint32_t& sec)
{
    hour = BcdToDec((encodedTime >> 16) & 0xFF);
    min = BcdToDec((encodedTime >> 8) & 0xFF);
    sec = BcdToDec(encodedTime & 0xFF);
}

/**
 * Convert Modified Julian Date to date
 *
 * @param encodedTime
 * @return time_t
 */
time_t MjdToDate (int64_t encodedTime)
{
    int32_t J, C, Y, M;
    int32_t mjd = encodedTime >> 24;
    uint32_t hour = 0;
    uint32_t min = 0;
    uint32_t sec = 0;

    // Let's decode the time first
    BcdToTime(encodedTime, hour, min, sec);

    // Decoding the date
    // Adapted from Fliegel/van Flandern ACM 11/#10 p 657 Oct 1968
    J = mjd + 2400001 + 68569;
    C = 4 * J / 146097;
    J = J - (146097 * C + 3) / 4;
    Y = 4000 * (J + 1) / 1461001;
    J = J - 1461 * Y / 4 + 31;
    M = 80 * J / 2447;
    int32_t day = J - 2447 * M / 80;
    J = M / 11;
    int32_t month = M + 2 - (12 * J);
    int32_t year = 100 * (C - 49) + Y + J;

    // Converting to time_t
    struct tm time = {0};
    time.tm_mday = day;
    time.tm_mon = month - 1;
    time.tm_year = year - 1900;
    time.tm_hour = hour;
    time.tm_min = min;
    time.tm_sec = sec;

    return mktime(&time);
}


/**
 * Convert an ISO-8859-1 string to a UTF-8 string
 *
 * @param in input
 * @param len length
 * @return string in UTF-8
 */
static string Iso8859ToUtf(const unsigned char *in, size_t len)
{
    string res;

    // Let's convert each character from iso8859 to UTF-8 one by one
    for(size_t i = 0; i < len; i++)
    {
        // If the value is greater than 0x80 then it requires 2 bytes in UTF-8
        // Let's figure out whether we need one byte or two for the UTF-8 representation
        if (in[i] < 0x80)
        {
            res += in[i];
        }
        else
        {
            // leading byte
            res += 0xc0 | (in[i] & 0xc0) >> 6;
            // trailing byte
            res += 0x80 | (in[i] & 0x3f);
        }
    }

    return res;
}

/**
 * Decode text information that is coded as described in ETSI EN 300 468 annex A
 *
 * @param str string
 * @param len length
 * @return decoded text in UTF-8
 */
string DecodeText(const unsigned char *str, size_t len)
{
    // Sanity check
    if(!str || len == 0)
    {
        OS_LOG(DVB_ERROR, "<%s> Invalid argument passed (%p, %lu)\n", __FUNCTION__, str, len);
        return string("");
    }

    OS_LOG(DVB_TRACE1, "<%s> called: str[0] = 0x%x, len = %lu\n", __FUNCTION__, str[0], len);

    // ETSI EN 300 468: Text information is coded using character sets and methods described in annex A.
    // A.2 Selection of character table:
    // Text fields can optionally start with non-spacing, non-displayed data which specifies the alternative character
    // table to be used for the remainder of the text item.

    /// @TODO: Implement as per spec.
    // Let's assume ISO-8859-1 for the time being.
    string res = Iso8859ToUtf(str, len);

    return res;
}
