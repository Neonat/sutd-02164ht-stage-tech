# SUTD 02.164HT — stage technology

Stage visuals and physical interaction experiments for the final performance in **02.164HT Dramaturgy**, taught by **Prof Nien**. Previously named `dramaturgh`.

The repository contains two independent browser displays: a button-driven brain visual and a music-controlled night clock. Operators can rehearse them locally, project the visuals during the performance, and optionally connect the brain interface to ESP32 hardware. Shared concept artwork is kept alongside the runnable projects.

## Run the browser displays

No frontend build is required. With Python 3 installed, serve the repository over HTTP:

```sh
git clone https://github.com/Neonat/sutd-02164ht-stage-tech.git
cd sutd-02164ht-stage-tech
python3 -m http.server 8000
```

- [Brain interface](BrainButtonInterface/index.html): open http://localhost:8000/BrainButtonInterface/.
- [Night clock](SoundControlTimer/index.html): open http://localhost:8000/SoundControlTimer/ and load the show track or use manual controls.
- [Timecode deck](SoundControlTimer/timecode-deck.html): open http://localhost:8000/SoundControlTimer/timecode-deck.html.

For the hardware mode, flash the [ESP32 sketch](BrainButtonInterface/hardware/cortex_button_esp32.ino) with the device's Wi-Fi credentials, read its IP in the Serial Monitor, and connect from the brain page on the same network. The sketch uses the FastLED library for the LED strip. See the [hardware and deployment guide](BrainButtonInterface/README.md) for wiring, CORS and the HTTPS/LAN restriction.

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
- Full setup, deployment, and CORS notes: [BrainButtonInterface/README.md](BrainButtonInterface/README.md) —
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

## Architecture and boundaries

Each display owns its HTML, styles, rendering and controls in a single page. Only the brain page uses the ESP32 HTTP counter; the clock and timecode deck run independently without hardware. The clock follows a loaded track's playback position or explicit manual cues, rather than detecting beats from ambient sound.

The concept stills and video below are reference material, not generated runtime output or evidence of a deployed installation. The browser-only brain demo can fire visually without hardware; an HTTPS-hosted page cannot directly poll the ESP32's HTTP LAN address.
