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
