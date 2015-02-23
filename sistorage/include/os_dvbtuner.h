// Copyright (c) 2014 ARRIS Group, Inc. All rights reserved.
//
// This program is confidential and proprietary to ARRIS Group, Inc. (ARRIS),
// and may not be copied, reproduced, modified, disclosed to others, published
// or used, in whole or in part, without the express prior written permission
// of ARRIS.

#ifndef _OS_DVBTUNER_H_
#define _OS_DVBTUNER_H_

// Other libraries' includes

// Project's includes

// Forward declarations


/**
 * RMF DVB Tuner class. Used by DvbSiStorage to implement the DVB Scan feature.
 */
class os_DvbTuner : public DvbTuner
{
public:
    /**
     * Constructor
     */
    os_DvbTuner();

    /**
     * Destructor
     */
    ~os_DvbTuner();

    /**
     * Tune to given frequency using given modulation and symbol rate
     *
     * @param freq frequency
     * @param mod modulation
     * @param symbol_rate symbol rate
     * @return int32_t 0 if successful
     */
    virtual int32_t tune(uint32_t freq, DvbModulationMode mod, uint32_t symbol_rate);

    /**
     * Untune a previously tuned tuner
     * Note: It destroys the pipeline(QAMSrc) and frees the resources
     */
    virtual void untune();

private:
    /**
     * Copy constructor
     */
    os_DvbTuner(const os_DvbTuner& other)  = delete;

    /**
     * Assignment operator
     */
    os_DvbTuner& operator=(const os_DvbTuner&)  = delete;
};

#endif /* _OS_DVBTUNER_H_ */
