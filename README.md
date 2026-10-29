# MIDI Player for the PX-42F

Plays Standard MIDI Files on the PX-42F calculator's speaker, several
instruments at once. It's a native app built on the PX-42F App Template (the
framework's files, `px42f/` and `sim/`, come from there). The synthesizer
itself is in the PX-42F-Plus firmware; the app only picks the song
(`px->midi_play()`, app API version 4) and shows how it's going.

The user manual is [`manual.html`](manual.html) (built by `python3 tools/make_manual.py`).

## Using it

Copy `.mid` files into `/MIDI` on the calculator's drive (Settings →
USB Drive → Connect to Computer).

| Key | Does |
|---|---|
| ↑ ↓ | choose a song |
| ENTER | play it |
| R/S | play the chosen song, or stop |
| ⌫ | stop |
| Σ+ (top left) | repeat all the songs, or play each once |
| ON | back to the menu (the music stops; held 2 s: off) |

The screen shows the list, a progress bar with the time, the notes sounding,
and 16 boxes, one a MIDI channel, lit for the first 3/4 of each note (the tenth is
the drums).

## What the synthesizer plays

- Format 0 and 1 files up to 128 KB, with tempo changes. 24 notes at once.
- Each channel's General MIDI program picks one of eleven instruments: piano,
  bell, organ, plucked, bass, strings, brass, reed, flute, lead, pad. Channel
  10 is the drum kit (kick, snare, hats, toms, cymbals, clicks).
- Volume, expression, modulation, the sustain pedal and pitch bend work;
  other controllers are ignored.
- The speaker is small and mono: it sounds like a good ringtone, not an
  orchestra.

## Build and install

It needs the PX-42F-Plus firmware with app API version 4 or later.

    make            builds build/MIDI-Player.bin
    make sim        runs it on the computer (no sound: the simulator pretends a song lasts 20 s)
    make install    copies it onto the calculator's drive (USB Drive mode)

To hear what the firmware's synthesizer does with any `.mid` file without the
calculator: `PX-42F-Plus/tools/px_sound/run.sh OUT_DIR song.mid` writes it as a WAV.
