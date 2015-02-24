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


#ifndef MPEGDESCRIPTOR_H_
#define MPEGDESCRIPTOR_H_

// C system includes
#include <stdint.h>

// C++ system includes
#include <vector>

/**
 * Descriptor tag enumeration
 */
enum class DescriptorTag : uint8_t
{
    NETWORK_NAME = 0x40,             //!< NETWORK_NAME
    SERVICE_LIST = 0x41,             //!< SERVICE_LIST
    STUFFING     = 0x42,             //!< STUFFING
    SATELLITE_DELIVERY = 0x43,       //!< SATELLITE_DELIVERY
    CABLE_DELIVERY = 0x44,           //!< CABLE_DELIVERY
    VBI_DATA = 0x45,                 //!< VBI_DATA
    VBI_TELETEXT = 0x46,             //!< VBI_TELETEXT
    BOUQUET_NAME = 0x47,             //!< BOUQUET_NAME
    SERVICE = 0x48,                  //!< SERVICE
    COUNTRY_AVAILABILITY = 0x49,     //!< COUNTRY_AVAILABILITY
    LINKAGE = 0x4a,                  //!< LINKAGE
    NVOD_REFERENCE = 0x4b,           //!< NVOD_REFERENCE
    TIME_SHIFTED_SERVICE = 0x4c,     //!< TIME_SHIFTED_SERVICE
    SHORT_EVENT = 0x4d,              //!< SHORT_EVENT
    EXTENDED_EVENT = 0x4e,           //!< EXTENDED_EVENT
    TIME_SHIFTED_EVENT = 0x4f,       //!< TIME_SHIFTED_EVENT
    COMPONENT = 0x50,                //!< COMPONENT
    MOSAIC = 0x51,                   //!< MOSAIC
    STREAM_IDENTIFIER = 0x52,        //!< STREAM_IDENTIFIER
    CA_IDENTIFIER = 0x53,            //!< CA_IDENTIFIER
    CONTENT_DESCRIPTOR = 0x54,       //!< CONTENT_DESCRIPTOR
    PARENTAL_RATING = 0x55,          //!< PARENTAL_RATING
    TELETEXT = 0x56,                 //!< TELETEXT
    TELEPHONE = 0x57,                //!< TELEPHONE
    LOCAL_TIME_OFFSET = 0x58,        //!< LOCAL_TIME_OFFSET
    SUBTITLING = 0x59,               //!< SUBTITLING
    TERRESTRIAL_DELIVERY = 0x5a,     //!< TERRESTRIAL_DELIVERY
    MULTILINGUAL_NETWORK_NAME = 0x5b,//!< MULTILINGUAL_NETWORK_NAME
    MULTILINGUAL_BOUQUET_NAME = 0x5c,//!< MULTILINGUAL_BOUQUET_NAME
    MULTILINGUAL_SERVICE_NAME = 0x5d,//!< MULTILINGUAL_SERVICE_NAME
    MULTILINGUAL_COMPONENT = 0x5e,   //!< MULTILINGUAL_COMPONENT
    PRIVATE_DATA_SPECIFIER = 0x5f,   //!< PRIVATE_DATA_SPECIFIER
    SERVICE_MOVE = 0x60,             //!< SERVICE_MOVE
    SHORT_SMOOTHING_BUFFER = 0x61,   //!< SHORT_SMOOTHING_BUFFER
    FREQUENCY_LIST = 0x62,           //!< FREQUENCY_LIST
    PARTIAL_TRANSPORT_STREAM = 0x63, //!< PARTIAL_TRANSPORT_STREAM
    DATA_BROADCAST = 0x64,           //!< DATA_BROADCAST
    SCRAMBLING = 0x65,               //!< SCRAMBLING
    DATA_BROADCAST_ID = 0x66,        //!< DATA_BROADCAST_ID
    TRANSPORT_STREAM = 0x67,         //!< TRANSPORT_STREAM
    DSNG = 0x68,                     //!< DSNG
    PDC = 0x69,                      //!< PDC
    AC_3 = 0x6a,                     //!< AC_3
    ANCILLARY_DATA = 0x6b,           //!< ANCILLARY_DATA
    CELL_LIST = 0x6c,                //!< CELL_LIST
    CELL_FREQUENCY_LINK = 0x6d,      //!< CELL_FREQUENCY_LINK
    ANNOUNCEMENT_SUPPORT = 0x6e,     //!< ANNOUNCEMENT_SUPPORT
    APPLICATION_SIGNALLING = 0x6f,   //!< APPLICATION_SIGNALLING
    ADAPTATION_FIELD_DATA = 0x70,    //!< ADAPTATION_FIELD_DATA
    SERVICE_IDENTIFIER = 0x71,       //!< SERVICE_IDENTIFIER
    SERVICE_AVAILABILITY = 0x72,     //!< SERVICE_AVAILABILITY
    DEFAULT_AUTHORITY = 0x73,        //!< DEFAULT_AUTHORITY
    RELATED_CONTENT = 0x74,          //!< RELATED_CONTENT
    TVA_ID = 0x75,                   //!< TVA_ID
    CONTENT_IDENTIFIER = 0x76,       //!< CONTENT_IDENTIFIER
    TIME_SLICE_FEC_IDENTIFIER = 0x77,//!< TIME_SLICE_FEC_IDENTIFIER
    ECM_REPETITION_RATE = 0x78,      //!< ECM_REPETITION_RATE
    S2_SATELLITE_DELIVERY = 0x79,    //!< S2_SATELLITE_DELIVERY
    ENHANCED_AC_3 = 0x7a,            //!< ENHANCED_AC_3
    DTS = 0x7b,                      //!< DTS
    AAC = 0x7c,                      //!< AAC
    XAIT_LOCATION = 0x7d,            //!< XAIT_LOCATION
    FTA_CONTENT_MANAGEMENT = 0x7e,   //!< FTA_CONTENT_MANAGEMENT
    EXTENSION = 0x7f,                //!< EXTENSION
    USER_DEFINED_START = 0x80,       //!< USER_DEFINED_START
    LOGICAL_CHANNEL = 0x9c,          //!< LOGICAL_CHANNEL
    USER_DEFINED_END = 0xfe,         //!< USER_DEFINED_END
    FORBIDDEN = 0xFF                 //!< FORBIDDEN
};

