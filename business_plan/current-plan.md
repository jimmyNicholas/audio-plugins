# Current plan

Version 2, written 2026-11-09. Replaces the [first plan](history/2026-11-08-first-plan.md) of 2026-11-08. The research behind it is in [`research/`](research/), the reasoning for each choice in [`decisions.md`](decisions.md), and the way it came about in [`conversation-summary.md`](conversation-summary.md).

Status words used below: **Decided**, **Leaning** (current preference), **Open**.

## In one paragraph

Release work in **albums**: themed sets of small, quick, unique plugins, released together with music made with them and a sample pack made from them. Each album sits on **my own website**, where the story and design philosophy behind the tools are told. That story is the marketing and the discovery: it gives search engines something to rank, gives people something to share, and bridges the music and software audiences. Every plugin has a free browser version on its page. People can buy **singles**, an **album**, or the **discography** of everything, all cheaply. The checkout is rented from a store that handles tax and delivery, so I own the site, the story and the email list without becoming a tax collector in every country. Releases are regular but not on a fixed schedule: an album goes out when it passes its checklist.

## 1. What a release is

**Decided.** An album contains:

| Part | What it is | Why |
| --- | --- | --- |
| Plugins | Usually 3 to 6 small tools around one theme | The paid product |
| Free browser versions | One per plugin, on its page | Try before buying; SEO; shareable |
| Music | A few tracks, or an EP, made mainly with the album's tools, with timestamps where each tool is heard | Shows what the tools are for; brings listeners in |
| Sample pack | Bounces made with the tools | Sits between the music and producer audiences |
| Story | Liner notes, a "why" and "prototype" note per plugin, optionally an essay | The marketing and discovery |
| Art | Cover and banner | Identity and social previews |

The full checklist and templates are in [`templates/`](templates/).

## 2. Pricing

**Leaning.** Three tiers, all cheap:

| Tier | Contents | Guide price (AUD) |
| --- | --- | --- |
| Single | One plugin | A$5 to A$12 |
| Album | Everything in one album | Less than its singles added up, about half |
| Discography | Every album | A discount that grows with the catalogue, as Bandcamp labels do (starting near 50% off, towards 90%) |

**Open: what the discography includes.**
- **"Everything so far":** a fixed bundle, repriced at each release. Simple; buyers come back for new albums.
- **"Everything, including future albums":** pay once, receive every later album. Bigger sale up front, but every future album is then free to that buyer. On itch this works as one project that gets new files with each release; itch's docs say earlier buyers get new files automatically, unless a file's minimum price is above what they paid ([research](research/bandcamp-discography-model.md), section 1.7).
- A middle option: future-inclusive for a year, then it becomes "everything so far".

**Decided against:** subscriptions and Patreon. They clash with the dislike of ongoing obligations and need a large following ([research](research/indie-plugin-monetisation.md)).

**Worth adding (Leaning):** one free "taster" plugin now and then, built from earlier albums. Most indie plugin makers use a free plugin, not a browser demo, as their funnel (Valhalla, Tokyo Dawn Labs, Kilohearts, Aberrant DSP). Keep the browser versions, and measure whether they lead to sales.

## 3. Where things are sold

**Decided:** my own site is the home and the shop window. The domain stays with VentraIP.

**Leaning:** rent the checkout.

| Option | Role | Notes |
| --- | --- | --- |
| **itch.io behind the site** | Start here | Embedded buy buttons; itch handles payment, VAT in Payouts mode, and file delivery; supports the growing-project discography. Checkout looks like itch. |
| A merchant-of-record overlay checkout (Payhip, Lemon Squeezy, Gumroad) | Alternative or fallback | Buyers stay on the site. Research found risks: Lemon Squeezy moving users to Stripe, Gumroad payout problems for one seller, Paddle wanting custom terms under US$10. Payhip bundles deliver later products to existing buyers. |
| Stripe directly | Only if sales grow | Lowest fees and most control, but I become responsible for GST, VAT and similar taxes abroad. |
| **Bandcamp** | Second store, for music and sample packs only | Plugins are not a normal Bandcamp product. Strong for audio; collects GST for Australian buyers; follower emails; discography offer up to 90% off; Bandcamp Fridays. |
| itch.io as a plain listing too | Optional extra shop window | Costs nothing; some people only shop there. |
| Steam | Later, maybe | US$100 per product, so one collection at most. |
| Shopify | Rejected | Monthly fee whether or not anything sells. |

**Open:** which checkout reliably serves an Australian seller at A$5 price points in 2026, and how it hands over the details needed to issue a licence file.

## 4. Website

**Decided:** built by Jimmy, static, hosted free (Cloudflare Pages builds from a private repo).

Pages:
- **Album page:** theme, liner notes, tracks with "heard at" timestamps linking to each plugin, buy album, buy discography.
- **Plugin page:** free browser version, before and after audio, why it exists, the prototype it started as, buy single, "part of album X".
- **Writing:** essays, many grown from the research in [`docs/research/`](../docs/research/). These rank, get shared, and can't be copied.
- **Discography page:** everything, with the current price.
- **Email sign-up on every page.** The email is a release letter: here's the record, the tools, and what I was chasing.

