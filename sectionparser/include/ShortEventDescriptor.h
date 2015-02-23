// Copyright (c) 2014 ARRIS Group, Inc. All rights reserved.
//
// This program is confidential and proprietary to ARRIS Group, Inc. (ARRIS),
// and may not be copied, reproduced, modified, disclosed to others, published
// or used, in whole or in part, without the express prior written permission
// of ARRIS.

#ifndef SHORTEVENTDESCRIPTOR_H_
#define SHORTEVENTDESCRIPTOR_H_

// C system includes
#include <stdint.h>

// C++ system includes
#include <string>

// Other libraries' includes

// Project's includes
#include "DvbUtils.h"
#include "MpegDescriptor.h"

/**
 * Short event descriptor class
 */
class ShortEventDescriptor: public MpegDescriptor
{
public:
    /**
     * Constructor
     *
     * @param desc mpeg descriptor
     */
    ShortEventDescriptor(const MpegDescriptor& desc)
        : MpegDescriptor(desc)
    {
    }

    /**
     * Destructor
     */
    virtual ~ShortEventDescriptor()
    {
    }

    /**
     * Get the language code
     *
     * @return language_code
     */
    std::string getLanguageCode(void) const
    {
        return std::string((const char*)m_data.data(), 3);
    }

    /**
     * Get the length of the event name
     *
     * @return event_name_length
     */
    uint8_t getEventNameLength() const
    {
        return m_data[3];
    }

    /**
     * Get the event name
     *
     * @return event_name
     */
    std::string getEventName() const
    {
        return DecodeText(m_data.data() + 4, getEventNameLength());
    }

    /**
     * Get the length of the text
     *
     * @return text_length
     */
    uint8_t getTextLength() const
    {
        return m_data[4 + getEventNameLength()];
    }

    /**
     * Get the text
     *
     * @return text
     */
    std::string getText() const
    {
        return DecodeText(m_data.data() + 5 + getEventNameLength(), getTextLength());
    }

    /**
     * Get a string with debug data
     *
     * @return string a text string for debug purposes
     */
    std::string toString() const
    {
        return std::string("Not implemented");
    }
};

#endif /* SHORTEVENTDESCRIPTOR_H_ */
