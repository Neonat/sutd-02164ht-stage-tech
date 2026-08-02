# dramaturgh

Stage-tech experiments for a theatre piece. Three self-contained
projects, plus shared visual material.

## CORTEX·01 — `cortex-deploy/`

A spinning 1-bit halftone brain wired to a physical button. An ESP32
counts presses, drives a WS2812B LED strip (each press fires a pulse that
shifts from white to red as the count climbs), and serves the count over
Wi-Fi:

```
GET /counter -> {"counter": 12}
GET /reset   -> {"counter": 0}
```

`cortex-deploy/index.html` polls the device and fires the on-screen
neurons on every press. The device count is authoritative — the page
adopts whatever the ESP32 reports.

- Web page: `cortex-deploy/index.html` (single static file, no build step)
- Firmware: `cortex-deploy/hardware/cortex_button_esp32.ino`
  (button on GPIO 5, LED strip data on GPIO 7)
- Full setup, deployment, and CORS notes: `cortex-deploy/README.md` —
  read it before deploying; HTTPS pages cannot talk to a LAN device.

## DECK — `timecode-deck.html`

A standalone show-control style display: live feed, timecode, and rail in
a black-on-black monospace aesthetic. Pure HTML/CSS/JS in one file — open
it directly in a browser. No hardware and no relation to the CORTEX
counter.

## SOUNDCONTROLTIMER — `SoundControlTimer/`

A giant 7-segment electronic clock for the main visualiser. It shows a
night that runs from **11 PM to 7 AM** — but time never passes on its
own: the clock is advanced only by the **stages of the music**, and it
stops dead at 07:00.

Two ways to drive it:

- **Manual** — the operator fires *Next stage* (Space / N / →) as the
  live music reaches each section; the clock glides to that stage's time
  over a per-stage travel duration, then holds.
- **Track** — load the show track in the page; each stage has a cue
  point (m:ss) and the clock is welded to the track — playing, pausing
  and scrubbing all move the night.

The stage list (name, clock target, travel, cue) is editable in the
on-screen panel, saved in the browser, and exportable as JSON. Display
options: LED colour, 12/24-hour, seconds on/off, size. `H` hides all
chrome for the projector. `S` is the shutdown — everything stops and the
screen goes fully black; `R` restarts the night at 11 PM and plays the
track again from the top. Night progress reads as a liquid-filled tube
along the bottom of the screen. One static file: `SoundControlTimer/index.html`
(`timecode-deck.html` alongside it is the reference copy of DECK it grew
from).

## Visual material

Concept art and reference footage, not referenced by either page:

- `DRAMA_welcome.png`, `DRAMA_drowning.png`, `DRAMA_tunnel.png`,
  `DRAMA_codedhuman.png` — halftone concept stills
- `A_black_and_white_halftone_vid.mp4` — halftone video test
