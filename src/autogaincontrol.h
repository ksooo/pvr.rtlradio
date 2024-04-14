/*
 *  Copyright (C) 2024 Team Kodi (https://kodi.tv)
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSE.md for more information.
 */

#pragma once

#include "utils/scalar_condition.h"

#include <atomic>
#include <mutex>
#include <thread>

class rtldevice;

class CAutoGainControl
{
public:
  explicit CAutoGainControl(rtldevice& device);
  virtual ~CAutoGainControl();

  void Update(uint8_t const* buffer, size_t count);

private:
  CAutoGainControl() = delete;
  CAutoGainControl(const CAutoGainControl&) = delete;
  CAutoGainControl& operator=(const CAutoGainControl&) = delete;

  void worker(scalar_condition<bool>& started);

  std::mutex m_mutex;
  rtldevice& m_device;
  std::atomic<bool> m_running{false};
  std::thread m_agcThread;
  uint8_t m_minAmplitude{255};
  uint8_t m_maxAmplitude{0};
};
