/*
* Copyright (c) 2023, NVIDIA CORPORATION. All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a
* copy of this software and associated documentation files (the "Software"),
* to deal in the Software without restriction, including without limitation
* the rights to use, copy, modify, merge, publish, distribute, sublicense,
* and/or sell copies of the Software, and to permit persons to whom the
* Software is furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
* FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
* DEALINGS IN THE SOFTWARE.
*/
#pragma once

#include "rtx_resources.h"
#include "rtx_options.h"

struct RaytraceArgs;

namespace dxvk {
  class Config;
  class RtxContext;

  class DxvkRtxdiRayQuery {
    
  public:
    
    DxvkRtxdiRayQuery(DxvkDevice* device);
    ~DxvkRtxdiRayQuery() = default;

    void dispatch(RtxContext* ctx, const Resources::RaytracingOutput& rtOutput);
    void dispatchGradient(RtxContext* ctx, const Resources::RaytracingOutput& rtOutput);
    void dispatchConfidence(RtxContext* ctx, const Resources::RaytracingOutput& rtOutput);

    void showImguiSettings();
    void setRaytraceArgs(Resources::RaytracingOutput& rtOutput) const;
    bool getEnableDenoiserConfidence() const { return enableTemporalReuse() && enableDenoiserGradient() && enableDenoiserConfidence(); }
    
    RTX_OPTION("rtx.di", bool, enableCrossPortalLight, true, "");
    RTX_OPTION("rtx.di", bool, enableInitialVisibility, true, "Se tracciare un raggio di visibilità per il campione di luce selezionato nel passaggio di campionamento iniziale.");
    RTX_OPTION("rtx.di", bool, enableBestLightSampling, true, "Se includere una singola luce migliore dal vicinato dei pixel del frame precedente nel campionamento iniziale.");
    RW_RTX_OPTION("rtx.di", bool, enableRayTracedBiasCorrection, true, "Se utilizzare la correzione del bias tracciata da raggi nel passaggio di riutilizzo spaziale.");
    RTX_OPTION("rtx.di", bool, enableSampleStealing, true, "Nessun guadagno di qualità dell'immagine visibile, ma mostra un considerevole calo delle prestazioni (8% nel passaggio di integrazione).");
    RW_RTX_OPTION("rtx.di", bool, stealBoundaryPixelSamplesWhenOutsideOfScreen, true, "Ruba i campioni del bordo dello schermo quando un punto di hit è fuori dallo schermo.");
    RTX_OPTION("rtx.di", bool, enableSpatialReuse, true, "Se applicare il riutilizzo spaziale.");
    RTX_OPTION("rtx.di", bool, enableTemporalBiasCorrection, true, "");
    RTX_OPTION("rtx.di", bool, enableTemporalReuse, true, "Se applicare il riutilizzo temporale.");
    RTX_OPTION("rtx.di", bool, enableDiscardInvisibleSamples, true, "Se scartare i serbatoi che sono determinati essere invisibili nell'ombreggiatura finale.");
    RTX_OPTION("rtx.di", bool, enableDiscardEnlargedPixels, true, "");
    RW_RTX_OPTION("rtx.di", bool, enableDenoiserConfidence, true, "");
    RTX_OPTION("rtx.di", bool, enableDenoiserGradient, true, "Abilita il calcolo del gradiente, che viene utilizzato dal calcolo della confidenza e dalla validazione del campione GI.");
    RTX_OPTION("rtx.di", uint32_t, initialSampleCount, 4, "Il numero di luci selezionate casualmente dal pool globale da considerare quando si seleziona una luce con RTXDI.");
    RTX_OPTION("rtx.di", uint32_t, spatialSamples, 2, "Il numero di campioni di riutilizzo spaziale nelle aree convergenti.");
    RTX_OPTION("rtx.di", uint32_t, disocclusionSamples, 4, "Il numero di campioni di riutilizzo spaziale nelle aree di disocclusione.");
    RTX_OPTION("rtx.di", uint32_t, disocclusionFrames, 8, "");
    RTX_OPTION("rtx.di", uint32_t, gradientFilterPasses, 4, "");
    RW_RTX_OPTION("rtx.di", uint32_t, permutationSamplingNthFrame, 0, "Applica il campionamento di permutazione quando (frameIdx % questo == 0), 0 significa disattivato.");
    RTX_OPTION("rtx.di", uint32_t, maxHistoryLength, 4, "Età massima dei serbatoi per il riutilizzo temporale.");
    RTX_OPTION("rtx.di", float, gradientHitDistanceSensitivity, 10.f, "");
    RTX_OPTION("rtx.di", float, confidenceHistoryLength, 8.f, "");
    RTX_OPTION("rtx.di", float, confidenceGradientPower, 8.f, "");
    RTX_OPTION("rtx.di", float, confidenceGradientScale, 6.f, "");
    RTX_OPTION("rtx.di", float, minimumConfidence, 0.1f, "");
    RTX_OPTION("rtx.di", float, confidenceHitDistanceSensitivity, 300.0f, "");
  };
}
