/**
 * @file artnetrdmresponder.h
 *
 */
/* Copyright (C) 2018-2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#ifndef ARTNETRDMRESPONDER_H_
#define ARTNETRDMRESPONDER_H_

#include <cstdint>

#include "artnetrdm.h"

#include "rdmdeviceresponder.h"
#include "rdmpersonality.h"
#include "rdmhandler.h"
#include "rdm.h"

#include "lightset.h"

class ArtNetRdmResponder: public RDMDeviceResponder, public ArtNetRdm {
public:
	ArtNetRdmResponder(RDMPersonality *pRDMPersonality, LightSet *pLightSet);
	~ArtNetRdmResponder() override;

	void Full(uint8_t nPortIndex) override;
	uint8_t GetUidCount(uint8_t nPortIndex) override;
	void Copy(uint8_t nPortIndex, uint8_t *) override;
	const uint8_t *Handler(uint8_t nPortIndex, const uint8_t *) override;

private:
	struct TRdmMessage *m_pRdmCommand;
	RDMHandler *m_RDMHandler;
};

#endif /* ARTNETRDMRESPONDER_H_ */
