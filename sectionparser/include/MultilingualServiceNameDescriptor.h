// Copyright (c) 2014 ARRIS Group, Inc. All rights reserved.
//
// This program is confidential and proprietary to ARRIS Group, Inc. (ARRIS),
// and may not be copied, reproduced, modified, disclosed to others, published
// or used, in whole or in part, without the express prior written permission
// of ARRIS.

#ifndef MULTILINGUALSERVICENAMEDESCRIPTOR_H_
#define MULTILINGUALSERVICENAMEDESCRIPTOR_H_

// C++ system includes
#include <vector>

// Other libraries' includes

// Project's includes
#include "MpegDescriptor.h"
#include "DvbUtils.h"

// Syntax of the descriptor from ETSI EN 300 468
//
//multilingual_service_name_descriptor(){
//    descriptor_tag                         8 uimsbf
//    descriptor_length                      8 uimsbf
//    for (i=0;i<N;i++) {
//        ISO_639_language_code             24 bslbf
//        service_provider_name_length       8 uimsbf
//        for (j=0;j<N;j++){
//            char                           8 uimsbf
//        }
//        service_name_length                8 uimsbf
//        for (j=0;j<N;j++){
//            char                           8 uimsbf
//        }
//    }
//}

/**
 * Multilingual service name descriptor class
 */
class MultilingualServiceNameDescriptor: public MpegDescriptor
{
public:
    /**
     * Constructor
     *
     * @param desc mpeg descriptor
     */
    MultilingualServiceNameDescriptor(const MpegDescriptor& desc)
        : MpegDescriptor(desc)
    {
        const uint8_t* p = m_data.data();
        if(p)
        {
            const uint8_t* end = p + m_data.size();
            while(p + 4 < end)
            {
                m_items.push_back(p);
                // language code
                p += 3;
                // provider name length and provider name
                p += p[0] + 1;
                // service name length and service name
                p += p[0] + 1;
            }
        }
    }

    /**
     * Get the count
     *
     * @return count
     */
    uint8_t getCount() const
    {
        return m_items.size();
    }

    /**
     * Get the language code
     *
     * @param n
     * @return language_code
     */
    std::string getLanguageCode(uint8_t n) const
    {
        return std::string((const char*)m_items[n], 3);
    }

    /**
     * Get the length of the service provider name
     *
     * @param n
     * @return service_provider_name_length
     */
    uint8_t getServiceProviderNameLength(uint8_t n) const
    {
        return *(m_items[n] + 3);
    }

    /**
     * Get the service provider name
     *
     * @param n
     * @return service_provider_name
     */
    std::string getServiceProviderName(uint8_t n) const
    {
        return DecodeText(m_items[n] + 4, getServiceProviderNameLength(n));
    }

    /**
     * Get the length of the service name
     *
     * @param n
     * @return service_name_length
     */
    uint8_t getServiceNameLength(uint8_t n) const
    {
        return *(m_items[n] + 4 + getServiceProviderNameLength(n));
    }

    /**
     * Get the service name
     *
     * @param n
     * @return service_name
     */
    std::string getServiceName(uint8_t n) const
    {
        return DecodeText(m_items[n] + 5 + getServiceProviderNameLength(n), getServiceNameLength(n));
    }

private:
    /**
     * List of items
     */
    std::vector<const uint8_t*> m_items;
};

#endif /* MULTILINGUALSERVICENAMEDESCRIPTOR_H_ */
