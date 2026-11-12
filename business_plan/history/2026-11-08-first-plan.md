# Plugin business plan: paths, trade-offs and release pipeline

> **Superseded** by [`../current-plan.md`](../current-plan.md) on 2026-11-09. Kept for reference; the research docs tested this version.

Written 2026-11-08 from a planning conversation, then checked against web sources the same day. Purpose: one place for the decisions made so far, the options still open, and the release pipeline, so a collaborator can pick it up cold.

## How to read this

- **Status of each decision:** **Decided** (we are doing this), **Leaning** (current preference, not locked in), **Open** (not decided).
- **Evidence tags:** **[extract]** means the claim comes from a search-engine extract of the cited page, not a full read. **[inference]** means reasoning, not a source. Untagged statements are plans or opinions from the conversation.
- **Prices** are indicative and change often. Check the vendor's own pricing page before paying for anything. AUD figures converted from USD assume roughly A$1.50 to US$1.
- **Scale:** this is a hobby-sized business. Every choice below favours low fixed costs over features.

## Summary

The plan is to keep Max for Live for quick prototypes, and to ship a catalogue of small, cheap plugins built from one shared codebase. Each plugin gets a free browser version on a showcase website, which works as the funnel into the paid plugin. Sales go through a low-fee store, with bundles and a yearly pass as the main offer. Windows ships first because it needs no paid signing; Mac follows once demand is proven. Releases come from a bank of finished plugins on a fixed schedule, driven by a pipeline that refuses to release anything incomplete.

## Decision log

| # | Decision | Status | Choice so far |
| --- | --- | --- | --- |
| 1 | Prototyping tool | Decided | Max for Live stays for quick spikes |
| 2 | Shipping framework | Leaning | JUCE, with a DSP core that also builds for the web |
| 3 | Repo visibility | Decided | Make the repo private (to do, in GitHub settings) |
| 4 | Platforms | Decided | Windows first; Mac once proven; Linux if cheap |
| 5 | Plugin formats | Leaning | VST3 first; AU with Mac; CLAP later; no AAX |
| 6 | TAP's format | Open | Ableton cannot chain plugin MIDI effects (see below) |
| 7 | Store | Leaning | itch.io; Gumroad or Lemon Squeezy as fallbacks; not Shopify |
| 8 | Pricing model | Leaning | Cheap singles, bundles, and a yearly pass; no subscription |
| 9 | Copy protection | Leaning | Light: signed licence file, maybe one-off activation |
| 10 | Website | Decided | Showcase site with a free web version of each plugin, built by Jimmy |
| 11 | Release cadence | Leaning | Fixed schedule from a bank of finished plugins |
| 12 | Collaborator | Open | Possibly a friend; split and ownership to agree in writing |

## Corrections to the conversation

The research found four things said in the conversation that were wrong or out of date:

1. **itch.io does not split one project's sales between collaborators.** Its "open revenue sharing" is the platform's own cut, which you choose. Forum posts ask for per-project splits as a missing feature; revenue sharing does exist for bundles. **[extract]** https://itch.io/docs/creators/payments ; https://itch.io/t/2468387/best-practices-for-splitting-revenue-with-collaborators
2. **Azure Trusted Signing (now "Artifact Signing") is not open to Australians.** As of mid 2026, individuals are only supported in the USA and Canada, and an Australian user reported that Australia is missing from the organisation dropdown. Microsoft gave no timeline. **[extract]** https://learn.microsoft.com/en-us/answers/questions/5834570/when-will-azure-artifact-signing-support-australia ; https://learn.microsoft.com/en-us/answers/questions/2243504/trusted-signing-for-other-countries
3. **Sokpop stopped their monthly schedule.** They went from two games a month to one in March 2021, then dropped monthly deadlines from December 2022. Stacklands was their 90th game. Once they released on Steam, Steam sales earned more than Patreon and itch.io combined. **[extract]** https://en.wikipedia.org/wiki/Sokpop_Collective ; https://www.patreon.com/posts/important-switch-47586913 ; https://en.wikipedia.org/wiki/Stacklands
4. **The JUCE 8 free tier ("Starter") is capped at US$20,000 a year in revenue**, not the higher figure suggested. It no longer requires a splash screen. Indie (up to US$300,000) is the next tier. **[extract]** https://forum.juce.com/t/amendments-to-the-juce-end-user-licence-agreement-for-juce-8/61265 ; https://forum.juce.com/t/revenue-limits-for-juce-tiers/61058/23 . These come from forum posts, so check the EULA itself.

