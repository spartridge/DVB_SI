// Copyright (c) 2014 ARRIS Group, Inc. All rights reserved.
//
// This program is confidential and proprietary to ARRIS Group, Inc. (ARRIS),
// and may not be copied, reproduced, modified, disclosed to others, published
// or used, in whole or in part, without the express prior written permission
// of ARRIS.

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
