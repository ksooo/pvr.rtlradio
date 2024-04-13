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

#include "pad_decoder_dgli.h"
#include "pad_decoder_dynamiclabelsegment.h"
#include "pad_decoder_mot.h"

#include "dsp_dab/tools.h"

#include <atomic>
#include <list>

/*!
 * @brief This 2-bit field shall indicate the content of the Byte L-1 data field
 *
 * See ETSI EN 300 401 V2.1.1 (2017-01) clause 7.4.1.
 */
enum class F_PADType : uint8_t
{
  //! In ref. to ETSI EN 300 401 V2.1.1 the only value used.
  Type0 = 0b00,

  //! Reserved value
  Type1Reserved = 0b01,

  //! Reserved value
  Type2Reserved = 0b10,

  //! Reserved value
  Type3Reserved = 0b11,
};

/*!
 * @brief This 2-bit field shall indicate the presence and length of the X-PAD field.
 *
 * See ETSI EN 300 401 V2.1.1 (2017-01) clause 7.4.1.
 */
enum class X_PADInd : uint8_t
{
  //! No X-PAD
  noData = 0b00,

  //! Short X-PAD
  shortData = 0b01,

  //! Variable size X-PAD
  variableSizeData = 0b10,

  //! Reserved for future use
  reserved = 0b11,
};

/*!
 * @brief This 1 - bit flag shall signal whether the X-PAD field in the current DAB.
 * audio frame includes at least one contents indicator.
 *
 * See ETSI EN 300 401 V2.1.1 (2017-01) clause 7.4.1.
 */
enum class ContentsIndicatorFlag : uint8_t
{
  //! No contents indicator
  NoContentsIndicator = 0,

  //! Contents indicator(s) present
  ContentsIndicatorsPresent = 1
};

/*!
 * @brief This 4-bit field shall indicate the data content of the Byte L data field.
 *
 * See ETSI EN 300 401 V2.1.1 (2017-01) clause 7.4.1.
 *
 * @note The remaining values of the 4 bits are reserved for future use of the Byte L data field.
 */
enum class ByteLIndicator : uint8_t
{
  //! In-house information, or no information
  inHouseInfo = 0b0000,

  //! DRC (Dynamic Range Control) data for DAB audio (see ETSI TS 103 466 [1])
  dynamicRangeControl = 0b0001,
};

/*!
 * @brief Application type Description
 *
 * There are a maximum of 31 application types available.
 *
 * For applications that may generate long X-PAD data groups, two application types are
 * defined: one is used to indicate the start of an X-PAD data group and the other is used
 * to indicate the continuation of a data group after an interruption.
 * 
 * Byte streams require just one X-PAD application type.
 */
typedef enum X_PADApplicationType : int
{
  //! Default init value to see not set
  X_PADAppType_NotSet = -1,

  //! 0 End marker
  X_PADAppType_EndMarker = 0,

  //! 1 Data group length indicator
  X_PADAppType_DataGroupLengthIndicator = 1,

  //! 2 Dynamic label segment, start of X - PAD data group
  X_PADAppType_DynamicLabelSegment_StartOfX = 2,

  //! 3 Dynamic label segment, continuation of X - PAD data group
  X_PADAppType_DynamicLabelSegment_ContinuationOfX = 3,

  //! 4 to 11 User defined
  X_PADAppType_UserDefined4 = 4,
  X_PADAppType_UserDefined5 = 5,
  X_PADAppType_UserDefined6 = 6,
  X_PADAppType_UserDefined7 = 7,
  X_PADAppType_UserDefined8 = 8,
  X_PADAppType_UserDefined9 = 9,
  X_PADAppType_UserDefined10 = 10,
  X_PADAppType_UserDefined11 = 11,

  //! 12 MOT, start of X - PAD data group, see ETSI EN 301 234[6]
  X_PADAppType_MOT_StartOfX = 12,

  //! 13 MOT, continuation of X - PAD data group, see ETSI EN 301 234[6]
  X_PADAppType_MOT_ContinuationOfX = 13,

  //! 14 MOT, start of CA messages, see ETSI EN 301 234[6]
  X_PADAppType_MOT_StartOfCAMessages = 14,

  //! 15 MOT, continuation of CA messages, see ETSI EN 301 234[6]
  X_PADAppType_MOT_ContinuationOfCAMessages = 15,

  //! 16 to 30 User defined
  X_PADAppType_UserDefined16 = 16,
  X_PADAppType_UserDefined17 = 17,
  X_PADAppType_UserDefined18 = 18,
  X_PADAppType_UserDefined19 = 19,
  X_PADAppType_UserDefined20 = 20,
  X_PADAppType_UserDefined21 = 21,
  X_PADAppType_UserDefined22 = 22,
  X_PADAppType_UserDefined23 = 23,
  X_PADAppType_UserDefined24 = 24,
  X_PADAppType_UserDefined25 = 25,
  X_PADAppType_UserDefined26 = 26,
  X_PADAppType_UserDefined27 = 27,
  X_PADAppType_UserDefined28 = 28,
  X_PADAppType_UserDefined29 = 29,
  X_PADAppType_UserDefined30 = 30,

  //! 31 Not used
  X_PADAppType_Last_NotUsed = 31
} X_PADApplicationType;

// --- XPAD_CI -----------------------------------------------------------------
struct XPAD_CI
{
  size_t len;
  X_PADApplicationType type;

  static const size_t lens[];

  XPAD_CI() { Reset(); }
  XPAD_CI(uint8_t ci_raw)
  {
    len = lens[ci_raw >> 5];
    type = static_cast<X_PADApplicationType>(ci_raw & 0x1F);
  }
  XPAD_CI(size_t len, X_PADApplicationType type) : len(len), type(type) {}

  void Reset()
  {
    len = 0;
    type = X_PADAppType_NotSet;
  }
};

typedef std::list<XPAD_CI> xpad_cis_t;


// --- IPADDecoderObserver -----------------------------------------------------------------
class IPADDecoderObserver
{
public:
  virtual ~IPADDecoderObserver() = default;

  virtual void PADChangeDynamicLabel(const CDynamicLabel& dl) = 0;
  virtual void PADChangeSlide(const std::shared_ptr<MOT_FILE>& slide) = 0;

  virtual void PADLengthError(size_t announced_xpad_len, size_t xpad_len) = 0;
};


// --- CPADDecoder -----------------------------------------------------------------
class CPADDecoder
{
public:
  CPADDecoder(IPADDecoderObserver* observer, bool loose)
    : m_observer(observer), m_loose(loose), m_MOTAppType(X_PADAppType_NotSet)
  {
  }

  void SetMOTAppType(X_PADApplicationType type);
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
};
