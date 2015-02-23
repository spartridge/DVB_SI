// Copyright (c) 2014 ARRIS Group, Inc. All rights reserved.
//
// This program is confidential and proprietary to ARRIS Group, Inc. (ARRIS),
// and may not be copied, reproduced, modified, disclosed to others, published
// or used, in whole or in part, without the express prior written permission
// of ARRIS.

#ifndef MULTILINGUALCOMPONENTDESCRIPTOR_H_
#define MULTILINGUALCOMPONENTDESCRIPTOR_H_

// C++ system includes
#include <vector>

// Other libraries' includes

// Project's includes
#include "MpegDescriptor.h"
#include "DvbUtils.h"

/**
 * Multilingual component descriptor class
 */
class MultilingualComponentDescriptor: public MpegDescriptor
{
public:
    /**
     * Constructor
     *
     * @param desc mpeg descriptor
     */
    MultilingualComponentDescriptor(const MpegDescriptor& desc)
        : MpegDescriptor(desc)
    {
        const uint8_t* p = m_data.data();
        if(p)
        {
            const uint8_t* end = p + m_data.size();

            // component tag
            p += 1;

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
     * Get the component tag
     *
     * @return component_tag
     */
    uint8_t getComponentTag()  const
    {
        return m_data[0];
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
     * Get the length of text
     *
     * @param n
     * @return text_length
     */
    uint8_t getTextLength(uint8_t n) const
    {
        return *(m_items[n] + 3);
    }

    /**
     * Get the text
     *
     * @param n
     * @return text
     */
    std::string getText(uint8_t n) const
    {
        return DecodeText(m_items[n] + 4, getTextLength(n));
    }

private:
    /**
     * List of items
     */
    std::vector<const uint8_t*> m_items;
};

#endif /* MULTILINGUALCOMPONENTDESCRIPTOR_H_ */
