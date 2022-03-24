/**
 * projectM -- Milkdrop-esque visualisation SDK
 * Copyright (C)2003-2007 projectM Team
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 * See 'LICENSE.txt' included within this release
 *
 */
/**
 * $Id$
 *
 * Encapsulation of raw sound buffer. Used in beat detection
 *
 * $Log$
 */

#ifndef _PCM_H
#define _PCM_H

#include <array>
#include <cstdint>
#include <cstdlib>


// FFT_LENGTH is number of magnitude values available from getSpectrum().
// Internally this is generated using 2xFFT_LENGTH samples per channel.
size_t constexpr fftLength = 512;
class Test;

enum CHANNEL
{
    CHANNEL_L = 0,
    CHANNEL_0 = 0,
    CHANNEL_R = 1,
    CHANNEL_1 = 1
};

class Pcm
{
public:
    /* maximum number of sound samples that are actually stored. */
    static constexpr size_t maxSamples = 2048;

    void AddPcmFloat(const float* pcmData, size_t samples);
    void AddPcmFloat2Ch(const float* pcmData, size_t count);
    void AddPcm16(const int16_t pcmData[2][512]);
    void AddPcm16Data(const int16_t* pcmData, size_t samples);
    void AddPcm8(const uint8_t pcmData[2][1024]);
    void AddPcm8(const uint8_t pcmData[2][512]);

    /**
     * PCM data
     * smoothing does nothing
     * The returned data will 'wrap' if more than maxsamples are requested.
     */
    void GetPcm(float* data, CHANNEL channel, size_t samples, float smoothing);

    /** Spectrum data
     * smoothing does nothing
     * The returned data will be zero padded if more than FFT_LENGTH values are requested
     */
    void GetSpectrum(float* data, CHANNEL channel, size_t samples, float smoothing);

    static auto MakeTest() -> Test*;

private:
    template<int signalAmplitude, int signalOffset, class SampleType>
    void AddPcm(
        const SampleType* pcmData,
        size_t lOffset,
        size_t rOffset,
        size_t stride,
        size_t count);

    // mem-usage:
    // pcmd 2x2048*4b    = 16K
    // vdata 2x512x2*8b  = 16K
    // spectrum 2x512*4b = 4k
    // w = 512*8b        = 4k

    // circular PCM buffer
    // adjust "volume" of PCM data as we go, this simplifies everything downstream...
    // normalize to range [-1.0,1.0]
    std::array<float, maxSamples> m_pcmL{0.f};
    std::array<float, maxSamples> m_pcmR{0.f};
    int m_start{0};
    size_t m_newSamples{0};

    // raw FFT data
    std::array<double, 2 * fftLength> m_freqL{0.0};
    std::array<double, 2 * fftLength> m_freqR{0.0};
    // magnitude data
    std::array<float, fftLength> m_spectrumL{0.f};
    std::array<float, fftLength> m_spectrumR{0.f};

    std::array<double, fftLength> m_w{0.0};
    std::array<int, 34> m_ip{0};

    // copy data out of the circular PCM buffer
    void CopyPcm(float* to, int channel, size_t count);
    void CopyPcm(double* to, int channel, size_t count);

    // update FFT data if new samples are available.
    void UpdateFFT();
    void UpdateFFTChannel(size_t channel);

    friend class PCMTest;

    // see https://github.com/projectM-visualizer/projectm/issues/161
    class AutoLevel
    {
    public:
        auto UpdateLevel(size_t samples, double sum, double max) -> double;

    private:
        double m_level{0.01};
        // accumulate sample data
        size_t m_levelSamples{0};
        double m_levelSum{0.0};
        double m_levelax{0.0};
        double m_l0{-1.0};
        double m_l1{-1.0};
        double m_l2{-1.0};
    };

    // state for tracking audio level
    double m_level{1.f};
    AutoLevel m_leveler{};
};

#endif /** !_PCM_H */
