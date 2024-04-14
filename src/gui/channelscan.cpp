/*
 *  Copyright (C) 2024 Team Kodi (https://kodi.tv)
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSE.md for more information.
 */

#include "channelscan.h"

#include "autogaincontrol.h"
#include "dabmuxscanner.h"
#include "hdmuxscanner.h"
#include "rtldevice.h"
#include "exception_control/string_exception.h"
#include "utils/value_size_defines.h"

#include <kodi/gui/dialogs/Progress.h>

#include <chrono>

std::unique_ptr<CChannelScan> CChannelScan::Create(std::unique_ptr<rtldevice> device,
                                                   const struct tunerprops& tunerprops,
                                                   const std::vector<struct channelprops>& channelprops)
{
  return std::unique_ptr<CChannelScan>(new CChannelScan(std::move(device),
                                                        tunerprops,
                                                        channelprops));
}

CChannelScan::CChannelScan(std::unique_ptr<rtldevice> device,
                           const struct tunerprops& tunerprops,
                           const std::vector<struct channelprops>& channelprops)
  : m_device(std::move(device)),
    m_tunerprops(tunerprops),
    m_channelprops(channelprops)
{
  assert(m_device);
}

CChannelScan::~CChannelScan()
{
  m_running = false;
  if (m_control.joinable())
    m_control.join(); // Wait for thread to exit
}

void CChannelScan::ScanNextChannel()
{
  m_agc.reset();
  m_muxscanner.reset();
  if (m_device)
    m_device->cancel_async(); // Cancel any async read operations
  if (m_worker.joinable())
    m_worker.join(); // Wait for thread to exit
  m_muxdata = {};

  if (m_channel != m_channelprops.end())
  {
    auto channel = *m_channel;

    struct signalprops signalprops = {}; // Signal properties

    // Analog FM
    //
    if (channel.modulation == modulation::fm)
    {
      signalprops.samplerate = 1600 KHz;
      signalprops.bandwidth = 220 KHz;
      signalprops.lowcut = -103 KHz;
      signalprops.highcut = 103 KHz;

      // Analog signals require a DC offset to be applied to prevent a natural
      // spike from occurring at the center frequency on many RTL-SDR devices
      signalprops.offset = (signalprops.samplerate / 4);
    }

    // HD Radio
    //
    else if (channel.modulation == modulation::hd)
    {
      signalprops.samplerate = 1488375;
      signalprops.bandwidth = 440 KHz;
      signalprops.lowcut = -204 KHz;
      signalprops.highcut = 204 KHz;
      signalprops.offset = 0;
    }

    // DAB Ensemble
    //
    else if (channel.modulation == modulation::dab)
    {
      signalprops.samplerate = 2048 KHz;
      signalprops.bandwidth = 1712 KHz;
      signalprops.lowcut = -780 KHz;
      signalprops.highcut = 780 KHz;
      signalprops.offset = 0;
    }

    // Weather Radio
    //
    else if (channel.modulation == modulation::wx)
    {
      signalprops.samplerate = 1600 KHz;
      signalprops.bandwidth = 200 KHz;
      signalprops.lowcut = -8 KHz;
      signalprops.highcut = 8 KHz;

      // Analog signals require a DC offset to be applied to prevent a natural
      // spike from occurring at the center frequency on many RTL-SDR devices
      signalprops.offset = (signalprops.samplerate / 4);
    }

    else
      throw string_exception("unknown channel modulation");

    // Set the device to match the channel properties
    m_device->set_center_frequency(channel.frequency + signalprops.offset);
    m_device->set_frequency_correction(m_tunerprops.freqcorrection + channel.freqcorrection);
    m_device->set_sample_rate(signalprops.samplerate);

    if (channel.autogain)
    {
      m_agc = std::make_unique<CAutoGainControl>(*m_device);
    }
    else
    {
      m_agc.reset();
      m_device->set_automatic_gain_control(false);
      m_device->set_gain(channel.manualgain);
    }

    // Create the multiplex scanner instance if applicable to the modulation
    if (channel.modulation == modulation::hd)
      m_muxscanner =
          hdmuxscanner::create(signalprops.samplerate, channel.frequency,
                               std::bind(&CChannelScan::mux_data, this, std::placeholders::_1));
    else if (channel.modulation == modulation::dab)
      m_muxscanner =
          dabmuxscanner::create(signalprops.samplerate,
                                std::bind(&CChannelScan::mux_data, this, std::placeholders::_1));

    // Create a worker thread on which to pump data into agc and mux scanner
    scalar_condition<bool> started{false};
    m_worker = std::thread(&CChannelScan::worker, this, std::ref(started));
    started.wait_until_equals(true);
  }
}