Tracking: count clicks from browser versions to the store, so the funnel can be judged on evidence.

## 5. Building the plugins

- **Prototyping (Decided):** Max for Live, for quick spikes.
- **Framework (Leaning):** JUCE. The free Starter tier covers revenue up to US$20,000 a year, with no splash screen.
- **One sound, two builds (Decided):** each plugin's DSP is shared between the browser version and the plugin. Path A: plain C++ compiled with Emscripten for the browser and wrapped in JUCE. Path B: Faust or Cmajor exporting to both. **Open:** try one small effect each way before choosing.
- **Platforms (Decided):** Windows first, VST3 in a zip (no paid signing available to Australians through Azure). Mac once there is demand (A$149 a year Apple fee). Linux if cheap. No AAX.
- **Repo (Decided):** private. **To do:** change in GitHub settings.

## 6. TAP

**Open, leaning:** sell TAP as a Max for Live device now, on my own site or itch, and pitch it to Isotonik. Ableton cannot chain a plugin's MIDI output into the next device, so a VST3 version of TAP would not work well in Live. A plugin version for Bitwig, Reaper and Logic can come later. Freezing does not stop buyers opening the patch.

## 7. Copy protection

**Leaning:** one signed licence file per buyer, listing everything they own (one file covers a discography buyer). Checked offline, works on all the buyer's own computers, with their name shown in the about box. No checks while the plugin runs. This matches u-he, Sonic Charge and Vital. Online activation only if sharing becomes a real problem.

## 8. Cadence

**Decided:** regular, not scheduled. An album releases when its checklist passes. Keep at least one album in progress so gaps stay short. Where possible, release music and sample packs on a Bandcamp Friday.

## 9. Release pipeline

Each album has an `album.yaml`; each plugin has a `release.yaml`. Templates and a worked example are in [`templates/`](templates/).

```
plugin: idea -> building -> ready          album: draft -> ready -> released
```

**Checks before an album can be `ready`:**
- every plugin it lists passes its own checks: builds, DSP tests, pluginval, browser version loads and makes sound, licence accepts a test file and rejects a tampered one, story and media present
- every file named in the manifests exists
- liner notes written; every track has at least one "heard" timestamp
- album price below the singles added up
- SEO description 155 characters or fewer
- store drafts created (checkout, and Bandcamp if the album has music or samples)

**On release (run by hand):** build and package every plugin, upload files to the checkout, generate the album and plugin pages, link timestamps between them, update the discography page and price, mark the album `released`, and draft the email to the list.

**Stays manual:** creating store pages, Bandcamp uploads (no upload API found), writing the email.

**Keeping CI cheap:** Linux runners for anything that can run on Linux; Windows builds on my own PC as a self-hosted runner; Mac builds only on release.

## 10. Costs

| Phase | Fixed cost a year | Notes |
| --- | --- | --- |
| 1: Windows only | About A$0 beyond the domain | Hosting, email list, analytics, JUCE Starter and stores all free until something sells |
| 2: add Mac | About A$150 | Apple Developer Program, plus a Mac if neither collaborator has one |
| Later, if needed | Varies | Windows signing certificate, JUCE Indie past US$20k revenue, paid email tier, Steam US$100 per product |

Per-sale fees: itch takes the share I set plus about US$0.30 + 2.9%; Bandcamp takes 15% (10% past US$5,000 in a year) plus processing. Flat per-sale fees hurt at A$5, which is why albums and the discography are the main offers.

**Expectations:** hobby-sized. In one survey of 95 small plugin firms, about half were side projects under US$20,000 a year. The research's guess for this path is A$500 to A$5,000 a year once there are 8 to 12 plugins, with wide uncertainty.

## 11. Working with a friend

**Open.** Agree in writing before money comes in: revenue split, who receives payouts, who owns the code, each plugin and the brand, and what happens if one person stops. Use a shared brand name. itch does not split a single project's sales between people; payouts go to one account and are shared by agreement. Ask an accountant whether it counts as a partnership.

## 12. Tax and admin (Australia)

GST registration is only needed from A$75,000 turnover a year. Declare the income; an ABN helps. itch (Payouts mode) and Bandcamp collect VAT and similar taxes for many buyers. Ask an accountant how itch Payouts and Bandcamp are treated for an Australian seller.

## 13. Open questions

1. Which checkout to start with, given Australian payouts and A$5 prices.
2. How the chosen checkout passes buyer details so a licence file can be issued.
3. Discography: "everything so far" or "including future albums".
4. Framework path A or B.
5. TAP: Max for Live only, or Max for Live plus a plugin for other hosts; Isotonik's terms.
6. Whether browser versions convert to sales (measure from the first album).
7. Collaborator agreement.
8. Confirm with Bandcamp's own help pages that plugins can't be sold there (the research only saw search extracts).

## 14. Next steps

1. Make this repo private.
2. Try one tiny effect in both framework paths and pick one.
3. Choose the first album's theme (Slush Wave ideas fit) and fill in its `album.yaml` from the template.
4. Build the site's album and plugin page templates.
5. Set up the checkout with a cheap test product, including a test purchase of a growing project.
6. Write the manifest checker and push checks.
7. Release album 1.
