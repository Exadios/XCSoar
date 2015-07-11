# From original build/libaudio.mk

set(audio_default_sources
    ToneSynthesiser.cpp
    VarioSynthesiser.cpp
    PCMPlayer.cpp)
set(audio_android_sources ${audio_default_sources} SLES/Init.cpp)

