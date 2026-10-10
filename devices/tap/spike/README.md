# TAP MPE spike: how to test it

Ticket 01's throwaway device. It detunes notes with per-note pitch bend so we can see whether that reaches Wavetable. Takes about 15 minutes.

## 1. Get the files

1. In Cursor, switch to the branch `claude/codebase-vst-ableton-puxbqz` and pull.
2. Optional: in Cursor's terminal, from the repo root, run `npm test --prefix m4l`. The last lines should show `# fail 0`.

## 2. Add the folder to Live (once)

1. Open Live 11 and a new empty set.
2. In the browser on the left, under **Places**, click **Add Folder…**.
3. Choose the `m4l` folder inside your `next-annoying-piano` checkout. It now appears under Places.

## 3. Load the spike

1. Create a MIDI track (**Create > Insert MIDI Track**) and arm it.
2. From **Instruments**, drag **Wavetable** onto the track. The default sound is fine. Its title bar should show **MPE**.
3. Under **Places > m4l > spike**, drag **TAP MPE Spike.amxd** onto the track, to the **left** of Wavetable.
4. The device shows **Detune** (cents) and **Keys** (Black keys, All keys).
5. Play a few notes. You should hear Wavetable.

**If no sound comes out or the device shows an error:**

1. Click the **Edit** button in the device's title bar (the small icon at top right). The Max editor opens.
2. In the Max editor, open **Window > Max Console** and look for red lines.
   - `js: can't find file tap-spike.js`: in the Max editor choose **Options > File Preferences**, click **+**, add the `m4l/spike` folder, close the window, then delete the device from the track and drag it in again.
   - Anything else: copy the red lines into your reply and stop here.
3. If Live refuses to load the device at all, copy the message into your reply. I'll send a build-it-by-hand guide instead.

## 4. The three checks

### Check 1: each note has its own pitch

1. Set **Keys** to **Black keys** and **Detune** to **50**.
2. Hold **C** (white key) and keep holding it.
3. While C is held, play and hold **C#**.

**Pass:** C# sounds a quarter tone sharp and C does **not** move when C# comes in. **Fail:** C bends as well, or nothing is detuned at all.

### Check 2: the bend range

1. Drag **Audio Effects > Tuner** onto the track, to the right of Wavetable.
2. Set **Detune** to **100**. Play and hold **C#** on its own. Note what the Tuner reads.
3. Set **Detune** to **-100**. Play and hold **C#** again. Note what the Tuner reads.

**Pass:** the Tuner reads **D** the first time and **C** the second, each within a few cents. Otherwise, write down exactly what it reads (note and cents) both times.

### Check 3: timing

1. Set **Detune** to **0** and **Keys** to **All keys**.
2. Play fast runs, repeated notes and chords for a minute.
3. Turn the device off with the round button at the left of its title bar and play the same things.

Tell me whether the notes feel later or less steady with the device on. "Can't tell the difference" is a useful answer.

### Optional: only if check 1 fails

In the Max editor, open **View > Patcher Inspector** with nothing selected, and look for **Patch supports MPE**. Tell me whether it is ticked. Then untick it, save (**File > Save**), drag the device in again and repeat check 1.

## 5. What to send back

- Check 1: pass or fail, and what you heard.
- Check 2: the two Tuner readings.
- Check 3: your impression of the timing.
- Your Live version (**Live > About Live**) and Max version (shown at the top of the Max Console, or **Max > About Max** in the Max editor).
- Anything odd, such as stuck notes or red lines in the Max Console.

## Notes

- If a note gets stuck, turn the device off and on again.
- When a pull changes `tap-spike.js`, the device reloads it by itself but forgets its settings. Nudge Detune and Keys once afterwards.
- Don't save changes to the device from the Max editor unless a step above asks you to. The repo copy is rebuilt from `TAP MPE Spike.maxpat`.
