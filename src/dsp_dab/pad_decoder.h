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
#include "tools.h"

#include <atomic>
#include <list>
#include <map>
#include <stdint.h>
#include <string.h>
#include <string>
#include <vector>


// --- DL_SEG -----------------------------------------------------------------
struct DL_SEG
{
  uint8_t prefix[2];
  std::vector<uint8_t> chars;

  bool Toggle() const { return prefix[0] & 0x80; }
  bool First() const { return prefix[0] & 0x40; }
  bool Last() const { return prefix[0] & 0x20; }
  int SegNum() const { return First() ? 0 : ((prefix[1] & 0x70) >> 4); }
};

typedef std::map<int, DL_SEG> dl_segs_t;


// --- DL_SEG_REASSEMBLER -----------------------------------------------------------------
struct DL_SEG_REASSEMBLER
{
  dl_segs_t dl_segs;
  std::vector<uint8_t> label_raw;

  bool AddSegment(DL_SEG& dl_seg);
  bool CheckForCompleteLabel();
  void Reset();
};


// --- DL_STATE -----------------------------------------------------------------
struct DL_STATE
{
  std::vector<uint8_t> raw;
  int charset;

  DL_STATE() { Reset(); }
  void Reset()
  {
    raw.clear();
    charset = -1;
  }
};


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


// --- CDataGroup -----------------------------------------------------------------
class CDataGroup
{
public:
  CDataGroup(size_t dg_size_max);
  virtual ~CDataGroup() = default;

  bool ProcessDataSubfield(bool start, const uint8_t* data, size_t len);

protected:
  std::vector<uint8_t> m_dgRaw;
  size_t m_dgSize;
  size_t m_dgSizeNeeded;

  virtual size_t GetInitialNeededSize() { return 0; }
  virtual bool DecodeDataGroup() = 0;
  bool EnsureDataGroupSize(size_t desired_dg_size);
  bool CheckCRC(size_t len);
  void Reset();
};


// --- CDGLIDecoder -----------------------------------------------------------------
class CDGLIDecoder : public CDataGroup
{
public:
  CDGLIDecoder() : CDataGroup(2 + CalcCRC::CRCLen) { Reset(); }

  void Reset();

  size_t GetDGLILen();

private:
  size_t m_dgliLength;

  size_t GetInitialNeededSize() override { return 2 + CalcCRC::CRCLen; } // DG len + CRC
  bool DecodeDataGroup() override;
};


// --- CDynamicLabelDecoder -----------------------------------------------------------------
class CDynamicLabelDecoder : public CDataGroup
{
public:
  CDynamicLabelDecoder() : CDataGroup(2 + 16 + CalcCRC::CRCLen) { Reset(); }

  void Reset();

  DL_STATE GetLabel() { return m_label; }

private:
  DL_SEG_REASSEMBLER m_dl_sr;
  DL_STATE m_label;

  size_t GetInitialNeededSize() override  { return 2 + CalcCRC::CRCLen; } // at least prefix + CRC
  bool DecodeDataGroup() override;
};


// --- CMOTDecoder -----------------------------------------------------------------
class CMOTDecoder : public CDataGroup
{
public:
  CMOTDecoder() : CDataGroup(16384) { Reset(); } // = 2^14

  void Reset();

  void SetLen(size_t mot_len) { m_motLength = mot_len; }

  std::vector<uint8_t> GetMOTDataGroup();

private:
  size_t m_motLength;

  size_t GetInitialNeededSize() override { return m_motLength; } // MOT len + CRC (or zero!)
  bool DecodeDataGroup() override;
};


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
