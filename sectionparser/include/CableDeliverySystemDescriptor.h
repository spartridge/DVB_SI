// Copyright (c) 2014 ARRIS Group, Inc. All rights reserved.
//
// This program is confidential and proprietary to ARRIS Group, Inc. (ARRIS),
// and may not be copied, reproduced, modified, disclosed to others, published
// or used, in whole or in part, without the express prior written permission
// of ARRIS.

#ifndef CABLEDELIVERYSYSTEMDESCRIPTOR_H_
#define CABLEDELIVERYSYSTEMDESCRIPTOR_H_

// C++ system includes
#include <string>

// Other libraries' includes

// Project's includes
#include "MpegDescriptor.h"
#include "DvbUtils.h"

/**
 * Cable delivery system descriptor class
 */
class CableDeliverySystemDescriptor: public MpegDescriptor
{
public:
    /**
     * Constructor
     *
     * @param desc mpeg descriptor
     */
    CableDeliverySystemDescriptor(const MpegDescriptor& desc)
        : MpegDescriptor(desc)
    {
    }

    /**
     * Get the frequency as a binary coded decimal
     *
     * @return frequency (BCD)
     */
    uint32_t getFrequencyBcd()
    {
        return ((m_data[0] << 24) | (m_data[1] << 16) | (m_data[2] << 8) | (m_data[3]));
    }

    /**
     * Get the frequency
     *
     * @return frequency
     */
    uint32_t getFrequency()
    {
        return BcdToDec((m_data[0] << 8) | m_data[1]) * 1000000 + BcdToDec((m_data[2] << 8) | m_data[3]) * 100;
    }

    /**
     * Modulation scheme enumeration
     */
    enum Modulation
    {
        NOT_DEFINED = 0,//!< NOT_DEFINED
        QAM16,          //!< QAM16
        QAM32,          //!< QAM32
        QAM64,          //!< QAM64
        QAM128,         //!< QAM128
        QAM256,         //!< QAM256
    };

    /**
     * Get the modulation
     *
     * @return modulation
     */
    CableDeliverySystemDescriptor::Modulation getModulation()
    {
        return static_cast<CableDeliverySystemDescriptor::Modulation>(m_data[6]);
    }

    /**
     * Get the symbol rate as a binary coded decimal
     *
     * @return symbol rate (BCD)
     */
    uint32_t getSymbolRateBcd()
    {
        return ((m_data[7] << 20) | (m_data[8] << 12) | (m_data[9] << 4) | (m_data[10] >> 4));
    }

    /**
     * Get the symbol rate
     *
     * @return symbol rate
     */
    uint32_t getSymbolRate()
    {
        return (BcdByteToDec(m_data[7])*100000 + BcdByteToDec(m_data[8])*1000 +
                BcdByteToDec(m_data[9])*10 + BcdByteToDec(m_data[10]>>4))*100;
    }

    /**
     * Get the inner FEC scheme
     *
     * @return FEC_inner
     */
    uint8_t  getFecInner()
    {
        return (m_data[10] & 0xf);
    }

    /**
     * Get the outer FEC scheme
     *
     * @return FEC_outer
     */
    uint8_t  getFecOuter()
    {
        return (m_data[5] & 0xf);
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

#endif /* CABLEDELIVERYSYSTEMDESCRIPTOR_H_ */