## 1. Framework and code layout

**Status: Leaning JUCE.** Max stays for spikes because it is quick to try ideas in, but `.amxd` files are readable by anyone, and only reach Live Suite users.

| Option | For | Against |
| --- | --- | --- |
| **JUCE (C++)** | Industry standard; VST3, AU, AAX, standalone from one codebase; most examples and help; free tier to US$20k a year **[extract]** | C++; licence cost once revenue passes US$20k a year |
| nih-plug (Rust) | Modern language; CLAP and VST3 | No AU, so no Logic **[inference]** |
| iPlug2 (C++) | Lighter; permissive licence | Smaller community |
| RNBO (Max) | Keep patching in Max; exports C++ and web | Paid add-on; patch-style code is harder to share as a library **[inference]** |
| Faust | Free; one `.dsp` source compiles to WebAssembly and to a JUCE project **[extract]** | A DSP language only, so UI still lives in JUCE |
| Cmajor | Exports a patch to WebAudio (HTML, JS, WASM) and to a C++ JUCE project **[extract]** | Newer, smaller community |

Sources: https://faustdoc.grame.fr/manual/deploying ; https://github.com/Fr0stbyteR/faustwasm ; https://cmajor.dev/docs/Examples

**Key idea: write each plugin's sound once, build it twice.** The free web version and the paid plugin must share their DSP, or every plugin costs double to maintain. Two ways to get there:

- **Path A, plain C++ core:** DSP in dependency-free C++. JUCE wraps it for the plugin; Emscripten compiles it to WebAssembly for an AudioWorklet in the browser. Most control, most setup.
- **Path B, Faust or Cmajor:** write the DSP in Faust or Cmajor, and export to both targets. Less glue code, but you take on a second language.

Proposed layout for the plugin repo (new repo or this one, once private):

```
shared/dsp/            reusable DSP building blocks
shared/ui/             shared JUCE look and feel, about box, licence screen
plugins/<slug>/
  dsp/                 this plugin's sound, used by both targets
  plugin/              JUCE wrapper (VST3, later AU, CLAP)
  web/                 browser version (AudioWorklet + UI)
  media/               screenshots, audio demos, video link
  release.yaml         the manifest the pipeline checks (see section 9)
site/                  showcase website
tools/release/         manifest checker, packager, release scripts
```

## 2. TAP's format problem

**Status: Open.** TAP is a MIDI effect that sends per-note pitch (MPE) to the instrument after it. **Ableton Live does not chain a plugin's MIDI output into the next device on the same track.** Putting a VST on a MIDI track makes Live treat it as an audio device; the workaround is routing its MIDI to a second track, and Live merges that MIDI to one channel, which breaks MPE. **[extract]** https://help.ableton.com/hc/en-us/articles/209070189-Accessing-the-MIDI-output-of-a-VST-plug-in ; https://forum.juce.com/t/vst3-midi-plugins-wont-load-in-ableton-live/36323?page=2 ; https://bluecataudio.com/HelpDesk/knowledgebase.php?article=49 . Most of these sources date from the Live 10 era; nothing found says Live 12 changed this.

| Path | Trade-off |
| --- | --- |
| Keep TAP as Max for Live for Ableton, and build a plugin version for hosts with good MIDI-effect support (Bitwig, Reaper, Logic via AU MIDI effects **[inference]**) | Two versions to maintain; best fit for each host |
| Make TAP an instrument with its own sound | Works everywhere; loses "use your own synth" |
| Start the plugin catalogue with audio effects (Slush Wave ideas), leave TAP for later | No problem to solve now; TAP's existing fans wait |

Also relevant: Live 12 supports VST3 and AU, but not CLAP. **[extract]** https://www.antarestech.com/blog/vst3-au-aax-clap-plugin-formats-2026

