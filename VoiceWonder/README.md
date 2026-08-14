# UCG Voice Wonder Pro 1.0

VST3 vocal de producción para Windows 64 bits y FL Studio.

## Funciones

- Pitch continuo con Signalsmith Stretch; no usa repetición/remuestreo por bloque.
- Auto-tune cromático, mayor o menor con selección de tonalidad y velocidad.
- Desplazamiento de formantes con compensación de pitch.
- Análisis de WAV, MP3, FLAC, AIFF y OGG desde la librería del usuario.
- MATCH crea un perfil editable de tono, rango, brillo, calidez, presencia, aire, dinámica y sibilancia.
- 18 presets de fábrica con estados reales.
- Gate/expansor, de-esser, EQ vocal, compresor, saturación, radio, vocoder de ocho bandas, doubler, width, delay, reverb y limiter.
- Voz a MIDI con estabilización, hold y note-off.
- Guardado/carga de presets `.ucgvw` y restauración del estado del proyecto.
- Latencia reportada al DAW y dry/wet compensado.

## Instalación

Ejecuta `UCG-Voice-Wonder-Pro-Setup.exe`, abre FL Studio y selecciona **Options > Manage plugins > Find installed plugins**.

## MATCH

Pulsa **ANALYSE VOICE FILE**, selecciona una voz limpia de tu librería y espera el resumen. El perfil modifica Warmth, Presence, Air, Formant, Compression y De-Esser. No suplanta una identidad: transfiere características acústicas medibles.

## Validación

El workflow compila Release x64, ejecuta una prueba de voz sintética continua y rechaza muestras no finitas, clipping, silencios inesperados y discontinuidades antes de crear el instalador.

