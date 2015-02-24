//
// DVB_SI for Reference Design Kit (RDK)
//
// Copyright (C) 2015  Dmitry Barablin & Stan Partridge (stan.partridge@arris.com)
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA


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
