// Copyright (c) 2014 ARRIS Group, Inc. All rights reserved.
//
// This program is confidential and proprietary to ARRIS Group, Inc. (ARRIS),
// and may not be copied, reproduced, modified, disclosed to others, published
// or used, in whole or in part, without the express prior written permission
// of ARRIS.

#ifndef SERVICE_H_
#define SERVICE_H_

// C system includes
#include <stdint.h>

// C++ system includes
#include <vector>

// Other libraries' includes

// Project's includes
#include "MpegDescriptor.h"

/**
 * SDT service class
 */
class DvbService
{
public:
    /**
     * Constructor
     *
     * @param id service identifier
     * @param eitSched EIT schedule indicator
     * @param eitPresent EIT present/following flag
     * @param runningStatus running status of the service
     * @param freeCA false if all the component streams of the service are not scrambled, true otherwise
     */
    DvbService(uint16_t id, bool eitSched, bool eitPresent, uint8_t runningStatus, bool freeCA)
        : m_serviceId(id),
          m_eitSchedFlag(eitSched),
          m_eitPfFlag(eitPresent),
          m_runningStatus(runningStatus),
          m_freeCA(freeCA)
    {
    }

    /**
     * Destructor
     */
    virtual ~DvbService()
    {
    }

    /**
     * Get service descriptors
     *
     * @return vector of service descriptors
     */
    const std::vector<MpegDescriptor>& getServiceDescriptors() const
    {
        return m_serviceDescriptors;
    }

    /**
     * Add a service descriptor
     *
     * @param descriptor the service descriptor
     */
    void addDescriptor(const MpegDescriptor& descriptor)
    {
        m_serviceDescriptors.push_back(descriptor);
    }

    /**
     * Add service descriptors
     *
     * @param descriptors dervice descriptors
     */
    void addDescriptors(const std::vector<MpegDescriptor>& descriptors)
    {
        m_serviceDescriptors.insert(m_serviceDescriptors.end(), descriptors.begin(), descriptors.end());
    }

    /**
     * Remove service descriptors
     */
    void removeDescriptors()
    {
        m_serviceDescriptors.clear();
    }

    /**
     * Check if EIT p/f flag is set
     *
     * @return true if flag is set, false otherwise
     */
    bool isEitPfFlagSet() const
    {
        return m_eitPfFlag;
    }

    /**
     * Check if EIT schedule flag is set
     *
     * @return true if flag is set, false otherwise
     */
    bool isEitSchedFlagSet() const
    {
        return m_eitSchedFlag;
    }

    /**
     * Check if service is scrambled
     *
     * @return true if one or more component streams is controlled by a CA system
     */
    bool isScrambled() const
    {
        return m_freeCA;
    }

    /**
     * Get the running status of the service
     *
     * @return running status
     *
     * @todo define possible running_status values (enum)
     * 0 undefined
     * 1 not running
     * 2 starts in a few seconds (e.g. for video recording)
     * 3 pausing
     * 4 running
     * 5 service off-air
     * 6 to 7 reserved for future use
     */
    uint8_t getRunningStatus() const
    {
        return m_runningStatus;
    }

    /**
     * Get the service identifier
     *
     * @return service identifier
     */
    uint16_t getServiceId() const
    {
        return m_serviceId;
    }

private:
    /**
     * Service descriptors
     */
    std::vector<MpegDescriptor> m_serviceDescriptors;

    /**
     * Service identifier
     */
    uint16_t m_serviceId;

    /**
     * EIT schedule flag
     */
    bool m_eitSchedFlag;

    /**
     * EIT p/f flag
     */
    bool m_eitPfFlag;

    /**
     * Running status
     */
    uint8_t m_runningStatus;

    /**
     * Free CA mode indicator
     */
    bool m_freeCA;
};

#endif /* SERVICE_H_ */
