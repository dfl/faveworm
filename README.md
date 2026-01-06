# Faveworm Oscilloscope

A love letter to the analog oscilloscopes of yore, trasnforming audio into liquid light. Inspired by the legendary work at [oscilloscopemusic.com](https://oscilloscopemusic.com).

Built on the shoulders of giants with [Visage](https://github.com/VitalAudio/visage), Faveworm features a custom physics-based beam renderer raided from [Laurent de Soras' faveworm rendering plugin for Vapoursynth](https://ldesoras.fr/prod.html#src_vs).

![XY Mode Visualization](images/screenshot.png)

## Display Modes

| Mode | Description |
|------|-------------|
| **TimeFree** | Classic horizontal sweep, free running. Good for checking levels. |
| **TimeTrigger** | Locked horizontal sweep. Stays steady on periodic signals. |
| **XY** | The main event. Left channel = X, Right channel = Y. Pure math aesthetics. |

## Controls

### General
| Key | Action | Note |
|-----|--------|------|
| `M` | Cycle Mode | Switch between the 3 modes above |
| `Space` | Play/Pause | Stop time (or just the audio) |
| `B` | Bloom | Toggle that sweet, hazy glow |
| `P` | Persistence | Toggle phosphor trail memory |
| `Up/Down` | Intensity | Crank the beam brightness |

### Trigger Mode
| Key | Action |
|-----|--------|
| `L` | Lock | Toggle autocorrelation locking (steady!) |
| `E` | Edge | Rising vs. Falling edge trigger |
| `Shift+Up/Down` | Threshold | Set the trigger point |

### Filter (XY Mode)
This is where the magic happens. We use a State Variable Filter to phase-shift the stereo signal, turning mono sources into rotating 3D-like structures.

| Key | Action |
|-----|--------|
| `F` | Filter Impulse | Toggle the SVF on/off |
| `S` | Split Mode | **Stereo Split**: Sends different filter responses (LP, HP, BP, etc.) to X and Y axes for phase wizardry. |
| `D` | Preset Cycle | Cycle through the curated Split Mode presets (see below) |
| `Left/Right` | Cutoff | Sweep the frequency spectrum |

### Split Mode Recipes
Different filter combinations create entirely different visual geometries. Here are some of our favorites:

| Preset | X Axis | Y Axis | Sweet Spot | Aesthetic |
|--------|--------|--------|------------|-----------|
| **Retro** | Lowpass | Highpass | 40-80 Hz | Classic ellipses, warm analog feel |
| **Flowers** | Bandpass | Allpass | 150-250 Hz | Swirling geometric petals |
| **Kaleidoscope** | Bandstop | Allpass | 300-800 Hz | Psychedelic mandalas, sharp edges |
| **Organic** | Lowpass | Bandpass | 100-200 Hz | Soft loops and folding shapes |
| **Liquid** | Allpass | Highpass | 20-40 Hz | Vector graphics, CRT water |
| **Complement** | Bandpass | Bandstop | 200-500 Hz | Complementary bands, "hollow" sounds |

### Test Signal Generator (XY Mode)

Beta controls the oscillator waveform, at high values you will get chaos!
Protect your ears by turning down the volume.

| Key | Action |
|-----|--------|
| `T` | Toggle Signal | Turn the internal noise machine on/off |
| `[` / `]` | Frequency | Tune the base pitch |
| `Shift+[` / `Shift+]` | Detune | Adjust the ratio between X and Y oscillators to control rotation speed |

## Filter Joystick
Check the bottom-right corner. This circular pad is your unified control surface for the SVF filter.

```
            BP (bandpass)
               ↑
               |
    LP ←———— AP ————→ HP
  (lowpass)  (center)  (highpass)
               |
               ↓
            BR (notch)
```

- **Angle**: Morphs between filter topologies (smooth crossfading!)
- **Radius**: Filter intensity (Center = bypass/allpass, Edge = max filter character)
- **Mouse Wheel**: Cutoff Frequency
- **Shift + Wheel**: Resonance (Q factor) - careful, it screams!

## Drag & Drop
Just drag a WAV file onto the window. We eat these formats for breakfast:
- 16/24/32-bit PCM
- 32-bit Float (for that extra dynamic range)

## Under the Hood
- **Engine**: [Visage](https://github.com/VitalAudio/visage) (C++ UI logic)
- **DSP**: Custom C++ library `superfreak` tailored for audio visualizers.
- **Filters**: 360° morphing SVF implementation.
- **Oscillators**: Anti-aliased RPM oscillators for the test signal.

## Building

```bash
cd build
cmake ..
cmake --build . --target Faveworm
```
