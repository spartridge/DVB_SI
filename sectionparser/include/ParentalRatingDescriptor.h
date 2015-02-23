// Copyright (c) 2014 ARRIS Group, Inc. All rights reserved.
//
// This program is confidential and proprietary to ARRIS Group, Inc. (ARRIS),
// and may not be copied, reproduced, modified, disclosed to others, published
// or used, in whole or in part, without the express prior written permission
// of ARRIS.

#ifndef PARENTALRATINGDESCRIPTOR_H_
#define PARENTALRATINGDESCRIPTOR_H_

// Other libraries' includes

// Project's includes
#include "MpegDescriptor.h"

// Syntax of the descriptor from ETSI EN 300 468
//
//parental_rating_descriptor(){
//    descriptor_tag         8 uimsbf
//    descriptor_length      8 uimsbf
//    for (i=0;i<N;i++){
//        country_code      24 bslbf
//        rating             8 uimsbf
//    }
//}

/**
 * Parental rating descriptor class
 */
class ParentalRatingDescriptor: public MpegDescriptor
{
public:
    /**
     * Constructor
     *
     * @param desc mpeg descriptor
     */
    ParentalRatingDescriptor(const MpegDescriptor& desc)
        : MpegDescriptor(desc)
    {
    }

    /**
     * Get the count
     *
     * @return count
     */
    uint8_t getCount() const
    {
        return m_data.size()/4;
    }

    /**
     * Get the country code
     *
     * @param n
     * @return country_code
     */
    std::string getCountryCode(uint8_t n) const
    {
        return std::string((const char*)m_data.data() + (n*4), 3);
    }

    /**
     * Get the rating
     *
     * @param n
     * @return rating
     */
    uint8_t getRating(uint8_t n) const
    {
        return m_data[n*4 + 3];
    }
};

#endif /* PARENTALRATINGDESCRIPTOR_H_ */
