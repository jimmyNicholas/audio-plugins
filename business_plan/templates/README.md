# Album and plugin templates

What is needed to create an album, as templates to copy. `albums/` now lives at the repo root (album 1 is [`albums/01-bloodline-breaks/`](../../albums/01-bloodline-breaks/)); `plugins/` joins it there with the first JUCE spike.

```
albums/_template/              copy to albums/NN-<slug>/
  album.yaml                   the album manifest
  story/liner-notes.md         300 to 800 words
  story/essay.md               optional longer piece
  art/                         cover.png (3000 x 3000), banner.png
  music/                       the tracks, as WAV
  samples/                     the sample pack, as a zip

plugins/_template/             copy to plugins/<slug>/
  release.yaml                 the plugin manifest
  story/why.md                 why it exists
  story/prototype.md           what it started as
  media/                       screenshot.png, before.mp3, after.mp3
  (dsp/ plugin/ web/)          code, added when the codebase exists
```

A plugin can belong to more than one album. Albums refer to plugins by slug.

## What each piece takes

| Piece | What it is | Effort |
| --- | --- | --- |
| Plugins (3 to 6) | Code, tests, browser version | Most of the work |
| "Why" note per plugin | One or two paragraphs | Minutes each |
| Prototype note per plugin | What the Max patch was, what failed | A paragraph each |
| Before and after audio | Comes out of testing | Minutes |
| Tracks | Music made with the tools | Creative time |
| "Heard" timestamps | Noted while mixing | Minutes |
| Liner notes | The album's story | An evening |
| Essay (optional) | The deeper idea | An evening or two |
| Sample pack | Bounces from the tools | An evening |
| Cover and banner | Artwork | Varies |
| SEO title and description | One line each | Minutes |
| Store drafts | Checkout page, and Bandcamp if used | Half an hour |

## Checks

**A plugin can be `ready` when:**
- it builds for every platform listed
- its DSP tests pass
- pluginval passes at the agreed strictness
- the browser version loads and makes sound in a headless browser
- the build accepts a test licence file and rejects a tampered one
- `why.md`, `prototype.md`, the screenshot and the before and after audio exist
- the SEO description is 155 characters or fewer

**An album can be `ready` when:**
- every plugin it lists is `ready`
- every file named in `album.yaml` exists
- the liner notes are written (not the template text)
- every track has at least one "heard" timestamp, and each names a plugin in the album
- the album price is below its singles added together
- the SEO description is 155 characters or fewer
- the store drafts are created (`album_page` filled in; `bandcamp_draft: true` if the album has music or samples on Bandcamp)

**Release (run by hand):** build and package every plugin; upload files to the checkout; generate the album and plugin pages; link timestamps between them; update the discography page and price; set `status: released` and `release_date`; draft the email to the list.

## Worked example: album 1, "Melting Point"

Built from the Slush Wave ideas. Names, prices and text are placeholders.

```yaml
# albums/01-melting-point/album.yaml
number: 1
slug: melting-point
title: Melting Point
status: ready
release_date:

theme: >
  Slowing sound down until it loses its edges. Tools and music
  for the moment something solid turns to slush.

pricing_aud:
  album: 15                # singles: 9 + 7 + 7, plus samples 5 = 28
  discography_discount: 50

contents:
  plugins: [thaw, smear, drift]
  samples: samples/melting-point-samples.zip
  tracks:
    - file: music/01-thaw-season.wav
      title: Thaw Season
      heard:
        - { plugin: thaw,  at: "0:42", note: "the whole pad, slowed 4x" }
        - { plugin: drift, at: "2:10", note: "wobble on the lead" }
    - file: music/02-under-the-ice.wav
      title: Under the Ice
      heard:
        - { plugin: smear, at: "1:05", note: "piano smeared into a drone" }
    - file: music/03-meltwater.wav
      title: Meltwater
      heard:
        - { plugin: thaw, at: "0:00", note: "everything" }

story:
  liner_notes: story/liner-notes.md
  essay: story/essay.md
  related_writing:
    - docs/research/recent-ambient-practice.md

art:
  cover: art/cover.png
  banner: art/banner.png

seo:
  title: "Melting Point: slushwave tools and music"
  description: "Three free-to-try plugins for slowing and smearing sound, with the record made from them."
  keywords: [slushwave, ambient plugin, slowed reverb, tape wobble]

store:
  checkout: itch
  album_page: https://<brand>.itch.io/melting-point
  bandcamp_draft: true
```

```yaml
# plugins/thaw/release.yaml
slug: thaw
name: Thaw
one_liner: "Slows any sound down without the robot artefacts."
status: ready
albums: [melting-point]
price_aud: 9
platforms: [windows]
formats: [vst3]

story:
  why: story/why.md
  prototype: story/prototype.md

media:
  screenshot: media/screenshot.png
  before: media/before.mp3
  after: media/after.mp3

web_demo: true

seo:
  title: "Thaw: a free slow-down effect you can try in your browser"
  description: "Slow any sound to a crawl. Try it free online, then get the VST3."
```
