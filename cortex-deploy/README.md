# CORTEX·01

Spinning 1-bit brain + ESP32 press counter. One static file, no build step.

Talks to the ESP32 over Wi-Fi using the same API as the Streamlit
dashboard:

```
GET /counter -> {"counter": 12}
GET /reset   -> {"counter": 0}
```

The device count is authoritative. The page adopts whatever the ESP32
reports rather than keeping its own tally, so a browser refresh, a second
open tab, or an ESP reboot can never leave the two disagreeing.

## Read this before deploying

A page served over `https://` is not allowed to call `http://192.168.0.59`.
Browsers block it as mixed content and the request never leaves the tab.
There is no header, flag, or CORS setting on the ESP32 that changes this.

So there are two ways to run this, and they do not mix:

**On your network, talking to the hardware — use plain HTTP.**

```bash
python3 -m http.server 8000
```

Then open `http://<your-computer-ip>:8000` — not `localhost` if you want
to view it from a phone. This is the mode you want on the day.

**Deployed to Vercel — a public demo, no hardware.**

```bash
npx vercel --prod
```

Clicking the brain and pressing space still fire the neurons, so the page
is fully usable as a portfolio piece. It simply cannot see a device on
your LAN. If you connect anyway, the page detects the situation and says
so rather than sitting on a silent failure.

Wanting both at once means putting the ESP32 behind something with a
certificate — a small reverse proxy on the LAN, or a tunnel like
Cloudflare Tunnel or ngrok pointed at the ESP, then entering that
hostname in the page instead of the IP.

## The CORS header

Your Streamlit app works without any CORS setup because Python fetches
server-side, where the same-origin policy does not apply. JavaScript in a
browser tab is subject to it.

So the sketch must answer with:

```
Access-Control-Allow-Origin: *
```

Without it the ESP32 replies normally, you will see the request succeed in
the Serial Monitor, and the browser will still discard the response. The
included sketch sends it on every route. If you keep your own sketch, add
that one header to `/counter` and `/reset`.

## Using it

1. Flash `hardware/cortex_button_esp32.ino` with your Wi-Fi credentials.
2. Read the IP from the Serial Monitor.
3. Enter it in the field at the top of the page and press Connect.

The address is remembered for next time. Polling runs at 700 ms; a press
detected between two polls fires the neurons for exactly one second. If
several presses land in the same interval, the page fires a short burst
rather than collapsing them into one.

`Reset count` asks the ESP32 to zero itself and lets the next poll carry
the new value back, so the display and device stay in step. If the ESP
doesn't answer, the on-screen number is deliberately left alone.

## Controls

| | |
|---|---|
| Spin | rotation speed, 0 to stopped |
| Dot size | render resolution — larger dots, coarser screen |
| Tone | gamma and lift, shifts the black/white balance |
| Screen | clustered halftone or Bayer |
| BG | black or transparent (checkerboard confirms the alpha) |
| Invert | black ink on white |
| Test fire | fires the neurons without hardware |

Space or a click on the brain also counts as a press.

## Hardware

Button on GPIO 5 to GND, internal pull-up, 50 ms debounce.

A WS2812B strip (100 LEDs, data on GPIO 7) visualizes presses: each press
launches a pulse that travels down the strip with a fading trail, up to 8
pulses in flight at once. The pulse color starts white and shifts toward
full red as the count approaches 300 presses (`TRANSITION_PRESSES`).
