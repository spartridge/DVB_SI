// Copyright (c) 2014 ARRIS Group, Inc. All rights reserved.
//
// This program is confidential and proprietary to ARRIS Group, Inc. (ARRIS),
// and may not be copied, reproduced, modified, disclosed to others, published
// or used, in whole or in part, without the express prior written permission
// of ARRIS.

#ifndef EXTENDEDEVENTDESCRIPTOR_H_
#define EXTENDEDEVENTDESCRIPTOR_H_

// C++ system includes
#include <string>

// Other libraries' includes

// Project's includes
#include "MpegDescriptor.h"
#include "DvbUtils.h"

// Syntax of the descriptor from ETSI EN 300 468
//
//extended_event_descriptor() {
//    descriptor_tag                    8 uimsbf
//    descriptor_length                 8 uimsbf
//    descriptor_number                 4 uimsbf
//    last_descriptor_number            4 uimsbf
//    ISO_639_language_code            24 bslbf
//    length_of_items                   8 uimsbf
//    for ( i=0;i<N;i++){
//        item_description_length       8 uimsbf
//        for (j=0;j<N;j++){
//            item_description_char     8 uimsbf
//        }
//        item_length                   8 uimsbf
//        for (j=0;j<N;j++){
//            item_char                 8 uimsbf
//        }
//    }
//    text_length                       8 uimsbf
//    for (i=0;i<N;i++){
//        text_char                     8 uimsbf
//    }
//}

/**
 * Extended event descriptor class
 */
class ExtendedEventDescriptor: public MpegDescriptor
{
public:
    /**
     * Constructor
     *
     * @param desc mpeg descriptor
     */
    ExtendedEventDescriptor(const MpegDescriptor& desc)
        : MpegDescriptor(desc)
    {
        const uint8_t* p = m_data.data();
        if(p)
        {
            p += 5;
            const uint8_t* end = p + getLengthOfItems();
            while(p < end)
            {
                m_items.push_back(p);

                // item description and its length
                p += p[0] + 1; 

                // item and its length
                p += p[0] + 1;
            }
        }
    }

    /**
     * Get the number of the descriptor
     *
     * @return descriptor_number
     */
    uint8_t getNumber() const
    {
        return m_data[0] >> 4;
    }

    /**
     * Get the number of the last descriptor
     *
     * @return last_descriptor_number
     */
    uint8_t getLastNumber() const
    {
        return m_data[0] & 0xf;
    }

    /**
     * Get the language code
     *
     * @return language_code
     */
    std::string getLanguageCode() const
    {
        return std::string((const char*)m_data.data() + 1, 3);
    }

    /**
     * Get the length of the items
     *
     * @return length_of_items
     */
    uint8_t getLengthOfItems() const
    {
        return m_data[4];
    }

    /**
     * Get the number of items
     *
     * @return number_of_items
     */
    uint8_t getNumberOfItems() const
    {
        return m_items.size();
    }

    /**
     * Get the length of the item description
     *
     * @param n
     * @return item_description_length
     */
    uint8_t getItemDescriptionLength(uint8_t n) const
    {
        return *(m_items[n]);
    }

    /**
     * Get the description of the item
     *
     * @param n
     * @return item_description_char
     */
    std::string getItemDescription(uint8_t n) const
    {
        return DecodeText(m_items[n] + 1, *(m_items[n]));
    }

    /**
     * Get the length of the item
     *
     * @param n
     * @return item_length
     */
    uint8_t getItemLength(uint8_t n) const
    {
        return *(m_items[n] + getItemDescriptionLength(n) + 1);
    }

    /**
     * Get the item
     *
     * @param n
     * @return item_char
     */
    std::string getItem(uint8_t n) const
    {
        return DecodeText(m_items[n] + getItemDescriptionLength(n) + 2, getItemLength(n));
    }

    /**
     * Get the length of the text
     *
     * @return text_length
     */
    uint8_t getTextLength() const
    {
        return m_data[5 + getLengthOfItems()];
    }

    /**
     * Get the text
     *
     * @return text
     */
    std::string getText() const
    {
        return DecodeText(m_data.data() + 6 + getLengthOfItems(), getTextLength());
    }

private:
    /**
     * List of items
     */
    std::vector<const uint8_t*> m_items;
};

#endif /* EXTENDEDEVENTDESCRIPTOR_H_ */