## 3. Platforms and code signing

**Status: Decided, Windows first.**

| Platform | Cost | Notes |
| --- | --- | --- |
| **Windows** | A$0 to start | Ship the `.vst3` in a zip with copy instructions rather than an installer `.exe`; SmartScreen mostly warns about downloaded executables **[inference]**. Azure signing is not available in Australia (see corrections). A traditional code signing certificate is the paid alternative, at roughly a few hundred US dollars a year **[inference, not priced]**. |
| **macOS** | A$149 a year (Apple Developer Program, US$99) **[extract]** https://ozbargain.com.au/node/519154 | Needed for notarisation; unsigned plugins are hard for users to open **[inference]**. Needs a Mac (or paid CI macOS minutes). One account can sign everything for both collaborators. |
| **Linux** | A$0 | Small audience (Bitwig, Reaper); nearly free if the code is cross-platform **[inference]** |
| **AAX (Pro Tools)** | Avid agreement plus PACE signing | Skip |

**Trade-off of Windows first:** a large share of music producers use Macs, so early sales will be lower **[inference]**. Mac users can still use the free web versions and join the email list, which shows whether Mac is worth A$149 a year.

## 4. Store

**Status: Leaning itch.io.**

| Store | Fees | Tax | Fit |
| --- | --- | --- | --- |
| **itch.io** | You choose itch's cut (default 10%, can be lower), then the payment processor fee, generally US$0.30 + 2.9% **[extract]** | In "itch.io Payouts" mode itch collects VAT and is responsible for remitting it **[extract]**; Australian GST handling not confirmed | Free hosting, pay-what-you-want, bundles, download keys, `butler` command-line uploads. No per-project collaborator split (see corrections). |
| Gumroad | About 10% + US$0.50 per sale **[extract]** | Merchant of record since 1 January 2025, collects and remits tax worldwide **[extract]** | Collaborator splits, licence keys; expensive on cheap items |
| Lemon Squeezy | 5% + US$0.50, no monthly fee **[extract]** | Merchant of record | Licence key API with activation limits **[extract]**; owned by Stripe since July 2024, terms may change **[extract]** |
| Payhip | Free plan 5%, plus processor fees **[extract]** | VAT handling unclear in sources | Simple; merchant-of-record status not confirmed |
| Shopify | Monthly fee whether or not anything sells | | Rejected: fixed cost too high for a hobby |

Sources: https://itch.io/docs/creators/payments ; https://www.wearefounders.uk/gumroad-fees-2026-what-sellers-actually-pay-per-sale/ ; https://flaviocopes.com/payment-providers/lemon-squeezy-vs-gumroad/ ; https://docs.lemonsqueezy.com/help/licensing/license-api ; https://sellfy.com/blog/payhip-pricing/

**Watch the flat fee.** A US$0.50 per-sale fee on a A$5 plugin takes about 15% before the percentage is added **[inference]**. itch's processor fee is smaller but still flat. This is a strong reason to make bundles the main offer.

**Steam** is worth a later look: Sokpop found Steam outsold their other channels combined **[extract]**, and Steam does list software. Its fees and onboarding were not researched.

## 5. Pricing model

**Status: Leaning cheap singles, bundles and a yearly pass.**

| Model | For | Against |
| --- | --- | --- |
| Singles only (about US$5 to $15) | Simple; low barrier | Flat fees hurt; no reason to come back |
| Bundles | Higher order value; fees matter less | Needs several plugins first |
| **Yearly pass** ("everything released in 2027, paid once") | Captures the monthly-release excitement without a subscription; each release is a reason to promote the pass | Needs a store that lets a buyer receive later additions (see open questions) |
| Subscription (Patreon, as Sokpop did) | Steady income; funds the schedule | Not wanted; ongoing obligation to subscribers |
| Pay-what-you-want with a minimum | Good for niche tools and goodwill | Most buyers pay the minimum **[inference]** |

At the end of each year, that year's plugins become a discounted bundle and a new pass starts.

## 6. Copy protection (DRM)

**Status: Leaning light protection.** Website code is not a worry; the paid plugin is.

