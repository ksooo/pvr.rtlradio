/*
 *  Copyright (C) 2024 Team Kodi (https://kodi.tv)
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSE.md for more information.
 */

#pragma once

#include "muxscanner.h"
#include "props.h"
#include "utils/scalar_condition.h"

#include <atomic>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

class rtldevice;
class CAutoGainControl;

class CChannelScan
{
public:
  static std::unique_ptr<CChannelScan> Create(std::unique_ptr<rtldevice> device,
                                              const struct tunerprops& tunerprops,
                                              const std::vector<struct channelprops>& channelprops);
  virtual ~CChannelScan();

  bool Start();

  void processData(uint8_t const* buffer, size_t count);

private:
  CChannelScan() = delete;
  CChannelScan(const CChannelScan&) = delete;
  CChannelScan& operator=(const CChannelScan&) = delete;

  CChannelScan(std::unique_ptr<rtldevice> device,
               const struct tunerprops& tunerprops,
               const std::vector<struct channelprops>& channelprops);

  void ScanNextChannel();

  // Updates the state of the multiplex information
  void process_mux_data(const struct muxscanner::multiplex& muxdata);
  static void mux_data_cb(const struct muxscanner::multiplex& muxdata, void* ctx);

  // Worker thread procedure used to pump data into the mux scanner and agc
  void worker(scalar_condition<bool>& started);

  static void async_read_cb(uint8_t const* buffer, size_t count, void* ctx);

  // Worker thread procedure used to drive next channel scan and progress dialog
  void control(scalar_condition<bool>& started);

  std::unique_ptr<rtldevice> m_device; // Device instance
  struct tunerprops m_tunerprops = {}; // Tuner properties
  std::vector<struct channelprops> m_channelprops; // Channel properties
  std::vector<struct channelprops>::iterator m_channel = m_channelprops.begin(); // Current channel

  std::unique_ptr<CAutoGainControl> m_agc; // AGC

  std::unique_ptr<muxscanner> m_muxscanner; // Multiplex scanner instance
  struct muxscanner::multiplex m_muxdata = {}; // Multiplex properties
  std::mutex m_muxdatalock; // Multiplex properties lock
  std::condition_variable m_muxcv; // Event condition variable

  std::thread m_worker; // Worker thread
  std::thread m_control; // Control thread, drives next channel scan and progress dialog
  std::atomic<bool> m_running{false};
};
