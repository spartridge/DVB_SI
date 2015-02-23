// Copyright (c) 2014 ARRIS Group, Inc. All rights reserved.
//
// This program is confidential and proprietary to ARRIS Group, Inc. (ARRIS),
// and may not be copied, reproduced, modified, disclosed to others, published
// or used, in whole or in part, without the express prior written permission
// of ARRIS.

#ifndef LOGICALCHANNELDESCRIPTOR_H_
#define LOGICALCHANNELDESCRIPTOR_H_

// Other libraries' includes

// Project's includes
#include "MpegDescriptor.h"

// Syntax of the descriptor from ETSI EN 300 468
//
//logical_channel_descriptor(){
//     descriptor_tag                 8 uimsbf
//     descriptor_length              8 uimsbf
//     for (i=0; i<N;i++){
//         service_id                16 uimsbf
//         visible_service_flag       1 bslbf
//         reserved                   5 bslbf
//         logical_channel_number    10 uimsbf
//     }
//}

/**
 * Logican channel descriptor class
 */
class LogicalChannelDescriptor: public MpegDescriptor
{
public:
    /**
     * Constructor
     *
     * @param desc mpeg descriptor
     */
    LogicalChannelDescriptor(const MpegDescriptor& desc)
        : MpegDescriptor(desc)
    {
    }

    /**
     * Destructor
     */
    virtual ~LogicalChannelDescriptor()
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
     * Get the service identifier
     *
     * @param n
     * @return service_id
     */
    uint16_t getServiceId(uint8_t n) const
    {
        return (m_data[n*4] << 8) | m_data[n*4 + 1];
    }

    /**
     * Check if service is visible
     *
     * @param n
     * @return true if visible, false otherwise
     */
    bool isVisible(uint8_t n)
    {
        return m_data[n*4 + 2] & 0x80;
    }

    /**
     * Get the logical channel number
     *
     * @param n
     * @return logical_channel_number
     */
    uint16_t getLcn(uint8_t n) const
    {
        return ((m_data[n*4 + 2] & 0x3) << 8) | m_data[n*4 + 3];
    }
};

#endif /* LOGICALCHANNELDESCRIPTOR_H_ */
