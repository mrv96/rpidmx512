/**
 * @file main.cpp
 *
 */
/* Copyright (C) 2018-2021 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#include <stdio.h>
#include <stdint.h>
#include <assert.h>

#include "hardware.h"
#include "networkesp8266.h"
#include "ledblink.h"

#include "console.h"
#include "display.h"

#include "oscserverparms.h"
#include "oscserver.h"

// DMX output
#include "dmxparams.h"
#include "dmxsend.h"
#if defined(ORANGE_PI)
# include "storedmxsend.h"
#endif
#ifndef H3
// DMX real-time monitor
# include "dmxmonitor.h"
#endif
// Pixel Controller
#include "pixeldmxconfiguration.h"
#include "pixeltype.h"
#include "lightset.h"
#include "ws28xxdmxparams.h"
#include "ws28xxdmx.h"

#if defined(ORANGE_PI)
# include "storews28xxdmx.h"
#endif

#include "handler.h"

#if defined(ORANGE_PI)
 #include "spiflashinstall.h"
 #include "spiflashstore.h"
 #include "storeoscserver.h"
#endif

#include "software_version.h"

using namespace lightset;

constexpr char NETWORK_INIT[] = "Network init ...";
constexpr char BRIDGE_PARMAS[] = "Setting Bridge parameters ...";
constexpr char START_BRIDGE[] = "Starting the Bridge ...";
constexpr char BRIDGE_STARTED[] = "Bridge started";

extern "C" {

void notmain(void) {
	Hardware hw;
	NetworkESP8266 nw;
	LedBlink lb;
	Display display(DisplayType::SSD1306);

#ifndef H3
	DMXMonitor monitor;
#endif

#if defined (ORANGE_PI)
	SpiFlashInstall spiFlashInstall;
	SpiFlashStore spiFlashStore;

	StoreOscServer storeOscServer;
	StoreDmxSend storeDmxSend;
	StoreWS28xxDmx storeWS28xxDmx;

	OSCServerParams params((OSCServerParamsStore *)&storeOscServer);
#else
	OSCServerParams params;
#endif

	OscServer server;

	if (params.Load()) {
		params.Dump();
		params.Set(&server);
	}

	const auto tOutputType = params.GetOutputType();

	uint8_t nHwTextLength;
	printf("[V%s] %s Compiled on %s at %s\n", SOFTWARE_VERSION, hw.GetBoardName(nHwTextLength), __DATE__, __TIME__);

	console_puts("WiFi OSC Server ");
	console_set_fg_color(tOutputType == OutputType::DMX ? CONSOLE_GREEN : CONSOLE_WHITE);
	console_puts("DMX Output");
	console_set_fg_color(CONSOLE_WHITE);
#ifndef H3
	console_puts(" / ");
	console_set_fg_color(tOutputType == OutputType::MONITOR ? CONSOLE_GREEN : CONSOLE_WHITE);
	console_puts("Real-time DMX Monitor");
	console_set_fg_color(CONSOLE_WHITE);
#endif
	console_puts(" / ");
	console_set_fg_color(tOutputType == OutputType::SPI ? CONSOLE_GREEN : CONSOLE_WHITE);
	console_puts("Pixel controller {1 Universe}");
	console_set_fg_color(CONSOLE_WHITE);
#ifdef H3
	console_putc('\n');
#endif

	hw.SetLed(hardware::LedStatus::ON);

	console_set_top_row(3);

	console_status(CONSOLE_YELLOW, NETWORK_INIT);
	display.TextStatus(NETWORK_INIT);

	nw.Print();

	console_status(CONSOLE_YELLOW, BRIDGE_PARMAS);
	display.TextStatus(BRIDGE_PARMAS);

	DmxSend dmx;
	LightSet *pSpi = nullptr;

	if (tOutputType == OutputType::SPI) {
		PixelDmxConfiguration pixelDmxConfiguration;

#if defined (ORANGE_PI)
		WS28xxDmxParams ws28xxparms(new StoreWS28xxDmx);
#else
		WS28xxDmxParams ws28xxparms;
#endif

		if (ws28xxparms.Load()) {
			ws28xxparms.Set(&pixelDmxConfiguration);
			ws28xxparms.Dump();
		}

		// For the time being, just 1 Universe
		if (pixelDmxConfiguration.GetType() == pixel::Type::SK6812W) {
			if (pixelDmxConfiguration.GetCount() > 128) {
				pixelDmxConfiguration.SetCount(128);
			}
		} else {
			if (pixelDmxConfiguration.GetCount() > 170) {
				pixelDmxConfiguration.SetCount(170);
			}
		}

		auto *pPixelDmx = new WS28xxDmx(pixelDmxConfiguration);
		assert(pPixelDmx != nullptr);
		pSpi = pPixelDmx;

		display.Printf(7, "%s:%d G%d", PixelType::GetType(pixelDmxConfiguration.GetType()), pixelDmxConfiguration.GetCount(), pixelDmxConfiguration.GetGroupingCount());

		server.SetOutput(pSpi);
		server.SetOscServerHandler(new Handler(pPixelDmx));
	}
#ifndef H3
	else if (tOutputType == OutputType::MONITOR) {
		// There is support for HEX output only
		server.SetOutput(&monitor);
		monitor.Cls();
		console_set_top_row(20);
	}
#endif
	else {
#if defined (ORANGE_PI)
		DmxParams dmxparams((DmxParamsStore *)&storeDmxSend);
#else
		DmxParams dmxparams;
#endif
		if (dmxparams.Load()) {
			dmxparams.Dump();
			dmxparams.Set(&dmx);
		}

		server.SetOutput(&dmx);
	}

	server.Print();

	if (tOutputType == OutputType::SPI) {
		assert(pSpi != 0);
		pSpi->Print();
	} else 	if (tOutputType == OutputType::MONITOR) {
		printf(" Server ip-address    : " IPSTR "\n\n\n", IP2STR(nw.GetIp()));
	} else {
		dmx.Print();
	}

	for (uint8_t i = 0; i < 7 ; i++) {
		display.ClearLine(i);
	}

	display.Printf(1, "WiFi OSC %s", tOutputType == OutputType::SPI ? "Pixel" : "DMX");

	if (nw.GetOpmode() == WIFI_STA) {
		display.Printf(2, "S: %s", nw.GetSsid());
	} else {
		display.Printf(2, "AP (%s)\n", nw.IsApOpen() ? "Open" : "WPA_WPA2_PSK");
	}

	display.Printf(3, "IP: " IPSTR "", IP2STR(Network::Get()->GetIp()));

	if (nw.IsDhcpKnown()) {
		if (nw.IsDhcpUsed()) {
			display.PutString(" D");
		} else {
			display.PutString(" S");
		}
	}

	display.Printf(4, "In: %d", server.GetPortIncoming());
	display.Printf(5, "Out: %d", server.GetPortOutgoing());

	console_status(CONSOLE_YELLOW, START_BRIDGE);
	display.TextStatus(START_BRIDGE);

	server.Start();

	hw.SetLed(hardware::LedStatus::FLASH);

	console_status(CONSOLE_GREEN, BRIDGE_STARTED);
	display.TextStatus(BRIDGE_STARTED);

#if defined (ORANGE_PI)
	while (spiFlashStore.Flash())
		;
#endif

	hw.WatchdogInit();

	for (;;) {
		hw.WatchdogFeed();
		server.Run();
		lb.Run();
	}
}

}
