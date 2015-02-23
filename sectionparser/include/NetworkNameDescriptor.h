// Copyright (c) 2014 ARRIS Group, Inc. All rights reserved.
//
// This program is confidential and proprietary to ARRIS Group, Inc. (ARRIS),
// and may not be copied, reproduced, modified, disclosed to others, published
// or used, in whole or in part, without the express prior written permission
// of ARRIS.

#ifndef NETWORKNAMEDESCRIPTOR_H_
#define NETWORKNAMEDESCRIPTOR_H_

// C++ system includes
#include <string>

// Other libraries' includes

// Project's includes
#include "MpegDescriptor.h"
#include "DvbUtils.h"

/**
 * Network name descriptor class
 */
class NetworkNameDescriptor: public MpegDescriptor
{
public:
    /**
     * Constructor
     *
     * @param desc mpeg descriptor
     */
    NetworkNameDescriptor(const MpegDescriptor& desc)
        : MpegDescriptor(desc)
    {
    }

    /**
     * Get the network name
     *
     * @return network_name
     */
    std::string getName()
    {
        return DecodeText(m_data.data(), m_data.size());
    }

    /**
     * Get a string with debug data
     *
     * @return string a text string for debug purposes
     */
    std::string toString()
    {
        return DecodeText(m_data.data(), m_data.size());
    }
};

#endif /* NETWORKNAMEDESCRIPTOR_H_ */
