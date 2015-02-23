// Copyright (c) 2014 ARRIS Group, Inc. All rights reserved.
//
// This program is confidential and proprietary to ARRIS Group, Inc. (ARRIS),
// and may not be copied, reproduced, modified, disclosed to others, published
// or used, in whole or in part, without the express prior written permission
// of ARRIS.

#ifndef CONTENTDESCRIPTOR_H_
#define CONTENTDESCRIPTOR_H_

// Other libraries' includes

// Project's includes
#include "MpegDescriptor.h"

// Syntax of the descriptor from ETSI EN 300 468
//
//content_descriptor(){
//    descriptor_tag                 8 uimsbf
//    descriptor_length              8 uimsbf
//    for (i=0;i<N;i++) {
//        content_nibble_level_1     4 uimsbf
//        content_nibble_level_2     4 uimsbf
//        user_byte                  8 uimsbf
//    }
//}

/**
 * Content descriptor class
 */
class ContentDescriptor: public MpegDescriptor
{
public:
    /**
     * Constructor
     *
     * @param desc mpeg descriptor
     */
    ContentDescriptor(const MpegDescriptor& desc)
        : MpegDescriptor(desc)
    {
    }

    /**
     * Get the number of content identifiers (count)
     *
     * @return count
     */
    uint8_t getCount()
    {
        return m_data.size()/2;
    }

    /**
     * Get the first level of content identifier
     *
     * @param n
     * @return content_nibble_level_1
     */
    uint8_t getNibbleLvl1(uint8_t n) const
    {
        return m_data[n*2] >> 4;
    }

    /**
     * Get the second level of content identifier
     *
     * @param n
     * @return content_nibble_level_2
     */
    uint8_t getNibbleLvl2(uint8_t n) const
    {
        return m_data[n*2] & 0xf;
    }

    /**
     * Get the user byte
     *
     * @param n
     * @return user_byte
     */
    uint8_t getUserByte(uint8_t n) const
    {
        return m_data[n*2 + 1];
    }

};

#endif /* CONTENTDESCRIPTOR_H_ */
