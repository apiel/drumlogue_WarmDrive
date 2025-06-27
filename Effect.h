#pragma once

#include "unit.h"

#include <math.h>
#include <algorithm>
#include <cstdio>

class Effect {
 private:
  float clipping = 0.0f;
  float scaledClipping = 0.0f;
  float level = 1.0f;
  float drive = 0.0f;
  float compress = 0.0f;
  float bassBoost = 0.0f;
  float highBoost = 0.0f;
  float waveshape = 0.0f;
  float prevIn1 = 0.0f, prevOut1 = 0.0f;
  float prevIn2 = 0.0f, prevOut2 = 0.0f;
  float prevIn3 = 0.0f, prevOut3 = 0.0f;

  float clamp(float x, float min, float max) {
    // return std::max(min, std::min(max, x));
    return x < min ? min : x > max ? max
                                   : x;
  }

  float processSample(float input) {
    float out = input;
    out = applyClipping(out);
    out = applyBoost(out);
    out = highFreqBoost(out);
    out = applyDrive(out);
    out = applyCompression(out);
    out = applyWaveshape(out);
    out = tanhf(out);
    return out;
  }

  float applyBoost(float input) {
    if (bassBoost == 0.0f) return input;
    float out = applyBoostSub(input, prevIn1, prevOut1);
    out = applyBoostSub(out, prevIn2, prevOut2);
    // out = applyBoostSub(out, prevIn3, prevOut3);
    return out;
  }

  float applyBoostSub(float input, float & prevIn, float & prevOut) {
    if (bassBoost == 0.0f) return input;
    float freq = 0.2f + 0.8f * bassBoost;
    float boosted = (1.0f - freq) * prevOut + freq * (input + prevIn) * 0.5f;
    prevIn = input;
    prevOut = boosted;
    return boosted * (1.0f + bassBoost * 2.0f);
  }

  float applyDrive(float input) {
    if (drive == 0.0f) return input;
    return tanhf(input * (1.0f + drive * 5.0f));
  }

  float applyCompression(float input) {
    if (compress == 0.0f) return input;
    if (input > 0.0f) return powf(input, 1.0f - compress * 0.8f);
    return -powf(-input, 1.0f - compress * 0.8f);
  }

  float applyWaveshape(float input) {
    if (waveshape > 0.0f) {
      // float sineValue = sineLookupInterpolated(input);
      float sineValue = sinf(input);
      return input + waveshape * sineValue * 2.0f;
    }
    if (waveshape < 0.0f) {
      float sineValue = sineLookupInterpolated(input);
      return input + (-waveshape) * sineValue;
    }
    return input;
  }

#define LOOKUP_TABLE_SIZE 4096
  float sine[LOOKUP_TABLE_SIZE];

  float sineLookupInterpolated(float x) {
    x -= std::floor(x);
    return linearInterpolation(x, LOOKUP_TABLE_SIZE, sine);
  }

  float linearInterpolation(float index, uint16_t lutSize, float * lut) {
    return linearInterpolationAbsolute(index * (lutSize - 1), lutSize, lut);
  }

  float linearInterpolationAbsolute(float index, uint16_t lutSize, float * lut) {
    uint16_t index1 = static_cast<uint16_t>(index);
    uint16_t index2 = (index1 + 1) % lutSize;
    float fractional = index - index1;

    // Interpolate between the two LUT values
    return lut[index1] * (1.0f - fractional) + lut[index2] * fractional;
  }

  float applyClipping(float input) {
    if (scaledClipping == 0.0f) {
      return input;
    }
    return clamp(input + input * scaledClipping, -1.0f, 1.0f);

    // return clamp(input + input * scaledClipping, -0.10f, 0.10f);
  }

  double boostTimeInc = 0.0f;
  double boostTime = 0.0f;
  float highFreqBoost(float input) {
    if (highBoost == 0.0f) {
      return input;
    }
    // float highFreqComponent = input * boostTime;  // Emphasize high frequencies
    // boostTime += boostTimeInc;
    // return input + highFreqComponent;
    float out = highFreqBoostSub(input);
    out = highFreqBoostSub(out);
    return out;
  }

  float highFreqBoostSub(float input) {
    float highFreqComponent = input * boostTime;  // Emphasize high frequencies
    boostTime += boostTimeInc;
    return input + highFreqComponent;
  }

 public:
  Effect() {
    for (int i = 0; i < LOOKUP_TABLE_SIZE; i++) {
      float normalizedX = (float)i / (float)(LOOKUP_TABLE_SIZE - 1) * 2.0f - 1.0f;  // Map index to [-1.0, 1.0]
      sine[i] = sinf(normalizedX * M_PI);
      // tanh[i] = std::tanh(normalizedX); // Precompute tanh
      // noise[i] = 2.0f * ((float)rand() / (float)RAND_MAX) - 1.0f; // Random noise [-1.0, 1.0]
    }
  }

  int Init(const unit_runtime_desc_t * desc) {
    prevIn1 = prevOut1 = 0.0f;
    prevIn2 = prevOut2 = 0.0f;
    prevIn3 = prevOut3 = 0.0f;
    return k_unit_err_none;
  }

  void Reset() {
    prevIn1 = prevOut1 = 0.0f;
    prevIn2 = prevOut2 = 0.0f;
    prevIn3 = prevOut3 = 0.0f;
  }

  void Process(const float * in, float * out, uint32_t frames) {
    // for (uint32_t i = 0; i < frames; i++) {
    //   float input = in[i];
    //   float output = processSample(input);bassBoost
    //   out[i] = clamp(output, -1.0f, 1.0f);
    // }

    const float * __restrict in_p = in;
    float * __restrict out_p = out;
    const float * out_e = out_p + (frames << 1);  // assuming stereo output

    // Caching current parameter values. Consider interpolating sensitive parameters.
    // const Params p = params_;

    for (; out_p != out_e; in_p += 2, out_p += 2) {
      // Process samples here

      // Note: this is a dummy unit only to demonstrate APIs, only passing through audio
      out_p[0] = processSample(in_p[0]);  // left sample
      out_p[1] = processSample(in_p[1]);  // right sample
    }
  }

  void setParameter(uint8_t id, int32_t value) {
    float pct = value / 100.0f;
    switch (id) {
      case 0:
        clipping = pct;
        scaledClipping = pct * pct * 20;
        break;
      case 1:
        drive = pct;
        break;
      case 2:
        compress = pct;
        break;
      case 3:
        waveshape = pct * 2.0f - 1.0f;
        break;
      case 4:
        bassBoost = pct;
        break;
      case 5:
        highBoost = pct;
        break;
    }
  }

  int32_t getParameterValue(uint8_t id) const {
    switch (id) {
      case 0:
        return int32_t(clipping * 100);
      case 1:
        return int32_t(drive * 100);
      case 2:
        return int32_t(compress * 100);
      case 3:
        return int32_t((waveshape + 1.0f) * 50.0f);
      case 4:
        return int32_t(bassBoost * 100);
      case 5:
        return int32_t(highBoost * 100);
      default:
        return 0;
    }
  }

  const char * getParameterStrValue(uint8_t id, int32_t value) {
    static char buf[16];
    snprintf(buf, sizeof(buf), "%d%%", value);
    return buf;
  }

  const uint8_t * getParameterBmpValue(uint8_t id, int32_t value) {
    return nullptr;  // Not used for now
  }
};