| Level | How it works | Stops | Does not stop | Cost |
| --- | --- | --- | --- | --- |
| None | Buyer downloads the file | Nothing | Anything | A$0 |
| **Signed licence file** | At purchase, a small server function creates a licence with the buyer's name and email, signed with your private key. The plugin checks it offline with the public key built into it. Show "Licensed to ..." in the about box. | Fake keys; casual sharing (your name is on it) | Someone posting their own file; patched binaries | A$0 on a free serverless tier **[inference]** |
| **One-off online activation** | The plugin activates the key against a server once, with a machine limit, then works offline | Mass key sharing | Patched binaries | A$0 with Lemon Squeezy's licence API **[extract]** |
| iLok / PACE | Commercial anti-tamper | Most casual cracking | Determined crackers | Far too expensive here |

Lemon Squeezy's licence API: `activate` takes the key and an instance name, returns the activation limit and usage, and refuses new activations past the limit. It needs no API key, so a plugin can call it directly. Repeated activations from one machine can use up the limit, so store the instance ID. **[extract]** https://docs.lemonsqueezy.com/guides/tutorials/license-keys ; https://docs.lemonsqueezy.com/api/license-api/activate-license-key . JUCE's `juce_product_unlocking` module covers the plugin side **[inference]**.

**Recommendation:** a signed licence file, with one-off activation only if sharing becomes a real problem. Never require checks while the plugin runs. Any popular plugin gets cracked eventually, and at these prices the people who pay are honest buyers; don't make their experience worse. Budget about a weekend, no more.

## 7. Website and funnel

**Status: Decided, Jimmy builds the site.**

The funnel:

```
search / shared link
        |
        v
plugin page with free web version  -->  email sign-up (bundle discount, release alerts)
        |                                         |
        v                                         v
"Get the plugin" (single)         release emails  -->  yearly pass / bundle
```

- **Why the free version doesn't need crippling:** a browser version can't sit in a DAW, be automated, or render to a track. The paid plugin adds exactly that, so the web version can be good, which helps SEO and sharing.
- **Shareable links:** save a preset into the page URL so people share sounds, not just pages.
- **Per plugin page:** web demo, audio examples, short "what it does", screenshots, "Get the plugin" button, SEO title and description.
- **One bundle or pass page**, linked from every plugin page.
- **Hosting:** Cloudflare Pages is free and builds from private repos. GitHub Pages does not serve from a private repo on a free account **[inference, check]**. The domain stays with VentraIP, pointed at the host by DNS.
- **Email:** a free tier of MailerLite or Buttondown. The list is the most valuable part of the funnel.

## 8. Release cadence and collaboration

**Status: Leaning fixed schedule from a bank.**

- **The bank:** have three or four finished plugins ready before the first release. A buffer stops one busy month breaking the schedule.
- **Cadence:** monthly is demanding for two people with other jobs. Sokpop, a team of four doing this full time, dropped monthly deadlines after December 2022 **[extract]**. Every six weeks is a respectable alternative.
- **Working with a friend (Open).** Agree in writing before any money comes in:
  - the revenue split, and who receives payouts
  - who owns the code, each plugin and the brand name
  - what happens to the catalogue if one person stops
  - Use a shared brand name rather than a personal one.
- **Splitting money on itch.io:** since itch does not split single projects, either one person holds the account and pays the other by agreement, or each person sells their own plugins and the shared bundle splits revenue between them **[extract on bundles; mechanics unverified]**. Gumroad supports collaborator splits on products.

## 9. Release pipeline

The rule: **a plugin cannot release until every field in its manifest is filled in and every check passes.** Once it passes, it releases itself on its date.

### Plugin states

```
idea --> building --> banked --> scheduled --> released
                        ^             |
                        +-- check fails, back to banked
```

- **idea:** a name and a sentence.
- **building:** code in progress; manifest partly filled.
- **banked:** complete and passing all checks, no date yet.
- **scheduled:** has a release date; the pipeline will release it.
- **released:** live on the store and the site.

### The manifest (`plugins/<slug>/release.yaml`)

