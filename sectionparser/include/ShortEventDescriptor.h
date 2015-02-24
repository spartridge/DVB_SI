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
