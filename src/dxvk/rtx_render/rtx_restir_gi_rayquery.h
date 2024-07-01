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
#include "rtx_options.h"

namespace dxvk {

  enum class ReSTIRGIBiasCorrection : int {
    None,
    BRDF,
    Raytrace,
    Pairwise,
    PairwiseRaytrace
  };

  enum class ReSTIRGISampleStealing : int {
    None,
    StealSample,
    StealPixel
  };

  enum class ReSTIRGIMIS : int {
    None,
    Roughness,
    Parallax
  };
  
  class RtxContext;

  class DxvkReSTIRGIRayQuery: public RtxPass {
    
  public:
    
    DxvkReSTIRGIRayQuery(DxvkDevice* device);
    ~DxvkReSTIRGIRayQuery() = default;

    void dispatch(RtxContext* ctx, const Resources::RaytracingOutput& rtOutput);

    void showImguiSettings();

    int getTemporalHistoryLength(float frameTimeMs) {
      if(useAdaptiveTemporalHistory())
        return static_cast<int>(std::max(temporalAdaptiveHistoryLengthMs() / frameTimeMs, 20.0f));
      else
        return temporalFixedHistoryLength();
    }

    static void setToNRDPreset();

    static void setToRayReconstructionPreset();

  private:
    virtual bool isActive() override { return RtxOptions::Get()->useReSTIRGI(); }

    RTX_OPTION("rtx.restirGI", bool, useTemporalReuse, true, "Abilita il riutilizzo temporale.");
    RTX_OPTION("rtx.restirGI", bool, useSpatialReuse, true, "Abilita il riutilizzo spaziale.");
    RTX_OPTION("rtx.restirGI", bool, useFinalVisibility, true, "Testa la visibilità nell'output.");

    // ReSTIR GI cannot work very well on specular surfaces. We need to mix the specular output with its input to improve quality.
    RTX_OPTION("rtx.restirGI", ReSTIRGIMIS, misMode, ReSTIRGIMIS::Parallax, "Modalità MIS per mescolare l'output speculare con l'input.");
    RTX_OPTION("rtx.restirGI", float, misRoughness, 0.3, "Rugosità di riferimento quando viene utilizzato il MIS di rugosità. Valori più alti danno un peso maggiore agli input ReSTIR.");
    RTX_OPTION("rtx.restirGI", float, parallaxAmount, 0.02, "Forza della parallasse quando viene utilizzato il MIS di parallasse. Valori più alti danno un peso maggiore agli input ReSTIR.");

    // ReSTIR virtual sample can improve results on highly specular surfaces by storing virtual samples "behind the mirror",
    // instead of actual samples "on the mirror".
    // When an indirect ray hits a highly specular surface, the hit T will get accumulated until a path vertex with significant
    // contribution is hit. Then the hit T will be used to extend the 1st indirect ray, whose extended end point will be the
    // virtual sample's position. If the significant path vertex has high specular contribution, its distance to light source
    // will also get accumulated.
    RTX_OPTION("rtx.restirGI", bool, useVirtualSample, true, "Usa la posizione virtuale per i campioni da superfici altamente speculari.");
    RTX_OPTION("rtx.restirGI", float, virtualSampleLuminanceThreshold, 2.0, "L'ultimo vertice del percorso con luminanza maggiore di 2 volte la radianza accumulata precedente verrà virtualizzato. Valori più alti tendono a mantenere il primo vertice del percorso con contributo non nullo.");
    RTX_OPTION("rtx.restirGI", float, virtualSampleRoughnessThreshold, 0.2, R"(La superficie con rugosità sotto questa soglia è considerata altamente speculare, cioè uno "specchio".)");
    RTX_OPTION("rtx.restirGI", float, virtualSampleSpecularThreshold, 0.5, "Se la porzione di luce speculare diretta di un vertice del percorso altamente speculare è superiore a questo. La sua distanza dalla sorgente luminosa verrà accumulata.");
    RTX_OPTION("rtx.restirGI", float, virtualSampleMaxDistanceRatio, 0.0, "Limita la distanza virtuale massima, misurata come proporzione della distanza dalla telecamera. 0 disabilita il limite.");

    RTX_OPTION("rtx.restirGI", bool, useTemporalBiasCorrection, true, "Corregge il bias causato dalla riproiezione temporale.");
    RW_RTX_OPTION("rtx.restirGI", ReSTIRGIBiasCorrection, biasCorrectionMode, ReSTIRGIBiasCorrection::PairwiseRaytrace, "Modalità di correzione del bias per combinare il centrale con i suoi vicini nel riutilizzo spaziale.");
    RTX_OPTION("rtx.restirGI", float, pairwiseMISCentralWeight, 0.1, "L'importanza del campione centrale nelle modalità di correzione del bias a coppie.");
    