```yaml
slug: drift
name: Drift
status: scheduled            # idea | building | banked | scheduled | released
release_date: 2027-03-01
price_aud: 9
bundles: [pass-2027]
platforms: [windows]         # add macos, linux later
formats: [vst3]

store:
  itch_page: https://<brand>.itch.io/drift
  draft_created: true        # store pages are made by hand, see below

site:
  seo_title: "Drift: a free tape wobble effect in your browser"
  seo_description: "Play with Drift in your browser, then get the VST3."   # 155 characters or fewer
  web_demo: true

media:
  screenshot: media/drift.png
  audio_demos: [media/drift-before-after.mp3]
  video: https://...

credits:
  authors: [jimmy]
```

### Checks before a plugin can be banked

| Check | Fails if |
| --- | --- |
| Manifest complete | Any required field missing; SEO description too long; files it names don't exist |
| Builds | The plugin or web version doesn't build |
| DSP tests | Unit tests of the shared DSP fail |
| pluginval | Tracktion's free plugin validator finds a crash or bad behaviour (run at a fixed strictness) **[inference: tool not researched here]** |
| Web version | The page loads and makes sound in a headless browser |
| Store draft | `draft_created` is not true |
| Licence | The build accepts a test licence and rejects a tampered one |

### What runs when

| Trigger | Does |
| --- | --- |
| Every push | Manifest check, build, tests, pluginval on Windows (Linux for the web build) |
| Daily schedule (GitHub Actions cron) | Finds `scheduled` plugins whose date is today or earlier; builds release zips; uploads them to itch with `butler`; sets `status: released` and commits; the commit triggers a site rebuild on Cloudflare Pages, which publishes the plugin page |
| Release commit | Optional: tells the email service to send a prepared campaign |

### What stays manual

- **Creating the store page.** None of itch, Gumroad or Lemon Squeezy were found to create products through an API **[inference, not confirmed for itch]**. Make the page as a draft when the plugin is banked.
- **Making the store page public on release day**, unless itch allows a page to be public before files exist (open question).
- **Writing the release email.**

### Keeping CI cheap

GitHub's free plan includes about 2,000 Actions minutes a month for private repos; Windows uses minutes twice as fast as Linux, and macOS ten times as fast **[extract]** https://docs.github.com/en/billing/concepts/product-billing/github-actions . A self-hosted runner on your own PC appears to be free: a planned fee for self-hosted runners on private repos was announced for March 2026 and then postponed **[extract, conflicting sources]** https://itbrief.com.au/story/github-cuts-actions-runner-prices-adds-new-usage-fee . So: Linux for everything that can run on Linux, Windows builds on your own PC, Mac builds only on release tags.

## 10. Costs

### Phase 1: Windows only

| Item | Choice | A$ a year |
| --- | --- | --- |
| Domain | Already with VentraIP | Already paying |
| Website hosting | Cloudflare Pages | 0 |
| Download files | Store hosts them | 0 |
| Licence function | Free serverless tier | 0 |
| Store | itch.io (fees per sale only) | 0 fixed |
| Email list | Free tier | 0 |
| Analytics | Cloudflare Web Analytics | 0 |
| JUCE | Starter, under US$20k revenue | 0 |
| Windows signing | Ship unsigned zips | 0 |
| CI | Free minutes plus self-hosted runner | 0 |
| **Total fixed** | | **about A$0 beyond the domain** |

### Phase 2: add Mac

| Item | A$ a year |
| --- | --- |
| Apple Developer Program | 149 **[extract]** |
| A Mac, if neither collaborator has one | One-off hardware cost |
| **Total fixed** | **about A$150** |

### Possible later costs

- Windows code signing certificate, if SmartScreen warnings hurt sales.
- JUCE Indie licence, once revenue passes US$20k a year.
- Paid email tier, once the list outgrows the free tier.

**Break-even for Phase 2:** at about A$8 after fees per sale, roughly 20 sales a year covers the Apple fee **[inference]**. The real cost is time, which the pipeline is there to reduce.

### Ways to cut costs further

- Stay Windows-only until the email list shows Mac demand.
- Build on your own machines with self-hosted runners.
- Use no DRM, or only signed licence files.
- One Apple account signs for both collaborators.
- Skip AAX.
- Make your own audio demos and screenshots.

