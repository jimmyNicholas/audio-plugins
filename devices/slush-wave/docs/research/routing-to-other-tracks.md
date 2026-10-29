# Sending a Slush Wave path to another track

Question: what are the ways to get one of Slush Wave's paths (the centre path or the sides path) onto another track in the Set, so it can be processed or mixed separately, and how good is each?

Researched 2026-10-29. Target: a Max for Live audio effect in Ableton Live 11 Suite with its bundled Max 8 (8.5.x), Intel Mac, macOS 13, runtime-only `gen~` licence (see [max-building-blocks.md](max-building-blocks.md)).

## How to read this

- **Method caveat:** the sandbox could not fetch `docs.cycling74.com`, `cycling74.com`, `www.ableton.com`, `help.ableton.com` or `www.metafunction.co.uk` (DNS blocked). Claims about those pages come from search-engine extracts of the named page, marked **[extract]**. Ableton's `Ableton/maxdevtools` repository on GitHub (commit 5b39dec, Sept 2026) was cloned and read directly, marked **[read]**. Reasoning by the researcher is marked **[inference]**.
- **Confidence:** High = a primary source states it. Medium = a primary source implies it, or only forum posts state it. Low = inference only.
- Many forum sources are from Live 8 to Live 10. Where nothing confirms Live 11 behaviour, that is said.

## Answer in short

| Route | Works in Live 11? | Timing | Latency compensated by Live? | Freeze / export | Verdict |
|---|---|---|---|---|---|
| Extra `plugout~` outputs (3/4) picked up by another track's **Audio From** chooser | Yes, since Live 10, for audio effects too | Same as the device's main output (inference) | Yes, through Live's normal routing graph (inference, Medium) | Export: normal Live routing, renders. Freeze: tracks linked by routing are reported unfreezable | **Use this** |
| Device pushes its extra outputs to a track via the LOM (`DeviceIO.routing_type`, Audio Routes or Surround Panner pattern) | Yes, LOM `DeviceIO` exists since Live 10 | Same as above | Same as above (inference) | Same as above | Optional convenience on top of the above; fiddly API |
| `send~` / `receive~` between devices on different tracks | Sometimes passes audio | Unpredictable: zero to about a buffer (reports of 4 to 22 ms) | No | Not reliable | **Do not use.** Cycling '74 says it is not supported |
| `plugsend~` / `plugreceive~` | Sometimes | Unpredictable | No | Not reliable | Not supported; staff warn of corruption |
| Rack chain plus Live's own routing (no Max) | Yes | Sample accurate | Yes | Same freeze caveat | Only gives Pre FX / Post FX / Post Mixer of a whole chain, not a tap inside a device |
| Named `buffer~` circular buffer shared between devices | Possible | At least one audio buffer, fixed only by design | No | Not reliable | Workaround only |

**Recommendation:** give the device a stereo output pair per path that can be sent away: `plugout~ 1 2 3 4 5 6` (main mix on 1/2, centre path on 3/4, sides path on 5/6), with a per-path switch "mix into device output / send to its own output". The user puts a new audio track's **Audio From** on the Slush Wave track, picks the Slush Wave 3/4 or 5/6 output in the lower chooser, and sets that track's Monitor to In. Do not use `send~`/`receive~`. Details and spike tests are in section 5.

## 1. `send~` / `receive~` between devices on different tracks

### 1.1 What Ableton and Cycling '74 say

- Ableton's production guidelines: "The 'name space' in Max is global ... you can share data between Max for Live devices. The 'signal processing space' is independent - each Max for Live device processes its audio data separately." **[read, High]** https://github.com/Ableton/maxdevtools/blob/main/m4l-production-guidelines/m4l-production-guidelines.md (section "Local and Global Naming")
- Max 8 "Limitations of Max for Live": "The use of the send~ and receive~ objects to pass audio between Max for Live devices is not supported." Message `send`/`receive` between devices is supported "but there may be some latency involved". **[extract, High]** https://docs.cycling74.com/legacy/max8/vignettes/live_limitations
- The older Max for Live audio device guide: "Sending audio to another Max for Live Audio Effect, Instrument, or MIDI Effect device using the Max send, receive, send~, or receive~ objects is not supported." **[extract, High]** https://docs.cycling74.com/legacy/max5/vignettes/core/live_audiodevices.html
- On `plugsend~`/`plugreceive~` a Cycling '74 staff member said they could give "unpredictable latencies all the way to data corruption" and there is "no easy way to pass audio data in Live predictably and reliably". **[extract, forum, Medium]** https://cycling74.com/forums/plugsendplugreceive, https://cycling74.com/forums/send-and-receive-on-different-ableton-tracks-won't-work

