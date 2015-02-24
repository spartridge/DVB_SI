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


#ifndef MULTILINGUALNETWORKNAMEDESCRIPTOR_H_
#define MULTILINGUALNETWORKNAMEDESCRIPTOR_H_

// C++ system includes
#include <vector>

// Other libraries' includes

// Project's includes
#include "MpegDescriptor.h"
#include "DvbUtils.h"

// Syntax of the descriptor from ETSI EN 300 468
//
//multilingual_network_name_descriptor(){
//    descriptor_tag                  8 uimsbf
//    descriptor_length               8 uimsbf
//    for (i=0;i<N;i++) {
//        ISO_639_language_code      24 bslbf
//        network_name_length         8 uimsbf
//        for (j=0;j<N;j++){
//            char                    8 uimsbf
//        }
//    }
//}

/**
 * Multilingual network name descriptor class
 */
class MultilingualNetworkNameDescriptor: public MpegDescriptor
{
public:
    /**
     * Constructor
     *
     * @param desc mpeg descriptor
     */
    MultilingualNetworkNameDescriptor(const MpegDescriptor& desc)
        : MpegDescriptor(desc)
    {
        const uint8_t* p = m_data.data();
        if(p)
        {
            const uint8_t* end = p + m_data.size();
            while(p + 3 < end)
            {
                m_items.push_back(p);
                // language code
                p += 3;
                // network name length and network name
                p += p[0] + 1;
            }
        }
    }

    /**
     * Get the count
     *
     * @return count
     */
    uint8_t getCount() const
    {
        return m_items.size();
    }

    /**
     * Get the language code
     *
     * @param n
     * @return language_code
     */
    std::string getLanguageCode(uint8_t n) const
    {
        return std::string((const char*)m_items[n], 3);
    }

    /**
     * Get the length of the network name
     * @param n
     * @return network_name_length
     */
    uint8_t getNetworkNameLength(uint8_t n) const
    {
        return *(m_items[n] + 3);
    }

    /**
     * Get the network name
     *
     * @param n
     * @return network_name
     */
    std::string getNetworkName(uint8_t n) const
    {
        return DecodeText(m_items[n] + 4, getNetworkNameLength(n));
    }

private:
    /**
     * List of items
     */
    std::vector<const uint8_t*> m_items;
};

#endif /* MULTILINGUALNETWORKNAMEDESCRIPTOR_H_ */
