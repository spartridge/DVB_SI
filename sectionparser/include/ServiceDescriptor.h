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
