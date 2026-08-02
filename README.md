# dramaturgh

Stage-tech experiments for a theatre piece. Self-contained projects,
plus shared visual material.

## CORTEX·01 — `BrainButtonInterface/`

A spinning 1-bit halftone brain wired to a physical button. An ESP32
counts presses, drives a WS2812B LED strip (each press fires a pulse that
shifts from white to red as the count climbs), and serves the count over
Wi-Fi:

```
GET /counter -> {"counter": 12}
GET /reset   -> {"counter": 0}
```

`BrainButtonInterface/index.html` polls the device and fires the on-screen
neurons on every press. The device count is authoritative — the page
adopts whatever the ESP32 reports.

- Web page: `BrainButtonInterface/index.html` (single static file, no build step)
- Firmware: `BrainButtonInterface/hardware/cortex_button_esp32.ino`
  (button on GPIO 5, LED strip data on GPIO 7)
- Full setup, deployment, and CORS notes: `BrainButtonInterface/README.md` —
  read it before deploying; HTTPS pages cannot talk to a LAN device.

## SOUNDCONTROLTIMER — `SoundControlTimer/`

A giant 7-segment electronic clock for the main visualiser. It shows a
night that runs from **11 PM to 7 AM** — but time never passes on its
own: the clock is advanced only by the music, and it stops dead at 07:00.

Two ways to drive it:

- **Manual** — *Hour forward* (Space / N / →) glides the clock one hour
  ahead over a few seconds, then holds; ← / B steps an hour back.
- **Track** — load the show track in the page; it maps linearly onto the
  night (track start = 23:00, track end = 07:00). Playing, pausing and
  scrubbing all move the clock.

Display options: LED colour, 12/24-hour, seconds on/off, size. `H` hides
all chrome for the projector. `S` is the shutdown — everything stops and
the screen goes fully black; `R` restarts the night at 11 PM and plays
the track again from the top. Night progress reads as a segmented machine
meter along the bottom — discrete cells that snap on one by one, the
boundary cell flickering, with occasional momentary dropouts. The whole
display runs cold and machine-like: red LED by default (ice/amber/green
selectable), scanlines, and an irregular power-flicker in the digits.

One static file: `SoundControlTimer/index.html`. Alongside it,
`SoundControlTimer/timecode-deck.html` is DECK, the show-control display
(live feed / timecode / rail) the night clock grew from — standalone,
no hardware.

## Visual material

Concept art and reference footage, not referenced by either page:

- `DRAMA_welcome.png`, `DRAMA_drowning.png`, `DRAMA_tunnel.png`,
  `DRAMA_codedhuman.png` — halftone concept stills
- `A_black_and_white_halftone_vid.mp4` — halftone video test