### 1.2 Timing in practice

- In plain Max, `send~`/`receive~` add no delay unless they close a feedback loop, where they cost one signal vector. **[extract, forum, Medium]** https://cycling74.com/forums/latency-of-send~-and-receive~-objects
- In Live the reports disagree: delay that followed the audio buffer size (about 12 ms at 512 samples, about 5 ms at 128), a measured 4 to 22 ms at a 96-sample buffer, and one user seeing zero (put down to both tracks running on the same thread). The thread title sums it up: latency is "basically not predictable". **[extract, forum, Medium]** https://cycling74.com/forums/latency-of-sendreceive-predictable, https://forum.ableton.com/viewtopic.php?t=130737
- Why **[inference, Medium]**: Live processes tracks in an order it chooses and, with multicore on, on several threads. Whether the sending device has run before the receiving device in a given audio buffer depends on that order, so the receiver gets either this buffer's audio or the previous one's (or a half-written one). Live does not know the two devices are linked, so it neither orders them nor compensates. Device order inside one track would be deterministic, but that is not the case being asked about.
- **Latency compensation:** none. Live only compensates latency it knows about (Defined Latency and its own routing graph). **[inference, High]**
- **Freeze, export, bounce:** freezing or exporting renders tracks in an order and at a speed Live chooses, possibly offline, so a hidden `send~` link can drop out, glitch or shift. No primary source covers this directly. **[inference, Medium]**

### 1.3 Multiple instances

- The `---` prefix is replaced "with a number that is unique to your device" when the patch loads, so it keeps a name **inside** one device. **[read, High]** (same guideline section). It cannot be used to pair a sender in one device with a receiver in another.
- A fixed global name (`send~ slushwave_sides`) collides as soon as two Slush Wave instances exist: two `send~` with one name is an error or last-wins situation in Max, and every receiver hears it. A pairing scheme (a user-set channel name or number shared by sender and receiver) avoids collisions but adds UI and still has the timing problems above. **[inference, Medium]**

**Verdict:** not supported, not sample-synchronous, not compensated. Rule it out.

## 2. Live's own routing

### 2.1 Taps a track can pick

- With another track chosen in **Audio From**, the lower **Input Channel** chooser offers Pre FX, Post FX and Post Mixer. "If a track has one or more Instrument or Effect Racks in its device chain, internal routing points (Pre FX, Post FX and Post Mixer) will also be available for every chain within the Rack." **[extract, High]** https://www.ableton.com/en/manual/routing-and-i-o/ (Live 12 manual; the same wording has been in the manual since well before Live 11, but the Live 11 page could not be fetched)
- There is no tap partway through a chain, so a point inside a device is out of reach; the usual workaround is splitting the chain across tracks. **[extract, forum, Medium]** https://forum.ableton.com/viewtopic.php?t=168210, https://forum.ableton.com/viewtopic.php?t=249650
- So a rack-only route would mean two copies of Slush Wave (or a crossover) on two Audio Effect Rack chains, one per path, with another track tapping a chain's Post FX. It works with full compensation but needs two device instances and the user to set the split up by hand. **[inference, High]**

### 2.2 Extra outputs from a Max for Live device

