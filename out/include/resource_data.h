#include <stdint.h>
extern const uint8_t resource_IDR_FAIL[];
extern const uint8_t resource_IDR_FAIL_end[];
extern const uint8_t resource_IDR_INSERT[];
extern const uint8_t resource_IDR_INSERT_end[];
extern const uint8_t resource_IDR_REMOVE[];
extern const uint8_t resource_IDR_REMOVE_end[];
extern const uint8_t resource_IDR_WAV_BEEPBWEEP[];
extern const uint8_t resource_IDR_WAV_BEEPBWEEP_end[];
extern const uint8_t resource_IDR_WAV_CLEAR[];
extern const uint8_t resource_IDR_WAV_CLEAR_end[];
extern const uint8_t resource_IDR_WAV_DRIP[];
extern const uint8_t resource_IDR_WAV_DRIP_end[];
#include "Util/ConstBuffer.hxx"
#include <tchar.h>
static constexpr struct {
  const TCHAR *name;
  ConstBuffer<void> data;
} named_resources[] = {  { _T("IDR_FAIL"), { resource_IDR_FAIL, 73140 } },
  { _T("IDR_INSERT"), { resource_IDR_INSERT, 73184 } },
  { _T("IDR_REMOVE"), { resource_IDR_REMOVE, 72988 } },
  { _T("IDR_WAV_BEEPBWEEP"), { resource_IDR_WAV_BEEPBWEEP, 58256 } },
  { _T("IDR_WAV_CLEAR"), { resource_IDR_WAV_CLEAR, 36660 } },
  { _T("IDR_WAV_DRIP"), { resource_IDR_WAV_DRIP, 6054 } },
  { 0, { nullptr, 0 } }
};