bool CChannelScan::Start()
{
  // fragen, ob alle channels gelöscht werden sollen oder aktualisiert

  // über alle channelprops gehen, scan mit timeout starten
  // muxscanner wie channelsettings verwenden

  // progress anzeigen scanning ensemble: 5C channels found: 12


  // ? autom. alle gefundenen channels aufnehmen/aktualisieren oder selektionsdialog
  // => dazu müsste man subchannelprops durchschleusen aus addon.cpp channelscan_dab

  // Create a worker thread on which to drive next channel scan and progress dialog
  scalar_condition<bool> started{false};
  m_control = std::thread(&CChannelScan::control, this, std::ref(started));
  started.wait_until_equals(true);

  return true;
}

void CChannelScan::mux_data(const struct muxscanner::multiplex& muxdata)
{
//  std::unique_lock<std::mutex> lock(m_muxdatalock);

  m_muxdata = muxdata; // Copy the updated mutex information

  if (m_muxdata.signalpresent)
    m_muxcv.notify_all();
  else if (m_muxdata.signaltimeout)
    m_muxcv.notify_all();
}

void CChannelScan::worker(scalar_condition<bool>& started)
{
  assert(m_device);

  // Asynchronous read callback function for the RTL-SDR device
  auto read_callback_func = [&](uint8_t const* buffer, size_t count) -> void
  {
    if (m_muxscanner)
      m_muxscanner->inputsamples(buffer, count);

    if (m_agc)
      m_agc->Update(buffer, count);
  };

  // Begin streaming from the device and inform the caller that the thread is running
  m_device->begin_stream();
  started = true;

  // Continuously read data from the device until cancel_async() has been called
  try
  {
    m_device->read_async(read_callback_func, static_cast<uint32_t>(32 KiB));
  }
  catch (...)
  {
//    m_worker_exception = std::current_exception();
  }

  m_muxcv.notify_all();
}

void CChannelScan::control(scalar_condition<bool>& started)
{
  assert(m_device);

  const auto progress = std::make_unique<kodi::gui::dialogs::CProgress>();
  progress->SetHeading("Channel scan"); //! @todo localize
  progress->SetLine(0, "Scanning emsemble: " + (*m_channel).name); //! @todo localize
  progress->SetCanCancel(true);
  progress->ShowProgressBar(true);

  m_running = true;
  started = true;

  progress->Open();

  int curr = 1;
  const int total = m_channelprops.size();

  // Scan first channel
  ScanNextChannel();

  int channels = 0;

  while (m_channel != m_channelprops.end() && !progress->IsCanceled() && m_running)
  {
    progress->SetLine(0, "Scanning emsemble: " + (*m_channel).name); //! @todo localize
    progress->SetPercentage((curr * 100) / total);

    if (m_muxdata.signalpresent || m_muxdata.signaltimeout)
    {
      if (m_muxdata.signalpresent)
      {
        for (int i = 0; i < 2000; ++i)
        {
          if (!progress->IsCanceled() && m_running)
            std::this_thread::sleep_for(std::chrono::milliseconds(10)); // wait for subchannels
        }
      }

      channels += m_muxdata.subchannels.size();
      progress->SetLine(1, "Total subchannels found: " + std::to_string(channels)); //! @todo localize Channels found: %i

      curr++;
      m_channel++;
      ScanNextChannel();
    }
    else
    {
      std::this_thread::sleep_for(std::chrono::milliseconds(10)); // wait for signal present / timeout
    }
  }

  // Cleanup

  m_agc.reset();
  m_muxscanner.reset();

  if (m_device)
    m_device->cancel_async(); // Cancel any async read operations

  if (m_worker.joinable())
    m_worker.join(); // Wait for thread to exit

  m_device.reset();
}