- Live 10 added multiple audio inputs and outputs for Max for Live devices, "which can be accessed via the track's Input and Output Channel choosers". **[extract, Medium; quoted on several third-party pages, original Ableton text not reached]** https://cycling74.com/articles/audio-routings-a-new-system-for-multi-channel-routing-in-ableton-live, https://cdm.link/free-multi-analyzer-shows-live-10-multichannel-max-cool/
- The channel count comes from the object arguments: `plugin~ 1 2 3 4` gives the normal stereo input plus a second pair (3/4), and the same for `plugout~`. A track's **Audio From** chooser can pick up a device's extra output on another track; **Audio To** on another track can feed a device's extra inputs. **[extract, forum, Medium; post by Cycling '74's developer]** https://forum.ableton.com/viewtopic.php?p=1729136
- Max docs: "Devices created with multi-channel inputs and outputs will support multichannel routing within Live", maximum 64 channels; `mc.plugin~`/`mc.plugout~` are the MC forms. **[extract, High]** https://docs.cycling74.com/max8/vignettes/mc_maxforlive_interface
- Routing is in **stereo pairs** (1-2, 3-4, 5-6, 7-8). The labels are Live's own ("device name | channel pair") and cannot be renamed from the device. **[extract, Medium]** https://cycling74.com/articles/audio-routes-using-audio-routes-tooling-for-your-mfl-devices, https://cycling74.com/forums/-plugout~-multiple-outputs
- **Audio effects too, not only instruments:** the Meta Function "Multi-band Junction" is a Max for Live audio effect for Live 10 that "split[s] an incoming audio signal into three bands, route[s] these bands to separate tracks". This is the same shape as Slush Wave's request. **[extract, Medium]** https://www.metafunction.co.uk/post/the-multi-band-junction-a-free-max-for-live-device-for-ableton-live-10
- Older pages that say Max for Live audio is limited to two channels (Max 5 to Max 7 era `plugout~` reference and the old device guide) predate Live 10 and are superseded. **[extract, Medium]** https://docs.cycling74.com/legacy/max7/refpages/plugout~
- **Latency and compensation [inference, Medium]:** this is Live's own routing graph, the same mechanism as a multi-out instrument's sub-outputs, so Live knows the destination depends on the source and orders and compensates it like any other track input. The extra outputs leave the device in the same audio block as 1/2, so the device's Defined Latency applies to them equally. No primary source states this for Max devices specifically; the spike must measure it.
- **Freeze [extract, forum, Medium]:** forum reports (Live 4 to Live 8 era) say that once audio is routed from one track to another, both source and destination become unfreezable, and a track with Monitor set to In cannot be frozen. https://forum.ableton.com/viewtopic.php?p=784306, https://www.soundonsound.com/techniques/track-freeze. Export (render) of the Set follows normal routing and so should include the routed paths **[inference, Medium]**; Ableton notes export only has delay compensation if compensation is on. https://help.ableton.com/hc/en-us/articles/209072409-Delay-Compensation-FAQ
- **Feedback caveat:** Live turns compensation off for tracks in a routing feedback loop (for example the receiving track sending back into a return that feeds the Slush Wave track). **[extract, High]** https://help.ableton.com/hc/en-us/articles/209072409-Delay-Compensation-FAQ

### 2.3 Device-side ("push") routing through the LOM

