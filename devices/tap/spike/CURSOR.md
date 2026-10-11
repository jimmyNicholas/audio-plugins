# Cursor: help test the TAP MPE spike

Instructions for the Cursor agent. To start, tell Cursor: **"Read `m4l/spike/CURSOR.md` and help me test the spike."**

## Your role

You are helping the user run ticket 01's checks in Ableton Live 11. You cannot see or control Live or Max, so the user does those steps and tells you what happened. Your jobs:

1. Check the repo is ready before the user opens Live.
2. Walk the user through `m4l/spike/README.md` **one step at a time**, waiting for their answer before the next step.
3. Help when something goes wrong, using the troubleshooting below.
4. Write the results into `m4l/spike/RESULTS.md` in the format below.

Background, if you need it: `docs/m4l/tickets/01-mpe-spike.md` (what the spike must prove) and `docs/m4l/HANDOFF.md` (open questions).

## Rules

- Do **not** change `tap-spike.js`, the `.maxpat`, the `.amxd` or the tests. This is a test of what is committed. If you think a fix is needed, describe it in `RESULTS.md` instead.
- Do not commit or push unless the user asks.
- Australian spelling, no em dashes. Short, plain steps.

## 1. Before Live: check the repo

Run these from the repo root and report anything that is not clean:

```sh
git checkout claude/codebase-vst-ableton-puxbqz
git pull origin claude/codebase-vst-ableton-puxbqz
git status --short
node --version                 # needs 18 or later
npm test --prefix m4l          # expect "# fail 0"
```

`git status --short` should print nothing. If `m4l/spike/TAP MPE Spike.amxd` shows as modified, ask the user before discarding it (see "If the device was re-saved in Max").

Then find the versions, so the user does not have to:

```sh
defaults read "/Applications/Ableton Live 11 Suite.app/Contents/Info.plist" CFBundleShortVersionString
find "/Applications/Ableton Live 11 Suite.app" -path "*Max.app/Contents/Info.plist" -maxdepth 6 \
  -exec defaults read {} CFBundleShortVersionString \;
```

If the app has a different name, run `ls /Applications | grep -i live` and adjust. Record both versions. The second is the Max version bundled with Live.

Finally, give the user the full path of the `m4l` folder (`pwd` from the repo root, plus `/m4l`), since they add it to Live's Places in the next step.

## 2. Walk through the README

Follow `m4l/spike/README.md` from section 2 ("Add the folder to Live"). Give one numbered step or one check at a time. After each check, ask exactly what the README asks for (pass or fail and what they heard, or the Tuner readings) and write it down before moving on.

For check 2, if the readings are off, work out the actual bend range from them and note it. The device assumes ±48 semitones, so +100 cents should read one semitone up. For example, if +100 cents reads as only about +4 cents, Live is using a ±2 semitone range (100 × 2 / 48 ≈ 4).

## 3. Troubleshooting

**`js: can't find file tap-spike.js` in the Max Console.** Follow the README fix (add `m4l/spike` in Max's **Options > File Preferences**). Confirm the path you give the user exists with `ls m4l/spike/tap-spike.js`.

**Other red lines in the Max Console.** Ask the user to copy them in. Look them up against `m4l/spike/tap-spike.js` and `TAP MPE Spike.maxpat`, explain what they mean, and record them verbatim in `RESULTS.md`. Do not edit the files.

**Live will not load the device.** Record the exact message. Stop the walkthrough; the next session will send a build-it-by-hand guide.

**Stuck notes.** Tell the user to turn the device off and on. Note which check it happened in and what they were playing.

**If the device was re-saved in Max.** That is useful information (open question 4 in the handoff). Show what Max changed, without touching the committed `.maxpat`:

```sh
mkdir -p /tmp/tap-spike
node m4l/tools/amxd.mjs unpack "m4l/spike/TAP MPE Spike.amxd" /tmp/tap-spike/resaved.maxpat
diff <(python3 -m json.tool "m4l/spike/TAP MPE Spike.maxpat") <(python3 -m json.tool /tmp/tap-spike/resaved.maxpat)
```

Summarise the differences in `RESULTS.md` (a short summary, not the whole diff). Then ask the user whether to keep the re-saved file aside (`cp "m4l/spike/TAP MPE Spike.amxd" /tmp/tap-spike/`) and restore the committed one (`git checkout -- "m4l/spike/TAP MPE Spike.amxd"`).

## 4. Write the results

Create `m4l/spike/RESULTS.md`:

```md
# Ticket 01 results

Date:
Live version:
Max version:
Did the generated .amxd load without a re-save in Max? yes / no (details)

## Check 1: independent pitch
Result: pass / fail
What the user heard:

## Check 2: bend range
Detune +100, C#: Tuner read
Detune -100, C#: Tuner read
Implied bend range:

## Check 3: timing
Impression:

## Optional check (only if check 1 failed)
"Patch supports MPE" was ticked: yes / no
Result after unticking:

## Errors and oddities
(Max Console lines verbatim, stuck notes, anything else)

## Suggested fixes (not applied)
```

When it is filled in, show it to the user and tell them to paste it back to Claude, or ask them whether to commit it.
