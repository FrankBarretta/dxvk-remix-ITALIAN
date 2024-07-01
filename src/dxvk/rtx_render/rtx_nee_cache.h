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

#include "../dxvk_format.h"
#include "../dxvk_include.h"

#include "../spirv/spirv_code_buffer.h"
#include "rtx_resources.h"
#include "rtx_option.h"
#include "rtx/algorithm/nee_cache_data.h"

namespace dxvk {

  class RtxContext;

  class NeeCachePass {

  public:
    NeeCachePass(dxvk::DxvkDevice* device);
    ~NeeCachePass();

    void dispatch(
      RtxContext* ctx, 
      const Resources::RaytracingOutput& rtOutput);

    void showImguiSettings();

    void setRaytraceArgs(RaytraceArgs& raytraceArgs, bool resetHistory) const;

    RW_RTX_OPTION("rtx.neeCache", bool, enable, true, "[Sperimentale] Abilita la cache NEE. L'integratore eseguirà NEE su triangoli emissivi, che di solito hanno contributi luminosi significativi, memorizzati nella cache.");
    RTX_OPTION("rtx.neeCache", bool, enableImportanceSampling, true, "Abilita il campionamento di importanza.");
    RTX_OPTION("rtx.neeCache", bool, enableMIS, true, "Abilita MIS.");
    RTX_OPTION("rtx.neeCache", bool, enableUpdate, true, "Abilita l'aggiornamento.");
    RTX_OPTION("rtx.neeCache", bool, enableOnFirstBounce, true, "Abilita la cache NEE sul primo rimbalzo.");
    RW_RTX_OPTION("rtx.neeCache", NeeEnableMode, enableModeAfterFirstBounce, NeeEnableMode::SpecularOnly, "Modalità di abilitazione della cache NEE sul secondo e sui rimbalzi successivi. 0 significa disattivato, 1 significa abilitato solo per i raggi speculari, 2 significa sempre abilitato.");
    RTX_OPTION("rtx.neeCache", bool, enableAnalyticalLight, true, "Abilita la cache NEE sulla luce analitica.");
    RTX_OPTION("rtx.neeCache", float, specularFactor, 1.0, "Fattore della componente speculare.");
    RTX_OPTION("rtx.neeCache", float, learningRate, 0.02, "Tasso di apprendimento. Valori più alti fanno adattare la cache più rapidamente ai cambiamenti di illuminazione.");
    RTX_OPTION("rtx.neeCache", float, uniformSamplingProbability, 0.1, "Probabilità di campionamento uniforme.");
    RTX_OPTION("rtx.neeCache", float, cullingThreshold, 0.01, "Soglia di eliminazione.");
    RTX_OPTION("rtx.neeCache", float, resolution, 8.0, "Risoluzione della cella. Valori più alti significano celle più piccole.");
    RTX_OPTION("rtx.neeCache", float, minRange, 400, "Il range per le celle di livello più basso.");
    RTX_OPTION("rtx.neeCache", float, emissiveTextureSampleFootprintScale, 1.0, "Scala dell'impronta del campione di texture emissiva.");
    RTX_OPTION("rtx.neeCache", bool,  approximateParticleLighting, true, "Usa l'albedo delle particelle come colore emissivo.");
    RTX_OPTION("rtx.neeCache", float, ageCullingSpeed, 0.02, "Questa soglia determina la velocità di eliminazione di un triangolo vecchio. Un triangolo che non viene rilevato per diversi frame sarà considerato meno importante e eliminato più rapidamente.");
    RTX_OPTION("rtx.neeCache", bool,  enableTriangleExploration, true, "Esplora i candidati di triangoli emissivi nello stesso oggetto.");
    RTX_OPTION("rtx.neeCache", float, triangleExplorationProbability, 0.05, "La probabilità di esplorare nuovi triangoli.");
    RTX_OPTION("rtx.neeCache", int,   triangleExplorationMaxRange, 20, "Range di indici da esplorare, quando l'esplorazione dei triangoli è abilitata.");
    RTX_OPTION("rtx.neeCache", float, triangleExplorationRangeRatio, 0.1, "Rapporto tra range di indici e conteggio dei triangoli, quando l'esplorazione dei triangoli è abilitata.");
    RTX_OPTION("rtx.neeCache", float, triangleExplorationAcceptRangeRatio, 0.33, "Rapporto tra range di indici accettati e range di ricerca, quando l'esplorazione dei triangoli è abilitata.");
    RTX_OPTION("rtx.neeCache", bool,  enableSpatialReuse, true, "Abilita la condivisione delle informazioni statistiche della cella NEE con i vicini.");
  private:
    Rc<vk::DeviceFn> m_vkd;
  };
} // namespace dxvk
