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

#ifndef COMPONENTDESCRIPTOR_H_
#define COMPONENTDESCRIPTOR_H_

// Other libraries' includes

// Project's includes
#include "MpegDescriptor.h"
#include "DvbUtils.h"

// Syntax of the descriptor from ETSI EN 300 468
//
//component_descriptor(){
//    descriptor_tag             8 uimsbf
//    descriptor_length          8 uimsbf
//    reserved_future_use        4 bslbf
//    stream_content             4 uimsbf
//    component_type             8 uimsbf
//    component_tag              8 uimsbf
//    ISO_639_language_code     24 bslbf
//    for (i=0;i<N;i++){
//        text_char              8 uimsbf
//    }
//}

/**
 * Component descriptor class
 */
class ComponentDescriptor: public MpegDescriptor
{
public:
    /**
     * Constructor
     *
     * @param desc mpeg descriptor
     */
    ComponentDescriptor(const MpegDescriptor& desc)
        : MpegDescriptor(desc)
    {
    }

    /**
     * Destructor
     */
    virtual ~ComponentDescriptor()
    {
    }

    /**
     * Get the stream content type
     *
     * @return stream_content
     */
    uint8_t getStreamContent() const
    {
        return m_data[0] & 0xf;
    }

    /**
     * Get the component type
     *
     * @return component_type
     */
    uint8_t getComponentType() const
    {
        return m_data[1];
    }

    /**
     * Get the component tag
     *
     * @return component_tag
     */
    uint8_t getComponentTag()  const
    {
        return m_data[2];
    }

    /**
     * Get the language code
     *
     * @return language_code
     */
    std::string getLanguageCode(void) const
    {
        return std::string((const char*)m_data.data() + 3, 3);
    }

    /**
     * Get the text description of the elementary stream
     *
     * @return text (UTF)
     */
    std::string getText(void) const
    {
        return DecodeText(m_data.data() + 6, m_data.size() - 6);
    }
};

#endif /* COMPONENTDESCRIPTOR_H_ */
