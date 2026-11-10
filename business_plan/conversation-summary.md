# How the plan came about

A summary of the planning conversation of 2026-11-08 and 2026-11-09, in order. It records the reasoning, so the decisions make sense to someone who was not there (for example, a collaborator).

## 1. Starting point: how do I make money from this?

The repo held one finished Max for Live device (TAP, an MPE piano that retunes as you play) and one paused one (Slush Wave). First advice covered ways to sell Max for Live devices: selling direct, publishers such as Isotonik, a free lite version on maxforlive.com, Patreon, and porting to plugin formats later. Two constraints came up: `.amxd` files can be opened by anyone, and the audience is limited to Live Suite users.

## 2. Move to JUCE, many small cheap plugins

Jimmy decided Max is for quick spikes and the products would be real plugins, and that the repo would go private. Discussion covered frameworks (JUCE, nih-plug, iPlug2, RNBO), the Ableton problem for TAP (Live cannot chain plugin MIDI effects), code signing, CI costs and bundles.

## 3. The funnel and copy protection

Jimmy proposed free browser versions on a website as the funnel into paid plugins, with the paid plugin selling convenience. Key ideas: write each sound once and build it for both browser and plugin; the browser version is naturally limited (no DAW, no automation), so it needn't be crippled. Copy protection was laid out as a ladder from none to iLok; the choice was light protection (a signed licence file), since everything popular gets cracked and honest buyers are the ones who pay. A first costing put the whole funnel at about A$0 a year beyond the domain, plus Apple's fee for Mac.

## 4. Windows first, itch, a release bank

Mac signing being the main cost, Windows goes first. Shopify was ruled out for its monthly fee and itch.io favoured. Inspired by Sokpop (Stacklands), the idea of a bank of plugins released automatically on a schedule took shape, with a manifest per plugin and checks that block anything incomplete. A yearly pass replaced the idea of a subscription. A possible collaborator was discussed.

## 5. First plan written, then checked

The first plan ([`history/2026-11-08-first-plan.md`](history/2026-11-08-first-plan.md)) was written and checked against sources. Four things said earlier were wrong: itch doesn't split a project's sales between people, Azure signing isn't open to Australians, Sokpop dropped its monthly schedule, and JUCE's free tier is capped at US$20,000.

## 6. Research: how small developers make money

[`research/indie-plugin-monetisation.md`](research/indie-plugin-monetisation.md) surveyed 26 developers. Main lessons: a free plugin is the usual funnel; light copy protection is normal among respected makers; Puremagnetik's monthly releases plus singles plus catalogue bundle is the closest match; the fallback stores are riskier than assumed; and income at this scale is hobby-sized.

## 7. Research: the Bandcamp discography model

Prompted by Geometric Lullaby and signalwave, [`research/bandcamp-discography-model.md`](research/bandcamp-discography-model.md) looked at selling many small items singly or as a cheap discography. Bandcamp doesn't accept plugins as normal products, but suits music and sample packs. The useful find: itch delivers new files on a project to earlier buyers, which makes a future-inclusive pass possible.

## 8. Own website, albums

Jimmy proposed his own site, releasing in themed albums with singles and discography options, on a regular but not fixed cadence. The checkout stays rented (itch behind the site, or a merchant-of-record checkout), so tax and delivery are handled by someone else while the site, story and email list are owned.

## 9. The story is the marketing

Jimmy observed that on itch the tools do the talking, while his own site can show the journey and bridge music and software. Each album pairs a record with its tools, the sample pack sits between the audiences, and the existing research docs can become essays. The story is part of the process, and it is the marketing and discovery.

## 10. What an album needs

A worked example (album 1, "Melting Point", from the Slush Wave ideas) set out the folder layout, an `album.yaml`, a plugin `release.yaml`, what each piece costs in effort, and the checks an album must pass. These became [`templates/`](templates/), and the whole conversation was organised into this folder.
