# Active engineering prompt

Build and deliver a production-grade VST3 vocal processor. Never label a placeholder, mock DSP, disconnected control, untested preset, or uncompiled source as final. The release gate requires continuous-audio stability, reference-file analysis, functional presets, state recall, Windows x64 compilation, a smoke test, an installer, and an integrity hash. Diagnose and correct failures autonomously until every gate passes.

## Release gates

- No per-block sample wrapping or discontinuous pitch resampling.
- Every visible control changes audio or MIDI behavior.
- Factory presets contain real parameter states.
- Reference analysis accepts library audio files and creates an editable MATCH profile.
- The host receives reported latency and stable MIDI note-on/note-off messages.
- Windows x64 VST3 and installer compile successfully.
- Automated DSP test rejects NaN, clipping, silence/dropouts and discontinuities.

