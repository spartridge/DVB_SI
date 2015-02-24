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


#ifndef SERVICELISTDESCRIPTOR_H_
#define SERVICELISTDESCRIPTOR_H_

// Other libraries' includes

// Project's includes
#include "MpegDescriptor.h"

// Syntax of the descriptor from ETSI EN 300 468
//
//service_list_descriptor(){
//    descriptor_tag          8 uimsbf
//    descriptor_length       8 uimsbf
//    for (i=0;i<N;i++){
//        service_id         16 uimsbf
//        service_type        8 uimsbf
//    }
//}

/**
 * Service list descriptor class
 */
class ServiceListDescriptor: public MpegDescriptor
{
public:
    /**
     * Constructor
     *
     * @param desc mpeg descriptor
     */
    ServiceListDescriptor(const MpegDescriptor& desc)
        : MpegDescriptor(desc)
    {
    }

    /**
     * Destructor
     */
    virtual ~ServiceListDescriptor()
    {
    }

    /**
     * Get the count
     *
     * @return count
     */
    uint8_t getCount() const
    {
        return m_data.size()/3;
    }

    /**
     * Get the service identifier
     *
     * @param n
     * @return service_id
     */
    uint16_t getServiceId(uint8_t n) const
    {
        return (m_data[n*3] << 8) | m_data[n*3 + 1];
    }

    /**
     * Get the service type
     *
     * @param n
     * @return service_type
     */
    uint16_t getServiceType(uint8_t n) const
    {
        return m_data[n*3 + 2];
    }
};

#endif /* SERVICELISTDESCRIPTOR_H_ */
