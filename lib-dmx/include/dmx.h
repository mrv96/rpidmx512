/**
 * @file dmx.h
 *
 */
/* Copyright (C) 2015-2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#ifndef DMX_H_
#define DMX_H_

#include <stdint.h>
#include <stdbool.h>

#define DMX_MAX_OUT		4U
#define DMX_MAX_IN		4U

#define DMX_DATA_BUFFER_SIZE					516									///< including SC, aligned 4
#define DMX_DATA_BUFFER_INDEX_ENTRIES			(1 << 1)							///<
#define DMX_DATA_BUFFER_INDEX_MASK 				(DMX_DATA_BUFFER_INDEX_ENTRIES - 1)	///<

#define DMX_TRANSMIT_BREAK_TIME_MIN				92U		///< 92 us
#define DMX_TRANSMIT_BREAK_TIME_TYPICAL			176U	///< 176 us
#define DMX_TRANSMIT_MAB_TIME_MIN				12U		///< 12 us
#define DMX_TRANSMIT_MAB_TIME_MAX				1000000	///< 1s
#define DMX_TRANSMIT_REFRESH_RATE_DEFAULT		40U		///< 40 Hz
#define DMX_TRANSMIT_PERIOD_DEFAULT				(1000000U / DMX_TRANSMIT_REFRESH_RATE_DEFAULT)	///< 25000 us
#define DMX_TRANSMIT_BREAK_TO_BREAK_TIME_MIN	1204U	///< us

#define DMX_MIN_SLOT_VALUE 						0		///< The minimum value a DMX512 slot can take.
#define DMX_MAX_SLOT_VALUE 						255		///< The maximum value a DMX512 slot can take.
#define DMX512_START_CODE						0		///< The start code for DMX512 data. This is often referred to as NSC for "Null Start Code".

enum TDmxChannels {
	DMX_MAX_CHANNELS = 512
};

typedef enum {
	DMX_PORT_DIRECTION_OUTP,
	DMX_PORT_DIRECTION_INP
} _dmx_port_direction;

struct _dmx_statistics {
	uint32_t mark_after_break;
	uint32_t slots_in_packet;
	uint32_t break_to_break;
	uint32_t slot_to_slot;
};

struct _dmx_data {
	uint8_t data[DMX_DATA_BUFFER_SIZE];
	struct _dmx_statistics statistics;
};

struct _total_statistics {
	uint32_t dmx_packets;
	uint32_t rdm_packets;
};

#ifdef __cplusplus
extern "C" {
#endif

extern void dmx_init_set_gpiopin(uint8_t);
extern void dmx_init(void);

extern void dmx_set_send_data(const uint8_t *, uint16_t);
extern void dmx_set_send_data_without_sc(const uint8_t *, uint16_t);
extern void dmx_clear_data(void);
extern void dmx_set_port_direction(_dmx_port_direction, bool);
extern _dmx_port_direction dmx_get_port_direction(void);
extern void dmx_data_send(const uint8_t *, uint16_t);
extern /*@shared@*/const /*@null@*/uint8_t *dmx_get_available(void) __attribute__((assume_aligned(4)));
extern /*@shared@*/const uint8_t *dmx_get_current_data(void) __attribute__((assume_aligned(4)));
extern /*@shared@*/const uint8_t *dmx_is_data_changed(void) __attribute__((assume_aligned(4)));
extern uint32_t dmx_get_output_break_time(void);
extern void dmx_set_output_break_time(uint32_t);
extern uint32_t dmx_get_output_mab_time(void);
extern void dmx_set_output_mab_time(uint32_t);
extern void dmx_reset_total_statistics(void);
extern /*@shared@*/const volatile struct _total_statistics *dmx_get_total_statistics(void) __attribute__((assume_aligned(4)));
extern uint32_t dmx_get_updates_per_seconde(void);
extern uint32_t dmx_get_send_data_length(void);
extern uint32_t dmx_get_output_period(void);
extern void dmx_set_output_period(uint32_t);
extern /*@shared@*/const /*@null@*/uint8_t *rdm_get_available(void) __attribute__((assume_aligned(4)));
extern /*@shared@*/const uint8_t *rdm_get_current_data(void) __attribute__((assume_aligned(4)));
extern uint32_t rdm_get_data_receive_end(void);

#ifdef __cplusplus
}
#endif

/*
 * C++ only
 */

#ifdef __cplusplus

#include "gpio.h"

enum TDmxRdmPortDirection {
	DMXRDM_PORT_DIRECTION_OUTP = DMX_PORT_DIRECTION_OUTP,
	DMXRDM_PORT_DIRECTION_INP = DMX_PORT_DIRECTION_INP
};

struct TDmxStatistics {
	uint32_t MarkAfterBreak;
	uint32_t SlotsInPacket;
	uint32_t BreakToBreak;
	uint32_t SlotToSlot;
};

struct TDmxData {
	uint8_t Data[DMX_DATA_BUFFER_SIZE];
	struct TDmxStatistics Statistics;
};

class DmxSet {
public:
	DmxSet();
	virtual ~DmxSet() {
	}

	virtual void SetPortDirection(uint32_t nPort, TDmxRdmPortDirection tPortDirection, bool bEnableData)=0;

	virtual void RdmSendRaw(uint32_t nPort, const uint8_t *pRdmData, uint16_t nLength)=0;

	virtual const uint8_t *RdmReceive(uint32_t nPort)=0;
	virtual const uint8_t *RdmReceiveTimeOut(uint32_t nPort, uint32_t nTimeOut)=0;

	static DmxSet* Get() {
		return s_pThis;
	}

private:
	static DmxSet *s_pThis;
};

class Dmx: public DmxSet {
public:
	Dmx(uint8_t nGpioPin = GPIO_DMX_DATA_DIRECTION, bool DoInit = true);

	void SetPortDirection(uint32_t nPort, TDmxRdmPortDirection tPortDirection, bool bEnableData = false) override;

	void RdmSendRaw(uint32_t nPort, const uint8_t *pRdmData, uint16_t nLength) override;

	const uint8_t *RdmReceive(uint32_t nPort) override;
	const uint8_t *RdmReceiveTimeOut(uint32_t nPort, uint32_t nTimeOut) override;

	void Init();

	void SetSendData(const uint8_t *pData, uint16_t nLength) {
		dmx_set_send_data(pData, nLength);
	}

	uint32_t GetUpdatesPerSecond() {
		return dmx_get_updates_per_seconde();
	}

	void ClearData() {
		dmx_clear_data();
	}

	const uint8_t* GetDmxCurrentData() {
		return dmx_get_current_data();
	}

	const uint8_t* GetDmxAvailable() {
		return dmx_get_available();
	}

	void SetDmxBreakTime(uint32_t nBreakTime) {
		dmx_set_output_break_time(nBreakTime);
	}

	uint32_t GetDmxBreakTime() {
		return dmx_get_output_break_time();
	}

	void SetDmxMabTime(uint32_t nMabTime) {
		dmx_set_output_mab_time(nMabTime);
	}

	uint32_t GetDmxMabTime() {
		return dmx_get_output_mab_time();
	}

	void SetDmxPeriodTime(uint32_t nPeriodTime) {
		dmx_set_output_period(nPeriodTime);
	}

	uint32_t GetDmxPeriodTime() {
		return dmx_get_output_period();
	}

	static Dmx* Get() {
		return s_pThis;
	}

private:
	bool m_IsInitDone;

	static Dmx *s_pThis;
};

#endif

#endif /* DMX_H_ */