/**
 * MPEG descriptor base class
 */
class MpegDescriptor
{
public:
    /**
     * Constructor
     *
     * @param tag descriptor tag
     * @param data descriptor data
     * @param length data length
     */
    MpegDescriptor(DescriptorTag tag, uint8_t *data, uint8_t length);

    /**
     * Destructor
     */
    virtual ~MpegDescriptor()
    {
    }

    /**
     * Get descriptor data
     *
     * @return vector
     */
    const std::vector<uint8_t>& getData() const
    {
        return m_data;
    }

    /**
     * Get the tag of the descriptor
     *
     * @return tag DescriptorTag
     */
    DescriptorTag getTag() const
    {
        return m_tag;
    }

    /**
     * Set the tag of the descriptor
     * @param tag
     */
    void setTag(DescriptorTag tag)
    {
        m_tag = tag;
    }

    /**
     * Set the descriptor data
     *
     * @param data the descriptor data
     */
    void setData(const std::vector<uint8_t>& data)
    {
        m_data = data;
    }

    /**
     * Find a descriptor by tag
     *
     * @param list descriptors
     * @param tag descriptor tag to search for
     * @return descriptor descriptor if found, null otherwise
     */
    static const MpegDescriptor* find(const std::vector<MpegDescriptor>& list, DescriptorTag tag);

    /**
     * Find descriptors by tag
     * @param list descriptors
     * @param tag descriptor tag to search for
     * @return vector of descriptors if found, null otherwise
     */
    static std::vector<MpegDescriptor> findAll(const std::vector<MpegDescriptor>& list, DescriptorTag tag);

    /**
     * Parse descriptors
     *
     * @param p pointer to the raw data
     * @param length length of the raw data
     * @return vector of descriptors
     */
    static std::vector<MpegDescriptor> parseDescriptors(uint8_t* p, uint16_t length);

protected:
    /**
     * Descriptor data
     */
    std::vector<uint8_t> m_data;
private:
    /**
     * Descriptor tag
     */
    DescriptorTag m_tag;
};

#endif /* MPEGDESCRIPTOR_H_ */
