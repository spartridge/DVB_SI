// Copyright (c) 2014 ARRIS Group, Inc. All rights reserved.
//
// This program is confidential and proprietary to ARRIS Group, Inc. (ARRIS),
// and may not be copied, reproduced, modified, disclosed to others, published
// or used, in whole or in part, without the express prior written permission
// of ARRIS.

#ifndef SERVICEDESCRIPTOR_H_
#define SERVICEDESCRIPTOR_H_

// C++ system includes
#include <string>

// Other libraries' includes

// Project's includes
#include "MpegDescriptor.h"
#include "DvbUtils.h"

/**
 * Service descriptor class
 */
class ServiceDescriptor: public MpegDescriptor
{
public:
    /**
     * Constructor
     *
     * @param desc mpeg descriptor
     */
    ServiceDescriptor(const MpegDescriptor& desc)
        : MpegDescriptor(desc)
    {
    }

    /**
     * Destructor
     */
    virtual ~ServiceDescriptor()
    {
    }

    /**
     * Get the service type
     *
     * @return service_type
     */
    uint8_t getServiceType()
    {
        return m_data[0];
    }

    /**
     * Get the length of the service provider name
     *
     * @return service_provider_name_length
     */
    uint8_t getServiceProviderNameLength()
    {
        return m_data[1];
    }

    /**
     * Get the service provider name
     *
     * @return service_provider_name
     */
    std::string getServiceProviderName()
    {
        return DecodeText(m_data.data() + 2, getServiceProviderNameLength());
    }

    /**
     * Get the length of service name
     *
     * @return service_name_length
     */
    uint8_t getServiceNameLength()
    {
        return m_data[2 + getServiceProviderNameLength()];
    }

    /**
     * Get the service name
     * @return
     */
    std::string getServiceName()
    {
        return DecodeText(m_data.data() + 3 + getServiceProviderNameLength(), getServiceNameLength());
    }

    /**
     * Get a string with debug data
     *
     * @return string a text string for debug purposes
     */
    std::string toString()
    {
        return std::string("Not implemented");
    }
};

#endif /* SERVICEDESCRIPTOR_H_ */
