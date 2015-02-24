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
