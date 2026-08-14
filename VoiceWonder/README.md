# UCG Voice Wonder

VST3 de voz ligero para Windows 11 y FL Studio: pitch, afinación cromática, filtro radio, compresión/vocoder, saturación, warmth/air y salida voz→MIDI.

## Compilar automáticamente

Sube esta carpeta a GitHub, abre **Actions → Build Windows VST3 → Run workflow** y descarga el artefacto `UCG-Voice-Wonder-Windows-VST3`.

## Instalar

Copia `UCG Voice Wonder.vst3` a `C:\Program Files\Common Files\VST3`, abre FL Studio y ejecuta **Manage plugins → Find installed plugins**. Para voz→MIDI, habilita la salida MIDI del wrapper del plugin y enrútala al instrumento deseado.

## Nota técnica

El perfil vocal analiza rasgos acústicos, no suplanta ni incluye voces de artistas. La corrección actual es de baja latencia y el pitch es deliberadamente liviano; la siguiente revisión debe sustituir el remuestreo por un phase-vocoder/formant shifter de producción.

