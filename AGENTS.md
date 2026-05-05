# PhSynth One

Projet VST3 JUCE/CMake pour FL Studio. Objectif initial : instrument synthé simple avec modes Pad/Bass/Lead/Pluck/String, ADSR, filtre, reverb, delay et pitch, design minimal futuriste type workstation/synth premium.

- Projet : `VST/PhSynthOne`
- Build Windows cible : Visual Studio 2022 + CMake, sortie VST3 dans `build/PhSynthOne_artefacts/Release/VST3/`
- Ne pas ajouter de dépendances lourdes hors JUCE FetchContent sauf nécessité.
