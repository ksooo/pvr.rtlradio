/*
    DABlin - capital DAB experience
    Copyright (C) 2015-2019 Stefan Pöschel

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include "mot_manager.h"
#include "pad_decoder_dgli.h"
#include "pad_decoder_dynamiclabelsegment.h"
#include "pad_decoder_mot.h"
#include "tools.h"

#include <atomic>
#include <list>
#include <map>
#include <stdint.h>
#include <string.h>
#include <string>
#include <vector>


// --- XPAD_CI -----------------------------------------------------------------
struct XPAD_CI
{
  size_t len;
  int type;

  static const size_t lens[];

  XPAD_CI() { Reset(); }
  XPAD_CI(uint8_t ci_raw)
  {
    len = lens[ci_raw >> 5];
    type = ci_raw & 0x1F;
  }
  XPAD_CI(size_t len, int type) : len(len), type(type) {}

  void Reset()
  {
    len = 0;
    type = -1;
  }
};

typedef std::list<XPAD_CI> xpad_cis_t;


// --- IPADDecoderObserver -----------------------------------------------------------------
class IPADDecoderObserver
{
public:
  virtual ~IPADDecoderObserver() = default;

  virtual void PADChangeDynamicLabel(const DL_STATE& dl) = 0;
  virtual void PADChangeSlide(const MOT_FILE& slide) = 0;

  virtual void PADLengthError(size_t announced_xpad_len, size_t xpad_len) = 0;
};


// --- CPADDecoder -----------------------------------------------------------------
class CPADDecoder
{
public:
  CPADDecoder(IPADDecoderObserver* observer, bool loose)
    : m_observer(observer), m_loose(loose), m_MOTAppType(-1)
  {
  }

  void SetMOTAppType(int type) { m_MOTAppType = type; }
  void Process(const uint8_t* xpad_data,
               size_t xpad_len,
               bool exact_xpad_len,
               const uint8_t* fpad_data);
  void Reset();

private:
  IPADDecoderObserver* m_observer;
  bool m_loose;
  std::atomic<int> m_MOTAppType;

  uint8_t m_xpad[196]; // longest possible X-PAD
  XPAD_CI m_lastXPAD_CI;

  CDynamicLabelDecoder m_dlDecoder;
  CDGLIDecoder m_DGLIDecoder;
  CMOTDecoder m_MOTDecoder;
  MOTManager m_MOTManager;
};