    RTX_OPTION("rtx.restirGI", bool, useDemodulatedTargetFunction, false, "Demodula la funzione target. Questo migliorerà il risultato nelle modalità non a coppie.");
    RTX_OPTION("rtx.restirGI", bool, usePermutationSampling, true, "Usa il campione di permutazione per perturbare i campioni. Questo migliorerà i risultati in DLSS.");
    RTX_OPTION("rtx.restirGI", bool, useDLSSRRCompatibilityMode, false, "Modalità di compatibilità DLSS-RR. In questa modalità la riproiezione temporale è randomizzata per ridurre la coerenza del campione.");
    RTX_OPTION("rtx.restirGI", int, DLSSRRTemporalRandomizationRadius, 80, "Nella modalità di compatibilità DLSS-RR la riproiezione temporale è randomizzata per ridurre la coerenza del campione. Questa opzione determina il raggio di randomizzazione.");
    RTX_OPTION("rtx.restirGI", ReSTIRGISampleStealing,  useSampleStealing, ReSTIRGISampleStealing::StealPixel, "Ruba i campioni ReSTIR GI nel path tracer. Questo migliorerà i risultati altamente speculari.");
    RTX_OPTION("rtx.restirGI", float, sampleStealingJitter, 0.0, "Sfasa i campioni di k pixel per evitare l'aliasing.");
    RTX_OPTION("rtx.restirGI", bool, stealBoundaryPixelSamplesWhenOutsideOfScreen , true, "Ruba i campioni ReSTIR GI anche se un punto di hit è fuori dallo schermo. Questo migliorerà ulteriormente i campioni altamente speculari a costo di un po' di bias.");
    RTX_OPTION("rtx.restirGI", bool, useDiscardEnlargedPixels, true, "Scarta i campioni ingranditi quando la telecamera si sta muovendo verso un oggetto.");
    RTX_OPTION("rtx.restirGI", float, historyDiscardStrength, 0.0, "La sensibilità dello scarto della storia. Valori più alti scartano più storia.");
    RTX_OPTION("rtx.restirGI", bool, useTemporalJacobian, true, "Calcola il determinante jacobiano nella riproiezione temporale.");
    RW_RTX_OPTION("rtx.restirGI", bool, useReflectionReprojection, true, "Usa la riproiezione della riflessione per oggetti riflettenti per ottenere un risultato stabile quando la telecamera si muove.");
    RTX_OPTION("rtx.restirGI", float, reflectionMinParallax, 3.0, "Quando la parallasse tra la normale e la riproiezione della riflessione è maggiore di questa soglia, scegli casualmente una posizione riproiettata e riutilizza il campione su di essa. Altrimenti, ottieni un campione tra le due posizioni.");
    RTX_OPTION("rtx.restirGI", bool, useBoilingFilter, true, "Abilita il filtro anti-ebollizione per sopprimere gli artefatti di ebollizione.");
    RTX_OPTION("rtx.restirGI", float, boilingFilterMinThreshold, 10.0, "Soglia del filtro anti-ebollizione quando la normale della superficie è perpendicolare alla direzione di vista.");
    RTX_OPTION("rtx.restirGI", float, boilingFilterMaxThreshold, 20.0, "Soglia del filtro anti-ebollizione quando la normale della superficie è parallela alla direzione di vista.");
    RTX_OPTION("rtx.restirGI", float, boilingFilterRemoveReservoirThreshold, 62.0, "Rimuove un campione quando il peso di un campione supera questa soglia.");
    RTX_OPTION_ENV("rtx.restirGI", bool, useAdaptiveTemporalHistory, true, "DXVK_USE_ADAPTIVE_RESTIR_GI_ACCUMULATION", "Regola la lunghezza della storia temporale in base al frame rate.");
    RTX_OPTION("rtx.restirGI", int, temporalAdaptiveHistoryLengthMs, 500, "Temporal history time length, when adaptive temporal history is enabled.");
    RTX_OPTION("rtx.restirGI", int, temporalFixedHistoryLength, 30, "Lunghezza fissa della storia temporale, quando la storia temporale adattiva è disabilitata.");
    RTX_OPTION("rtx.restirGI", int, permutationSamplingSize, 2, "Forza del campionamento di permutazione.");
    RTX_OPTION("rtx.restirGI", float, fireflyThreshold, 50.f, "Limita l'input speculare per sopprimere l'ebollizione.");
    RTX_OPTION("rtx.restirGI", float, roughnessClamp, 0.01f, "Limita la rugosità minima con cui viene valutata l'importanza di un campione.");
    RTX_OPTION("rtx.restirGI", bool, validateLightingChange, true, "Rimuove i campioni quando la luce diretta è cambiata.");
    RTX_OPTION_ENV("rtx.restirGI", bool, validateVisibilityChange, false, "DXVK_RESTIR_GI_VISIBILITY_VALIDATION", "Rimuove i campioni quando la visibilità è cambiata. Questa funzione è automaticamente disabilitata quando il campione virtuale è abilitato.");
    RTX_OPTION_ENV("rtx.restirGI", float, lightingValidationThreshold, 0.5, "DXVK_RESTIR_GI_SAMPLE_VALIDATION_THRESHOLD", "Invalida un campione quando il rapporto di cambiamento dei pixel è superiore a questo valore.");
    RTX_OPTION("rtx.restirGI", float, visibilityValidationRange, 0.05, "Controlla la distanza effettiva di hit di un raggio d'ombra, invalida un campione se la lunghezza di hit è maggiore di uno più questa porzione, rispetto alla distanza dalla superficie al campione.");
  };
}
