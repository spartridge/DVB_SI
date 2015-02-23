// Copyright (c) 2014 ARRIS Group, Inc. All rights reserved.
//
// This program is confidential and proprietary to ARRIS Group, Inc. (ARRIS),
// and may not be copied, reproduced, modified, disclosed to others, published
// or used, in whole or in part, without the express prior written permission
// of ARRIS.

#ifndef LOCALTIMEOFFSETDESCRIPTOR_H_
#define LOCALTIMEOFFSETDESCRIPTOR_H_

// C system includes
#include <stdint.h>

// C++ system includes
#include <string>

// Other libraries' includes

// Project's includes
#include "MpegDescriptor.h"

// Syntax of the descriptor from ETSI EN 300 468
//
//local_time_offset_descriptor(){
//    descriptor_tag                     8 uimsbf
//    descriptor_length                  8 uimsbf
//    for(i=0;i<N;i++){
//        country_code                  24 bslbf
//        country_region_id              6 bslbf
//        reserved                       1 bslbf
//        local_time_offset_polarity     1 bslbf
//        local_time_offset             16 bslbf
//        time_of_change                40 bslbf
//        next_time_offset              16 bslbf
//    }
//}

/**
 * Local time offset descriptor class
 */
class LocalTimeOffsetDescriptor: public MpegDescriptor
{
public:
    /**
     * Constructor
     *
     * @param desc mpeg descriptor
     */
    LocalTimeOffsetDescriptor(const MpegDescriptor& desc)
        : MpegDescriptor(desc)
    {
    }

    /**
     * Destructor
     */
    virtual ~LocalTimeOffsetDescriptor()
    {
    }

    /**
     * Get the count
     *
     * @return count
     */
    uint8_t getCount() const
    {
        return m_data.size()/13;
    }

    /**
     * Get the country code
     *
     * @param n
     * @return country_code
     */
    std::string getCountryCode(uint8_t n) const
    {
        return std::string((const char*)m_data.data() + (n*13), 3);
    }

    /**
     * Get the country region id
     *
     * @param n
     * @return country_region_id
     */
    uint8_t getCountryRegionId(uint8_t n) const
    {
        return (m_data[n*13 + 3] >> 2);
    }

    /**
     * Get the local time offset polarity
     *
     * @param n
     * @return local_time_offset_polarity
     */
    bool getPolarity(uint8_t n) const
    {
        return m_data[n*13 + 3] & 0x01;
    }

    /**
     * Get the local time offset
     *
     * @param n
     * @return local_time_offset
     */
    uint16_t getLocalTimeOffset(uint8_t n) const
    {
        return (m_data[n*13 + 4] << 8) | m_data[n*13 + 5];
    }

    /**
     * Get the time of change
     *
     * @param n
     * @return time_of_change
     */
    uint64_t getTimeOfChange(uint8_t n) const
    {
        return ((uint64_t)m_data[n*13 + 6] << 32) | ((uint64_t)m_data[n*13 + 7] << 24) |
                ((uint64_t)m_data[n*13 + 8] << 16) | ((uint64_t)m_data[n*13 + 9] << 8) |
                (uint64_t)m_data[n*13 + 10];
    }

    /**
     * Get the next time offset
     *
     * @param n
     * @return next_time_offset
     */
    uint16_t getNextTimeOffset(uint8_t n) const
    {
        return (m_data[n*13 + 11] << 8) | m_data[n*13 + 12];
    }
};

#endif /* LOCALTIMEOFFSETDESCRIPTOR_H_ */