- The LOM has `MaxDevice.audio_inputs` and `audio_outputs`, lists of `DeviceIO`, each with `available_routing_types`, `routing_type`, `available_routing_channels`, `routing_channel` (dictionaries with `display_name` and `identifier`). `default_external_routing_channel_is_none` is marked as added in Live 11.0. **[extract, High]** https://docs.cycling74.com/apiref/lom/maxdevice/, https://docs.cycling74.com/apiref/lom/deviceio/
- So the device itself can set "outputs 3/4 go to track X" (the Surround Panner and Cycling '74 Audio Routes pattern). Routings "have to either be Live tracks or hardware I/O connections". **[extract, High]** https://cycling74.com/articles/audio-routings-a-new-system-for-multi-channel-routing-in-ableton-live
- `live.routing` (the UI object for this) handles only one port per device, so a device with two sendable pairs needs the Audio Routes abstractions or its own `live.object` / JavaScript code. Forum users hit "invalid id" errors and refusals unless the exact dictionaries from `available_routing_*` are passed back. **[extract, Medium]** https://docs.cycling74.com/reference/live.routing, https://cycling74.com/forums/max-4-live-live-10-deviceio-get-available_routing_channels-invalid-id, https://github.com/h1data/M4L-ioRouting-js
- Whether `live.routing` exists in Max 8.5 with Live 11 or is newer was not confirmed. **[Low]**
- **Verdict [inference]:** same audio path as 2.2 (the routing lives in Live), so the same timing. It only moves the routing choice from the destination track into the Slush Wave UI. Worth it later; not needed for the first version.

## 3. Other routes

- **Sidechain-style extra inputs (`plugin~ 1 2 3 4`):** another track's **Audio To** can target the device's inputs 3/4. That brings audio **into** Slush Wave, the opposite direction, so it does not help send a path away. **[extract, forum, Medium]** https://forum.ableton.com/viewtopic.php?p=1729136
- **"Receiver" devices:** third-party "sender/receiver" Max devices (for example the Zero Latency MIDI Router) get round inter-device timing by adding a fixed receiver latency (512 samples) and reporting it so Live compensates. That fixes the offset only if the buffer never exceeds the fixed figure, and it is MIDI, not audio. **[extract, Medium]** https://www.maxforlive.com/library/device/3626
- **Named `buffer~` as a circular buffer:** a Cycling '74 toolbox project shares audio through a buffer with minimum latency of "the size of your audio buffer minus 64 samples". Same unsupported, uncompensated territory as `send~`. **[extract, Medium]** https://cycling74.com/tools/multichannel-audio-in-maxforlive
- **Live 11 additions:** Live 11 lets MIDI be routed to and from Max for Live audio effects (MIDI from other tracks as a sidechain); nothing new for audio between tracks was found. **[extract, Medium]** https://www.ableton.com/en/blog/live-11-and-max-for-live-get-a-glimpse-of-whats-possible-with-some-free-devices/

## 4. What Live 11 does not offer

- No tap inside a device or between two devices of one chain (only Pre FX / Post FX / Post Mixer per track or rack chain). **[extract, High]**
- No custom names for a Max device's extra outputs. **[extract, Medium]**
- No supported sample-accurate wireless audio between Max devices. **[extract, High]**

## 5. Recommendation and spike

**Route:** extra `plugout~` stereo pairs, picked up by the destination track's Audio From chooser (section 2.2). Optionally later, a LOM-driven destination menu in the device (section 2.3).

**Shape [inference]:**
- `plugout~ 1 2 3 4 5 6`: 1/2 = device output, 3/4 = centre path, 5/6 = sides path.
- Per path, a switch: "Mix" sends the path into 1/2 (and silence to its pair); "Send" sends the path to its pair and removes it from 1/2. Crossfade the switch to avoid clicks.
- The path outputs leave in the same audio block as 1/2, so the device's Defined Latency covers them. If the main mix delays a dry signal to line up with `pitchshift~` latency, the sent paths must be aligned the same way.
- The receiving track: Audio From = the Slush Wave track, lower chooser = the Slush Wave output pair, Monitor = In. Tell the user the source track and the receiving track cannot be frozen; resample or bounce instead.

**Spike tests:**
1. Load a test device with `plugout~ 1 2 3 4` (sine on 3/4) as an **audio effect** in Live 11.3 on the target Mac. Does the pair appear in another track's Audio From lower chooser, and under what name?
2. Sample alignment: put an impulse on 1/2 and 3/4, record both (1/2 via Post FX of the source track, 3/4 via the device output) onto two audio tracks; compare offsets at buffer sizes 128 and 512, with multicore on and off.
3. Compensation: set Defined Latency to, say, 1000 samples and repeat test 2. Do both recorded tracks shift equally and stay aligned with a plain reference track?
4. Ordering: put the receiving track to the left of and to the right of the source track; put the source track in a Group. Any change?
5. Export: Export Audio/Video with the receiving track soloed and with "all individual tracks". Are the routed paths present and aligned?
6. Freeze: confirm which of the two tracks Live 11 refuses to freeze, and the message.
7. Two Slush Wave instances on two tracks: each lists its own pairs, no cross-talk.
8. Packing: `tools/amxd.mjs` writes the patcher JSON as is, so a `plugout~ 1 2 3 4 5 6` box with 6 inlets should just work; confirm the packed device loads without a re-save.
9. (Later) Read `audio_outputs` through `live.object` and set one pair's `routing_type` to a track; check `live.routing` is present in Max 8.5.

## Sources the sandbox could not fetch

`docs.cycling74.com`, `cycling74.com` (forums, articles, tools), `www.ableton.com` (manual, blog), `help.ableton.com`, `www.metafunction.co.uk`. Each claim from them is a search extract and should be checked in a browser before it is relied on for anything beyond the spike.