## 11. Australian tax and admin

- GST registration is only required once turnover reaches A$75,000 a year **[inference: well-known ATO threshold, not re-checked here]**.
- Declare the income either way; an ABN makes this tidier.
- A merchant-of-record store collects overseas VAT and GST for you.
- If working with a friend, ask an accountant whether the arrangement counts as a partnership for tax.

## Open questions and research still to do

1. **Yearly pass mechanics:** which store lets a buyer automatically receive plugins added after purchase? (Gumroad and Lemon Squeezy let you update a product's files **[inference]**; itch bundles not checked.)
2. **itch.io and Australian GST** in Payouts mode.
3. **itch.io page timing:** can a page be public as "coming soon", so release day needs no manual step?
4. **Windows code signing alternatives** available to Australians, with current prices.
5. **Live 12 and plugin MIDI effects:** confirm the old limitation still holds before choosing TAP's path.
6. **Steam** as a later channel: fees, onboarding, how plugins sell there.
7. **Framework:** try one small plugin in Path A (plain C++ plus JUCE plus Emscripten) and one in Path B (Faust) before choosing.
8. **Collaborator agreement:** decide the split and ownership.

## Next steps

1. Make this repo private in GitHub settings.
2. Pick the framework path by building one tiny effect both ways.
3. Decide TAP's path.
4. Set up the plugin repo layout, manifest checker and push checks.
5. Fill the bank with three or four plugins.
6. Launch the site and email list with the first web versions.
7. Turn on the scheduled release.

## Sources

- JUCE 8 licence: https://forum.juce.com/t/amendments-to-the-juce-end-user-licence-agreement-for-juce-8/61265 ; https://forum.juce.com/t/revenue-limits-for-juce-tiers/61058/23
- itch.io payments and revenue sharing: https://itch.io/docs/creators/payments ; https://itch.io/t/2468387/best-practices-for-splitting-revenue-with-collaborators
- Azure (Artifact) Signing availability: https://learn.microsoft.com/en-us/answers/questions/5834570/when-will-azure-artifact-signing-support-australia ; https://learn.microsoft.com/en-us/answers/questions/2243504/trusted-signing-for-other-countries ; https://www.devclass.com/security/2026/01/14/code-signing-windows-apps-may-be-easier-and-more-secure-with-new-azure-artifact-service/4079554
- Ableton and plugin MIDI: https://help.ableton.com/hc/en-us/articles/209070189-Accessing-the-MIDI-output-of-a-VST-plug-in ; https://forum.juce.com/t/vst3-midi-plugins-wont-load-in-ableton-live/36323?page=2 ; https://bluecataudio.com/HelpDesk/knowledgebase.php?article=49 ; https://www.antarestech.com/blog/vst3-au-aax-clap-plugin-formats-2026
- Store fees: https://www.wearefounders.uk/gumroad-fees-2026-what-sellers-actually-pay-per-sale/ ; https://flaviocopes.com/payment-providers/lemon-squeezy-vs-gumroad/ ; https://sellfy.com/blog/payhip-pricing/
- Lemon Squeezy licence API: https://docs.lemonsqueezy.com/guides/tutorials/license-keys ; https://docs.lemonsqueezy.com/api/license-api/activate-license-key ; https://docs.lemonsqueezy.com/help/licensing/license-api
- Faust and Cmajor web export: https://faustdoc.grame.fr/manual/deploying ; https://github.com/Fr0stbyteR/faustwasm ; https://cmajor.dev/docs/Examples
- Apple Developer Program (AUD): https://ozbargain.com.au/node/519154
- GitHub Actions billing: https://docs.github.com/en/billing/concepts/product-billing/github-actions ; https://itbrief.com.au/story/github-cuts-actions-runner-prices-adds-new-usage-fee
- Sokpop and Stacklands: https://en.wikipedia.org/wiki/Sokpop_Collective ; https://en.wikipedia.org/wiki/Stacklands ; https://www.patreon.com/posts/important-switch-47586913 ; https://newsletter.gamediscover.co/p/how-one-of-sokpops-almost-100-steam
