# Decisions

Every decision so far, newest state first, with why it was made and what it replaced. See [`current-plan.md`](current-plan.md) for the plan these add up to.

Status: **Decided**, **Leaning** (current preference), **Open**, **Superseded** (replaced by a later decision), **Rejected**.

## Current decisions

| # | Decision | Status | Choice | Why | Date |
| --- | --- | --- | --- | --- | --- |
| 1 | Prototyping tool | Decided | Max for Live for quick spikes | Fastest way to try ideas; `.amxd` files are readable and only reach Live Suite users, so not the main product | 2026-11-08 |
| 2 | Shipping framework | Leaning | JUCE | Industry standard, most help, builds every format; free tier to US$20k a year | 2026-11-08 |
| 3 | Shared DSP for web and plugin | Decided | Write each sound once, build it for the browser and the plugin | Otherwise every plugin costs double to maintain | 2026-11-08 |
| 4 | Framework path | Open | Plain C++ plus Emscripten (A) or Faust/Cmajor (B) | Try one small effect each way first | 2026-11-08 |
| 5 | Repo visibility | Decided | Private (to do in GitHub settings) | Code is the paid product | 2026-11-08 |
| 6 | Platforms | Decided | Windows first, Mac when there is demand, Linux if cheap, no AAX | Windows needs no paid signing; Apple costs A$149 a year; Azure signing is not open to Australians | 2026-11-08 |
| 7 | TAP's format | Open | Leaning Max for Live now, plugin for other hosts later | Ableton cannot chain plugin MIDI effects | 2026-11-08 |
| 8 | Release unit | Decided | Albums: themed plugins plus music plus a sample pack plus story | Theming, SEO, bridges music and software | 2026-11-09 |
| 9 | Story and philosophy | Decided | Part of the process, and the marketing and discovery | itch lets the tools talk; an own site can show the journey | 2026-11-09 |
| 10 | Home platform | Decided | Own website, domain with VentraIP | Own the brand, pages, SEO and email list | 2026-11-09 |
| 11 | Checkout | Leaning | Rented: itch behind the site to start, or a merchant-of-record overlay | Avoid becoming responsible for tax in every country; switchable later | 2026-11-09 |
| 12 | Pricing tiers | Leaning | Singles, albums, discography, all cheap | Flat per-sale fees hurt at A$5, so bigger baskets matter; catalogue grows in value | 2026-11-09 |
| 13 | Discography contents | Open | "Everything so far" or "including future albums" | Both possible on itch; trade-off between repeat sales and a bigger first sale | 2026-11-09 |
| 14 | Bandcamp | Leaning | Second store for music and sample packs only | Plugins are not a normal Bandcamp product; strong for audio | 2026-11-09 |
| 15 | Free funnel | Leaning | Browser version per plugin, plus an occasional free taster plugin | Most indie makers use a free plugin; browser demos have no evidence yet, so measure them | 2026-11-09 |
| 16 | Copy protection | Leaning | One signed licence file per buyer listing everything they own; offline | Matches u-he, Sonic Charge, Vital; protects honest buyers' experience | 2026-11-09 |
| 17 | Cadence | Decided | Regular, not scheduled; release when the checklist passes | Fixed schedules strain hobby teams (Sokpop dropped theirs) | 2026-11-09 |
| 18 | Release pipeline | Decided | Album and plugin manifests; checks gate release; release run by hand | Quality stays steady without a fixed date | 2026-11-09 |
| 19 | Collaborator | Open | Agree split and ownership in writing first | Avoid disputes later | 2026-11-08 |

## Superseded and rejected

| Decision | Status | What it was | Replaced by | Why |
| --- | --- | --- | --- | --- |
| Main store | Superseded | itch.io as the store | Own site with rented checkout (#10, #11) | Own site can tell the story and own SEO |
| Release cadence | Superseded | Fixed schedule from a bank, released automatically by a timer | Regular, release when ready (#17) | Albums are finished when finished; a timer adds pressure without benefit |
| Release unit | Superseded | Single plugins, one at a time | Albums (#8) | Theming and story |
| Yearly pass | Superseded | "Everything released in 2027, paid once" | Discography (#12, #13), which can still be future-inclusive | Fits the album model; same mechanism on itch |
| Gumroad or Lemon Squeezy as fallback store | Superseded | Named fallbacks | Payhip or another merchant of record, checked first | Research found payout and platform-change risks |
| Shopify | Rejected | Store platform | | Monthly fee whether or not anything sells |
| Subscription or Patreon | Rejected | Monthly support model | | Disliked; needs a large following |
| Open source the plugins | Rejected | Airwindows-style model | | Conflicts with protecting the paid plugin |
| iLok / PACE | Rejected | Strong copy protection | | Far too expensive at this scale |
| Plugins on Bandcamp | Rejected | Selling plugins as Bandcamp products | Bandcamp for audio only (#14) | Not a normal Bandcamp product type |

## Corrections found along the way

These were said in conversation and later found wrong. The plan has been fixed.

1. itch.io does not split one project's sales between collaborators (it does for bundles).
2. Azure Trusted Signing (now Artifact Signing) is not open to Australian individuals or organisations.
3. Sokpop stopped its monthly schedule from December 2022.
4. JUCE's free tier is capped at US$20,000 a year, not more.
5. itch bundles are fixed at purchase, but files added to a project do reach earlier buyers, so a growing project works as a future-inclusive pass.
