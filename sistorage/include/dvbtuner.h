// Copyright (c) 2014 ARRIS Group, Inc. All rights reserved.
//
// This program is confidential and proprietary to ARRIS Group, Inc. (ARRIS),
// and may not be copied, reproduced, modified, disclosed to others, published
// or used, in whole or in part, without the express prior written permission
// of ARRIS.

#ifndef _DVBTUNER_H_
#define _DVBTUNER_H_

// C system includes
#include <stdint.h>

// C++ system includes
#include <memory>

// Other libraries' includes
// Project's includes

/**
 * Dvb Modulation enumeration
 */
enum DvbModulationMode
{
    DVB_MODULATION_UNKNOWN=0,
    DVB_MODULATION_QPSK,
    DVB_MODULATION_BPSK,
    DVB_MODULATION_OQPSK,
    DVB_MODULATION_VSB8,
    DVB_MODULATION_VSB16,
    DVB_MODULATION_QAM16,
    DVB_MODULATION_QAM32,
    DVB_MODULATION_QAM64,
    DVB_MODULATION_QAM80,
    DVB_MODULATION_QAM96,
    DVB_MODULATION_QAM112,
    DVB_MODULATION_QAM128,
    DVB_MODULATION_QAM160,
    DVB_MODULATION_QAM192,
    DVB_MODULATION_QAM224,
    DVB_MODULATION_QAM256,
    DVB_MODULATION_QAM320,
    DVB_MODULATION_QAM384,
    DVB_MODULATION_QAM448,
    DVB_MODULATION_QAM512,
    DVB_MODULATION_QAM640,
    DVB_MODULATION_QAM768,
    DVB_MODULATION_QAM896,
    DVB_MODULATION_QAM1024,
    DVB_MODULATION_QAM_NTSC // for analog mode
};


/**
 * DVB Tuner class. Used by DvbSiStorage to implement the DVB Scan feature.
 */
class DvbTuner
{
public:
    /**
     * createTuner - factory method to create platform specific tuner
     */
    static std::shared_ptr<DvbTuner>  createTuner();

    /**
     * Destructor
     */
    ~DvbTuner();

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

protected:
    /**
     * Constructor
     */
    DvbTuner();

private:
    /**
     * Copy constructor
     */
    DvbTuner(const DvbTuner& other)  = delete;

    /**
     * Assignment operator
     */
    DvbTuner& operator=(const DvbTuner&)  = delete;
};

#endif /* _DVBTUNER_H_ */
