/*
 *  Copyright (C) 2024 Team Kodi (https://kodi.tv)
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSE.md for more information.
 */

#include "autogaincontrol.h"

#include "rtldevice.h"

#include <chrono>
#include <cmath>

CAutoGainControl::CAutoGainControl(rtldevice& device)
  : m_device(device)
{
  // disable device hardware AGC
  m_device.set_automatic_gain_control(false);

  scalar_condition<bool> started{false};
  m_agcThread = std::thread(&CAutoGainControl::worker, this, std::ref(started));
  started.wait_until_equals(true);
}

CAutoGainControl::~CAutoGainControl()
{
  m_running = false;
  if (m_agcThread.joinable())
    m_agcThread.join(); // Wait for thread
}

void CAutoGainControl::worker(scalar_condition<bool>& started)
{
  uint8_t minAmplitude{255};
  uint8_t maxAmplitude{0};
  int currentGain{0};
  int currentGainIndex{0};
  int newGain{0};

  std::vector<int> gains;
  m_device.get_valid_gains(gains);

  m_running = true;
  started = true;

  while (m_running)
  {
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    {
      std::unique_lock<std::mutex> lock(m_mutex);
      minAmplitude = m_minAmplitude;
      maxAmplitude = m_maxAmplitude;
    }

    if (!m_running)
      break;

    // Check for overloading
    if (minAmplitude == 0 || maxAmplitude == 255)
    {
      // We have to decrease the gain
      if (currentGainIndex > 0 && static_cast<size_t>(currentGainIndex - 1) < gains.size())
      {
        currentGainIndex--;
        newGain = gains[currentGainIndex];
      }
    }
    else
    {
      if (currentGainIndex < (static_cast<ssize_t>(gains.size()) - 1))
      {
        // Calc if a gain increase overloads the device. Calc it from the gain values
        const int gain = gains[currentGainIndex + 1];
        const float deltaGain = (static_cast<float>(gain) / 10) - (static_cast<float>(currentGain) / 10);
        const float linGain = std::pow(10, deltaGain / 20);

        const int newMaxValue = static_cast<float>(maxAmplitude) * linGain;
        const int newMinValue = static_cast<float>(minAmplitude) / linGain;

        // We have to increase the gain
        if (newMinValue >= 0 && newMaxValue <= 255 &&
            static_cast<size_t>(currentGainIndex + 1) < gains.size())
        {
          currentGainIndex++;
          newGain = gain;
        }
      }
    }

    if (newGain != currentGain && m_running)
    {
      currentGain = newGain;
      m_device.set_gain(currentGain);
    }
  }
}

void CAutoGainControl::Update(uint8_t const* buffer, size_t count)
{
  std::unique_lock<std::mutex> lock(m_mutex);

  // Check if device is overloaded
  m_minAmplitude = 255;
  m_maxAmplitude = 0;

  for (uint32_t i = 0; i < count; ++i)
  {
    if (m_minAmplitude > buffer[i])
      m_minAmplitude = buffer[i];

    if (m_maxAmplitude < buffer[i])
      m_maxAmplitude = buffer[i];
  }
}
