# PhSynth One

Instrument VST3 simple pour FL Studio : synthé multi-caractères avec modes **Pad**, **Bass**, **Lead**, **Pluck** et **String**.

## Contrôles

- Sound Type : Pad / Bass / Lead / Pluck / String
- ADSR : Attack, Decay, Sustain, Release
- Filter : Cutoff, Resonance
- FX : Reverb, Delay
- Pitch : transposition en demi-tons
- Volume

## Build Windows pour FL Studio

Pré-requis : Visual Studio 2022 avec workload C++, CMake, Git.

```powershell
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release
```

Le VST3 sortira dans :

```text
build/PhSynthOne_artefacts/Release/VST3/PhSynth One.vst3
```

Copie ensuite le `.vst3` dans :

```text
C:\Program Files\Common Files\VST3
```

Puis dans FL Studio : **Options > Manage plugins > Find installed plugins**.

## Build local Linux / test rapide

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```
