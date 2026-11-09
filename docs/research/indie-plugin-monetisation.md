# Research: how small independent plugin and Max for Live developers make money

Researched 2026-11-09. Purpose: to test the money side of [`docs/business/plugin-business-plan.md`](../business/plugin-business-plan.md) against what real developers do. The work comes in three stages, as asked: **Observe** (what developers actually do), **Apply** (how each pattern fits Jimmy), and **Invest** (what each option costs, earns and trades away). Section 5 lists where the findings change the current plan.

## How to read this

- **Method caveat (important):** the sandbox could not fetch any page. The first four fetches (`valhalladsp.com`, `patreon.com`, `vital.audio`, `kilohearts.com`) all failed DNS because those hosts are outside the session's network allowlist, so no further fetches were tried. **Every claim below rests on search-engine extracts, not full reads.** No claim is tagged **[read]**. To read pages first-hand, those hosts must be allowed in the session's network settings.
- **Evidence tags:** **[extract]** means the claim comes from a search-engine extract of the cited page. **[inference]** means the researcher's reasoning, not a source. **(secondary)** after a citation means the page belongs to a journalist, retailer, forum user or aggregator, not to the developer or platform. Secondary sources are used for context only, and are labelled each time.
- **Source quality:** about two thirds of the cases have a first-party page behind them (developer FAQ, product page, store docs, the developer's own forum post, or an interview where the developer speaks). Tokyo Dawn Labs, Aberrant DSP, Vital's tiers, Audio Damage's prices, Plugin Alliance's subscription price and most Gumroad fee details rest partly or wholly on secondary pages.
- **Dates and prices:** every price and fee was seen in search extracts on 2026-11-09 unless a different date is given. Many extracts carry no date of their own, so a price may be older than it looks. Check the vendor's page before relying on any number. AUD figures convert USD at about A$1.50 to US$1, as the business plan does.
- **Revenue:** no developer in this survey publishes revenue figures. Where a number appears, it is one the developer or a survey published, cited as such. All other revenue statements are **[inference]** and are labelled as guesses.
- **Confidence per case:** High = a first-party source plus an independent source agree. Medium = one first-party source, or several secondary sources that agree. Low = a single secondary source, or inference.

## TL;DR

| # | Case | Model in one line | Pattern class | Fit for Jimmy | Confidence |
|---|------|-------------------|---------------|---------------|------------|
| 1 | Valhalla DSP | US$50 flat, no sales, no bundles, own store only; big free plugin (Supermassive) | Interesting | Partly | High |
| 2 | Airwindows | Everything free and MIT open source; funded by Patreon | Unique | Does not fit | Medium |
| 3 | u-he | Serial plus keyfile, no online activation; demos never expire but add noise | Common (protection) | Fits | High |
| 4 | Tokyo Dawn Labs | Free plugin, paid "Gentleman's Edition" of the same plugin | Interesting | Fits | Medium |
| 5 | Kilohearts | Free Essentials bundle feeds paid hosts; US$9.99 a month "subscribe to own" | Interesting | Partly | Medium |
| 6 | Baby Audio | Free plugins split out of paid ones; newsletter; perpetual licences | Common | Fits | Medium |
| 7 | Aberrant DSP | Cheap paid plugins (US$20 to US$30); free plugin built from the paid ones | Interesting | Fits well | Medium |
| 8 | Sonic Charge | Personal licence on any number of computers; rent-to-own bundle (24 payments) | Interesting | Partly | Medium |
| 9 | Xfer Records (Serum) | Lifetime free updates, including Serum 2; also on Splice rent-to-own | Common | Fits | Medium |
| 10 | Vital | GPL source, free Basic tier, paid tiers differ only by content; no copy protection | Unique | Partly | Medium |
| 11 | Surge XT | Free, GPL, volunteer team, no sales | Unique | Does not fit | Medium |
| 12 | Audio Damage | Discontinued products made free; cheap upgrades | Interesting | Fits later | Low |
| 13 | Puremagnetik | Monthly new plugin; subscription gets it early and cheap; sold singly later; big catalogue bundle | Interesting | Fits well | Medium |
| 14 | Plugin Alliance and Unfiltered Audio | Small maker hands distribution to a big platform; subscription plus deep discounts | Common (for platforms) | Does not fit now | Medium |
| 15 | Splice rent-to-own | Monthly payments that end in a perpetual licence | Interesting | Does not fit | Medium |
| 16 | Native Instruments | Perpetual bundles plus a free starter tier | Common (large firms) | Does not fit | Low |
| 17 | Isotonik Studios | Curated Max for Live publisher for 60+ developers; free and pay-what-you-want devices | Interesting | Partly (TAP) | Medium |
| 18 | Dillon Bastan | Paid devices via Isotonik, free devices on Gumroad, a commissioned pack with Ableton | Interesting | Partly (TAP) | Medium |
| 19 | Surreal Machines | Max for Live pack sold in Ableton's store, later ported to VST/AU on its own store | Unique | Later | Medium |
| 20 | A small Gumroad Max for Live seller | Gumroad payouts stopped; moved shop to Sellfy | Warning | Relevant | Low |
| 21 | Ableton Learning Synths | Free browser synth course; brand funnel for a big company | Unique | Partly | Medium |
| 22 | Leimma and Apotome | Free browser tools launched at a festival; cultural project | Unique | Partly | Medium |
| 23 | Sokpop | US$3 a month Patreon for two games a month; singles on itch; Steam back catalogue later doubled revenue | Interesting | Partly | Medium |
| 24 | itch.io (audio tools and bundles) | Plugins sold at US$5 to US$30; co-op bundles with revenue splits | Common (games) | Fits | Medium |
| 25 | Steam Direct | US$100 per product; audio production software accepted | Interesting | Later | Medium |
| 26 | Humble software bundles | Buyer chooses split; used by established audio firms | Unique | Does not fit | Low |

**Main findings [inference, from the cases cited below]:**

1. **A free *plugin*, not only a demo, is the most common funnel.** Valhalla, Tokyo Dawn, Kilohearts, Baby Audio, Aberrant DSP and Dillon Bastan all give away real plugins. No case was found where a free *browser* version was shown to sell a paid plugin. Jimmy's browser funnel is untested, not wrong, so it needs a backup and a way to measure it.
2. **Light copy protection is normal and respected.** u-he (serial plus keyfile, no activation), Sonic Charge (personal licence, any number of computers) and Vital (none) all sell well. The plan's signed licence file matches this.
3. **The closest real model to Jimmy's "yearly pass" is Puremagnetik**, which releases a plugin a month, gives subscribers each one early and cheaply, then sells it singly later and rolls everything into a catalogue bundle. It is a subscription, but the same shape can be sold as a one-off annual pass.
4. **itch.io bundles probably cannot deliver a pass.** Forum reports say a bundle's contents are frozen at purchase. A pass is better built as one itch project whose files grow over the year **[inference, test before relying on it]**.
5. **At A$5 prices, flat fees decide the store.** itch's processor fee is about US$0.30 plus 2.9%; Gumroad, Lemon Squeezy, Paddle and Moonbase all add US$0.50 or €1 a sale, and Paddle asks for custom terms under US$10. itch stays the right first store.
6. **Expect hobby-sized income.** In Moonbase's 2025 survey of 95 independent audio software firms, 49% were side projects under US$20,000 a year. One JUCE forum developer sold 14 copies of a US$10 plugin in two weeks, then 5 in five months. Plan for tens of sales a year per plugin, not hundreds.
7. **Steam charges US$100 per product**, so a catalogue of many small plugins should go there as one collection, not one product each.

## Stage 1: Observe

Each case lists what the developer does, with tags and citations. Numbers in square brackets refer to the Sources list at the end.

### 1.1 Small indie plugin makers

#### Valhalla DSP (Sean Costello). Confidence: High

- Plugins cost US$50 each "all year long" **[extract]** [1]. Costello says US$50 "is already a pretty low price", that he is committed to making excellent DSP affordable, and that if the tools are more affordable more people can use them **[extract]** [1].
- No bundles: the FAQ answer to "can I get a bundle deal?" is that "$50 per plugin IS the discount" **[extract]** [2]. No Black Friday sales; the FAQ jokes "Every Day Is Black Friday At Valhalla DSP!" **[extract]** [3].
- Sells only through the Valhalla DSP website, not resellers **[extract]** [4].
- Gives away full plugins: Supermassive is free (VST, AU, AAX) and had a version 5.0.0 update dated 25 November 2025 **[extract]** [5][6 (secondary)]. Costello wrote a post called "Why is Valhalla Supermassive Free?", but the extract showed only its opening, so his reasons are not known here **[extract]** [5].
- Founded 2009 **[extract]** [6 (secondary)]. Revenue: not published.
- **Why it works [inference]:** a reputation for quality plus one simple price removes the "wait for the sale" habit. The free plugin keeps the name in front of producers between paid releases.

#### Airwindows (Chris Johnson). Confidence: Medium

- All plugins are free and open source under the MIT licence, "brought to you by my Patreon" **[extract]** [7][8].
- Johnson writes that the Patreon is why he is "still here putting out new plugins", and that it lets him "not have to play by the same rules as commercial software makers" **[extract]** [8].
- Older posts tied open-sourcing to a Patreon goal (US$800 a month) **[extract]** [8]. Current patron numbers and income: not found.
- Airwindows Consolidated (May 2024) put 400+ effects in one free plugin (CLAP, VST3, AU, LV2); a March 2026 update reports 500+ **[extract]** [9 (secondary)][10 (secondary)].
- The MIT licence lets others build on the code with credit; a 2026 pay-what-you-want collection built on Airwindows DSP points buyers back to his Patreon **[extract]** [8].
- **Why it works for him [inference]:** almost two decades of near-constant releases built a following large enough to fund him directly. The output is the marketing.

#### u-he. Confidence: High

- Copy protection is serial number based: entering name and serial writes a licence key file, which the plugin reads. u-he says it does not use internet challenge/response, background services, activation counters, dongles or machine IDs **[extract]** [11].
- Demos have no time or feature limit, but unregistered copies play a vinyl-like static for 3 to 5 seconds about every 30 seconds after a few minutes **[extract]** [12].
- A forum post attributed to u-he says making purchase harder would not likely increase sales, and lists a fair price, a good plug-in, responsive support and user-friendly copy protection as what drives a purchase **[extract]** [13 (forum; authorship not confirmed)].

#### Tokyo Dawn Labs. Confidence: Medium

- TDR Nova is a free dynamic EQ; Nova GE (Gentleman's Edition) adds two bands, expansion, steeper filters, equal-loudness tools and an "Insane" quality mode **[extract]** [14][15].
- GE price: early coverage gave €40, a later update €50, a retailer US$60. Current price unconfirmed **[extract]** [16 (secondary)].
- **Pattern [inference]:** the free and paid products are the *same plugin*. The free one earns trust and search traffic; the paid one sells to people who already use it.

#### Kilohearts. Confidence: Medium

- Kilohearts Essentials is a free bundle of about 30 effects that also load as "snapins" inside the paid hosts Phase Plant, Multipass and Snap Heap. Every new Kilohearts account gets Essentials **[extract]** [17].
- Subscription: US$9.99 a month for every plugin and content bank, including new releases **[extract]** [18]. After every 12 payments, subscribers get a US$100 coupon towards a perpetual licence **[extract]** [18][19].
- Kilohearts Ultimate (all hosts and effects) about US$399 at retailers, seen in 2026 price tracking **[extract]** [20 (secondary listings)].
- **Pattern [inference]:** the free effects are not a teaser; they make the paid hosts more valuable. Every free user is a step from a paid host.

#### Baby Audio. Confidence: Medium

- Magic Switch began as a feature of the paid Super VHS plugin and was released as its own free plugin; Magic Dice followed as its successor **[extract]** [23 (retailer)].
- Founder Caspar Bock Soerensen, in 2026: "we take the term perpetual very seriously and absolutely expect to support future compatibility demands with free updates" **[extract]** [22].
- Bock also says the company donates plugins to free music-education groups **[extract]** [21].
- Newsletter: reviewers say Baby Audio announces free plugins and deals by email **[extract]** [23 (secondary)]. No first-party statement was found that the freebies are a deliberate lead magnet.

#### Aberrant DSP. Confidence: Medium (press and retailer sources)

- SketchCassette launched in 2019 at US$20; SketchCassette II at US$30 with a US$20 introduction; ShapeShifter launched at US$20 **[extract]** [25 (secondary)][26 (retailer)].
- Lofi Oddity is a free plugin that combines effects from three paid plugins (SketchCassette, Digitalis, ShapeShifter) **[extract]** [24 (secondary)].
- Complete bundle about US$120 at a retailer **[extract]** [26 (retailer)].
- **Pattern [inference]:** low prices plus a free "sampler" made of the paid plugins' parts. Every free user hears the paid sounds.

#### Sonic Charge. Confidence: Medium

- Synplant 2 US$149 (US$50 upgrade); Microtonic US$99; the Complete Bundle can be paid in monthly instalments and owned after 24 payments **[extract]** [27].
- Fredrik Lidström on the forum: a "personal licence" is tied to you, not your computer, so you can install it on any number of computers; re-run the Sonic Charge Authenticator if hardware changes **[extract]** [28].

#### Xfer Records (Serum). Confidence: Medium

- Serum 2 (March 2025) was a free update for all existing Serum owners; new buyers paid US$189 at launch, then US$249 **[extract]** [29 (secondary)][30 (secondary)].
- Serum was the first plugin on Splice rent-to-own; Splice says the rent-to-own total is roughly Xfer's undiscounted price, with no interest **[extract]** [31][32].

#### Vital (Matt Tytel). Confidence: Medium

- Four editions: Basic free, Plus US$25, Pro US$80, or US$5 a month. The synth engine is the same in all; tiers differ by presets, wavetables and extras such as text-to-wavetable **[extract]** [34 (secondary)].
- Source code is GPLv3; the sold binaries are not open source **[extract]** [35 (secondary)].
- KVR lists copy protection as "None" **[extract]** [33 (secondary)]. No first-party statement on piracy was found.
- **Why it works for him [inference]:** the free version is complete, so there is little to pirate; people pay for content and to support him.

#### Surge XT. Confidence: Medium

- Surge began as a commercial synth by Vember Audio; its creator released it under GPL3 in 2018; it is now maintained by volunteers **[extract]** [36]. No donation or sales scheme was found.

#### Audio Damage. Confidence: Low

- Founded 2002; hardware closed down to refocus on software **[extract]** [37][38].
- Keeps a library of free plugins and released its discontinued back catalogue at no cost **[extract]** [38]. Upgrades for owners reportedly US$5 to US$10 **[extract]** [89 (secondary)].

#### Puremagnetik. Confidence: Medium

- A subscription gives immediate access to the current month's new device at a discount of over 50% **[extract]** [39].
- A 2023 report: monthly plan US$9, yearly US$89, with plugins later sold separately at US$20 each **[extract]** [40 (secondary)].
- "The Cube" bundles 39 plugins, valued at US$800 at full price **[extract]** [41].
- Also gives away plugins (for example Expanse, Driftmaker, Innervelt) **[extract]** [40 (secondary)].
- **Pattern [inference]:** this is the closest real match to Jimmy's plan: a steady release cadence, early buyers rewarded, every release later sold singly, the whole catalogue as a big bundle.

### 1.2 Platforms, subscriptions and rent-to-own

#### Plugin Alliance and Unfiltered Audio. Confidence: Medium

- In 2015 Unfiltered Audio handed worldwide distribution, marketing and support to Plugin Alliance **[extract]** [42].
- Unfiltered Audio plugins are in Plugin Alliance's MEGA subscription (180+ plugins from 40+ brands) **[extract]** [43]. Launch price of the full yearly plan was US$249.99 a year, US$24.99 a month **[extract]** [44 (secondary)]; current price not confirmed.
- Perpetual prices are deeply discounted: Dent 2 at US$29.99 down from US$149 (date unclear) **[extract]** [43].
- July 2026: founder Dirk Ulrich bought Plugin Alliance and Brainworx back from inMusic **[extract]** [45 (secondary)].
- **Lesson [inference]:** joining a platform trades control of price and brand for reach.

#### Splice rent-to-own. Confidence: Medium

- Each payment counts towards the retail price; the final payment gives a lifetime licence; you can pause, cancel and keep progress **[extract]** [31]. Developer terms are not public.

#### Native Instruments. Confidence: Low

- Komplete 26 (May 2026) is a perpetual product (Select US$99 up to Collector's Edition US$1,949); Komplete Start stays free **[extract]** [46 (secondary)].

#### Moonbase's survey and pricing article (context, vendor source). Confidence: Medium

- 95 independent audio software companies answered; 53% were solo; 49% were side projects under US$20,000 a year, 21% earned US$20,000 to US$100,000, 30% more than US$100,000 **[extract]** [47]. Moonbase sells licensing and checkout to these firms and the sample is self-selected.
- Moonbase's own pricing article says subscription-only approaches have become "generally speaking, unpopular with music buyers", citing Minimal Audio and Waves, and that buyers have been trained to expect discounts **[extract]** [48].

### 1.3 Max for Live sellers

#### Isotonik Studios (publisher). Confidence: Medium

- Describes the Isotonik Collective as a curated group of developers, musicians and sound designers under its umbrella; one source puts it at 60+ **[extract]** [49].
- Sells paid packs (for example £25.99), free devices and pay-what-you-want devices (Push 3 Control Master LE) **[extract]** [50][49].
- Customer support requests are passed to the relevant developer **[extract]** [49].
- Developer revenue split and submission terms: **not found**.

#### Dillon Bastan. Confidence: Medium

- Sells through Isotonik: data.train £15, Iota £18 (from £22), Ecosystem £27.50; Strange Mod and BEAST Repeat are free; a multi-buy discount applies to three or more devices **[extract]** [51][52][53].
- Ableton's blog says he released Iota through Ableton and offers other work as free downloads (or for a charity donation) on Gumroad **[extract]** [54].
- Inspired by Nature was made with Ableton and ships with Live 12 Suite **[extract]** [55]. Commission terms are not public.
- **Pattern [inference]:** three channels at once: a publisher for paid devices, free devices for reach, and a first-party deal with Ableton for reputation.

#### Surreal Machines. Confidence: Medium

- Dub Machines, a Max for Live pack, sells in Ableton's own store at US$39; it needs Live 11.1 Standard with Max for Live **[extract]** [56].
- In 2017 they released VST/AU versions sold on their own site **[extract]** [57 (secondary)].
- **Pattern [inference]:** start inside Ableton, then port to plugins once the sound has a following. This is the same path Jimmy plans for TAP and his Max prototypes.

#### A small Gumroad Max for Live seller (Akihiko Matsumoto). Confidence: Low

- His Gumroad pages say that "due to recent changes in Gumroad's partnerships with PayPal and Stripe, we have been unable to receive payouts for several months", and several pages point to a new Sellfy shop **[extract]** [58].
- One seller's experience only, and the country is not stated in the extract. It is still a warning: a store that cannot pay you out is worse than a fee.

#### Max for Live technical facts. Confidence: Medium

- maxforlive.com lets you host a device there or link to an external store **[extract]** [59]. No seller fee terms were found.
- Freezing a device bundles its files for distribution, but nothing found says it stops someone opening and editing it **[extract]** [60]. **[inference]** An `.amxd` should be treated as readable by any buyer.

### 1.4 Web-first and browser tools

#### Ableton Learning Synths. Confidence: Medium

- A free browser synthesiser course, works in any browser with Web Audio; followed Learning Music (2017) **[extract]** [61]. Cycling '74's RNBO site features it **[extract]** [62].
- **Why it works for Ableton [inference]:** education builds the brand of a company that sells a DAW. It is not a per-product funnel.

#### Leimma and Apotome (Khyam Allami with Counterpoint). Confidence: Medium

- Free browser tools launched at CTM Festival 2021 **[extract]** [63 (secondary)]. Funding not found.

#### Browser demos of commercial audio products. Confidence: Low

- Bungee (a time-stretch library) runs a live browser demo built on AudioWorklet and WebAssembly **[extract]** [64]. No case was found where a plugin maker published how many web-demo users bought the plugin.

### 1.5 Game-dev analogues and stores

#### Sokpop Collective. Confidence: Medium

- From 2018, a Patreon subscription gave a new game about every two weeks for a few dollars a month (GDC 2019 talk by Tijmen Tio and Aran Koning) **[extract]** [65]. Games also sold singly at about US$3 on itch.io; close to 1,000 patrons in a 2019 report, about 1,300 in a later one **[extract]** [66 (secondary)].
- In a January 2021 poll, patrons overwhelmingly preferred one game a month to two **[extract]** [67].
- After setting up about 70 Steam pages for the back catalogue, Steam "essentially doubled our revenue"; Koning said earnings had been about 50/50 between Steam and Patreon before Stacklands **[extract]** [68]. Tio is also quoted saying Steam makes more than itch.io and Patreon together **[extract]** [69 (secondary)].
- Went from two games a month to one in March 2021, and later dropped the game-a-month target after their 100th release **[extract]** [90][91 (secondary)].

#### itch.io (store mechanics). Confidence: Medium

- You set itch's share (default 10%, can be 0%); then the processor fee, generally US$0.30 plus 2.9%. itch recommends selling at US$2 or more to avoid losing a large share to the processor **[extract]** [70].
- In "itch.io Payouts" mode itch collects and remits VAT; in "Direct to you" mode the seller is responsible **[extract]** [70].
- Co-op bundles: one host picks a whole-number revenue split per developer; bundle payments cannot be split for "Direct to you" accounts **[extract]** [71][72].
- **Bundles and later additions:** several creators report that bundle contents are frozen at purchase and new items do not reach earlier buyers; creators hand out keys by hand. Some charity bundles behaved differently **[extract, forum posts]** [73].
- A future release date blocks purchase and download until the date, and shows a countdown; draft pages are visible only by secret link **[extract]** [74].
- `butler push <dir> user/game:channel` uploads builds; no official API to create projects was found **[extract]** [75].
- Audio plugins are sold in itch's Tools section: seen prices include US$4.99, US$19.90 and US$29.90 **[extract]** [76].

#### Steam Direct. Confidence: Medium

- US$100 fee for each product; mainly games, but Audio/Video Production software may be accepted **[extract]** [77]. The fee is recoupable **[extract]** [78]. Revenue share 30% up to US$10 million **[extract]** [79 (secondary)].

#### Humble software bundles. Confidence: Low

- Buyers choose the split between developer, charity and Humble; music software bundles come from established firms (for example Acoustica) **[extract]** [80]. No path for a hobby developer to join was found.

### 1.6 Payments and fees (cross-cutting). Confidence: Medium

| Store or service | Fee seen | Merchant of record | Notes |
|---|---|---|---|
| itch.io | Your chosen share (default 10%) + about US$0.30 + 2.9% | Yes for VAT in Payouts mode **[extract]** [70] | Australian GST handling not confirmed |
| Gumroad | 10% + US$0.50 plus card processing on direct sales; 30% on Discover sales **[extract]** [81 (secondary)] | Yes since 1 January 2025 **[extract]** [81 (secondary)] | Minimum payout reportedly raised to US$100 in March 2026 **[extract]** [81 (secondary)]; payout problems reported by one seller [58] |
| Lemon Squeezy | 5% + US$0.50 (from the business plan) | Yes | Lemon Squeezy's 2026 update points users towards Stripe Managed Payments **[extract]** [82] |
| Paddle | 5% + US$0.50; products under US$10 need custom pricing **[extract]** [83] | Yes | Poor fit for A$5 plugins |
| Moonbase | 5% + €1 per sale; licensing free for the first 10,000 validations a month, then €5 per 10,000 **[extract]** [84] | Yes | Built for audio plugins; flat €1 hurts cheap items |

### 1.7 Classification

**COMMON (most do it):**

- A free plugin alongside paid ones (Valhalla, Tokyo Dawn, Kilohearts, Baby Audio, Aberrant, Bastan, Puremagnetik, Audio Damage).
- Perpetual licences with free updates (Valhalla, Baby Audio, Xfer, u-he).
- Light protection: serial or keyfile, or none (u-he, Sonic Charge, Vital; a KVR thread lists many more [85 (forum)]).
- Bundles of the catalogue (Kilohearts, Aberrant, Puremagnetik).
- Retail sales and discounts through resellers such as Plugin Boutique **[extract]** [86 (secondary)].

**INTERESTING (clever and transferable):**

- Same plugin free and paid, with the paid one adding depth (Tokyo Dawn).
- A free plugin built from the paid plugins' parts (Aberrant Lofi Oddity) or split off a paid one (Baby Audio Magic Switch).
- One fixed price, no sales, no bundles (Valhalla).
- Early access by subscription, then singles and a catalogue bundle later (Puremagnetik).
- Subscribe-to-own coupons (Kilohearts) and rent-to-own (Sonic Charge, Splice).
- Unlimited demo with periodic noise instead of a time limit (u-he).
- Back catalogue moved to a bigger store later (Sokpop on Steam).
- Making discontinued products free (Audio Damage).
- Max for Live first, plugin port later (Surreal Machines).

**UNIQUE (one or two do it, and why it works for them):**

- **Airwindows:** everything free, funded by patrons. Works because of a huge, long-running output and a loyal following.
- **Vital and Surge:** GPL source. Works for Vital because the free tier is complete and the brand came from an earlier open-source synth (Helm); Surge has a volunteer team and no sales.
- **Ableton Learning Synths:** a large company funding free education for brand reasons.
- **Leimma and Apotome:** a cultural project launched at a festival.
- **Plugin Alliance MEGA, Native Instruments:** scale that a hobby maker cannot match.
- **Humble bundles:** open to established brands.

## Stage 2: Apply to Jimmy

Jimmy's situation, from the brief and the plan: hobby-sized, cost-sensitive, Australian; Max for Live prototypes; many small cheap JUCE plugins, Windows first; a free browser version of each plugin on a showcase site as the funnel; a bank of finished plugins released on a schedule by a pipeline; likes bundles and a yearly pass, dislikes subscriptions; maybe a friend; worried about copying of the paid plugin.

| Pattern (source cases) | Fit | Why **[inference]** |
|---|---|---|
| Free plugin alongside paid (Valhalla, TDR, Baby Audio, Aberrant) | **Fits** | Cheap to add: a cut-down build of a plugin Jimmy already has. It gives a funnel that works inside a DAW, where the browser version cannot. |
| Free browser version as funnel (Learning Synths, Bungee) | **Partly fits** | Only big firms or libraries do it, and no conversion data exists. Its strengths are sharing and SEO. Keep it, but measure clicks from web demo to store, and do not make it the only free thing. |
| Same plugin free and paid (TDR) | **Fits** | Suits plugins with an obvious "more" (extra bands, modes, presets). Less suited to tiny one-idea plugins. |
| Free sampler built from paid plugins (Aberrant) | **Fits well** | A yearly free "taster" plugin with one mode from each of that year's plugins would advertise the pass. |
| Fixed price, no sales (Valhalla) | **Partly fits** | Simple and honest, but Valhalla has a reputation that makes it work. A new maker may need sale events for attention. A middle path: no discounts on singles, one sale a year on the previous year's bundle. |
| Puremagnetik release-and-catalogue model | **Fits well** | Same shape as the plan: steady releases, early buyers rewarded, singles later, catalogue bundle. Swap the subscription for a one-off yearly pass. |
| Subscribe-to-own and rent-to-own (Kilohearts, Sonic Charge, Splice) | **Does not fit** | Needs a licensing back end and pricier products. At A$5 to A$15 there is nothing to spread out. |
| Patreon or subscription (Airwindows, Sokpop) | **Does not fit** | Jimmy dislikes subscriptions, and both cases needed about 1,000 or more patrons or a long reputation. A yearly pass gives the same "reason to come back" without monthly obligations. |
| Open source (Airwindows, Vital, Surge) | **Does not fit** | Jimmy's worry is copying of the paid plugin. Open-sourcing the paid DSP invites exactly that. Open-sourcing a *shared utility* library might still win goodwill. |
| Light protection: keyfile, no activation (u-he, Sonic Charge) | **Fits** | Matches the plan's signed licence file. Evidence says heavier protection does not raise sales. |
| u-he style unlimited demo with noise | **Partly fits** | A noisy demo of the real plugin would let people test it in their DAW. The browser version already covers "try before you buy", so this is optional. |
| Lifetime free updates (Xfer, Baby Audio) | **Fits** | Costs little for small plugins and builds trust. Promise "updates for this major version" to keep a door open. |
| Publisher for Max for Live (Isotonik) | **Partly fits (TAP)** | A route for TAP to reach Ableton buyers without Jimmy doing marketing. Terms unknown, and Jimmy gives up price control. |
| Bastan's three channels | **Partly fits (TAP)** | Sell TAP via a publisher or Jimmy's own store, give away a small Max device for reach, and pitch Ableton only once there is a following. |
| Max for Live first, plugin later (Surreal Machines) | **Fits later** | Exactly the plan's path, but Surreal Machines got the Ableton store first. For Jimmy the order is reversed: plugins first, TAP stays Max for Live. |
| Platform distribution (Plugin Alliance) | **Does not fit now** | Needs a polished, proven product. Revisit if one plugin takes off. |
| Black Friday participation | **Partly fits** | Retail sale culture is strong [86], and Valhalla's opt-out needs a reputation. A single yearly sale on old bundles is enough. |
| Sokpop: itch singles, then Steam back catalogue | **Partly fits** | Steam doubled Sokpop's revenue, but Steam charges US$100 per product. Put a yearly collection on Steam, not each plugin. |
| itch co-op bundles | **Fits (with a friend)** | The only itch tool that splits revenue between people. Each person sells their own plugins and shares a bundle. |
| Making discontinued plugins free (Audio Damage) | **Fits later** | Old passes and old plugins become free tasters, which feeds the email list. |

## Stage 3: Investment and trade-offs

All money in AUD at about A$1.50 to US$1. All time and revenue figures are **[inference]**: estimates for a hobby developer, not sourced numbers. Revenue ranges are deliberately wide because no comparable maker publishes figures; the anchors are Moonbase's survey (49% of surveyed firms under US$20,000 a year) [47] and one developer's report of 14 sales of a US$10 plugin in two weeks, then 5 in five months [87].

### 3.1 Options one by one

| Option | Upfront time | Upfront A$ | Ongoing A$ a year | Effort to maintain | Revenue potential (guess) | Main risks | What it trades away |
|---|---|---|---|---|---|---|---|
| A. itch singles at A$5 to A$15 | 1 to 2 days to set up store and first page | 0 | 0 fixed; about 10% + US$0.30 + 2.9% a sale [70] | Low: pipeline uploads with butler [75]; pages by hand | Low: a few to tens of sales per plugin a year | Flat fee takes about 12% of a A$5 sale; itch is a games-first audience | Nothing much; itch's brand sits beside yours |
| B. Yearly pass as one itch project with growing files | 1 day, plus a test purchase | 0 | 0 | Low: one more upload per release | Medium relative to singles: one bigger sale per fan | Untested mechanics; if new files do not reach buyers, you hand out keys by hand | Some single-sale revenue from fans |
| C. Year-end bundle (itch bundle) | Half a day per year | 0 | 0 | Very low | Steady trickle; good for a yearly sale | Bundles freeze contents [73], fine for a closed year | Discounts the catalogue |
| D. Free browser version per plugin | Shared web build path; 1 to 3 days per plugin for UI and page | 0 (Cloudflare Pages free, per plan) | 0 to domain | Medium: two targets per plugin, browser changes | Unknown; no conversion data exists | Funnel may not convert; extra work per plugin | Time that could go into plugins |
| E. Free "taster" plugin (TDR, Aberrant pattern) | 1 to 3 days per year | 0 | 0 | Low | Indirect: list sign-ups, reach on plugin sites | Cannibalises some sales if too generous | A little exclusivity |
| F. Signed licence file (as planned) | About a weekend | 0 | 0 | Very low | Protects honest buyers' experience, not revenue | Shared files; cracks | Very little |
| G. Online activation (Moonbase or Lemon Squeezy) | 2 to 4 days | 0 | Moonbase: 0 under 10,000 validations a month [84]; store fees per sale | Medium: a service you depend on | Little measurable gain (u-he's view) [13] | Service changes (Lemon Squeezy moving to Stripe) [82]; user friction | Offline simplicity; goodwill |
| H. Mac version | Several days for build and notarisation set-up | 149 a year (Apple), plus a Mac if none | 149 | Medium: two platforms to test | Could roughly double addressable buyers **[inference]** | Cost before demand is proven | Time and A$149 a year |
| I. TAP as `.amxd` on itch or own store | 1 to 2 days | 0 | 0 | Low | Small: Max for Live audience only | Buyers can open and copy the patch [60] | Control of the code |
| J. TAP via Isotonik | A pitch and packaging | 0 | 0 | Low for Jimmy | Unknown: publisher reach, unknown split | Unknown terms; loss of price control | Margin and brand |
| K. Steam: one yearly collection | 2 to 4 days (Steamworks set-up, page, builds) | About 150 per product [77] | 0 fixed; 30% share [79] | Medium | Sokpop's Steam doubled revenue [68], but games are a different market | Plugin buyers rarely shop on Steam **[inference]**; tax and identity paperwork | A$150 and a 30% cut |
| L. Black Friday and one yearly sale | Half a day | 0 | 0 | Very low | Short spike | Trains buyers to wait for sales [48] | Full-price sales |
| M. Patreon or subscription | 1 day | 0 | Patreon fees | High: monthly obligation | Needs about 1,000 fans at US$3 (Sokpop scale) [66] | Burn-out; disliked by Jimmy | Freedom of schedule |
| N. Co-op bundle with a friend | Half a day | 0 | 0 | Low | Same as C, split | Whole-percent splits only [71]; Payouts mode needed for both | Some control |

### 3.2 Recommended paths

#### Path 1: Minimal ("The Shelf")

- **What:** itch only. Windows VST3 zips. Singles at A$5 to A$12. A yearly pass as one itch project (option B), tested first. Free browser version per plugin with a link and click count to the store. Signed licence file. An email list on a free tier. No sales except a year-end bundle.
- **Upfront:** about A$0 beyond the domain. Time: about 1 to 2 weeks of evenings for the store, pass test, site and pipeline, on top of building the plugins.
- **Ongoing:** about A$0 a year fixed. Per sale: itch's share plus processor fee.
- **Revenue guess [inference]:** A$0 to about A$1,500 in the first year.
- **Trades away:** Mac buyers; Ableton-only buyers of TAP; reach from a free plugin.
- **Move to Path 2 when:** the email list passes about 300 people, or plugins pass about 50 sales in a year, or Mac users ask for the plugin repeatedly. (These thresholds are judgement calls, not sourced.)

#### Path 2: Balanced ("The Catalogue"), recommended

- **What:** Path 1, plus: a free "taster" plugin each year built from that year's plugins (Aberrant pattern); TDR-style free and paid versions where a plugin has an obvious "more"; Mac once Path 1's triggers are met; TAP sold as `.amxd` on itch with a short pitch to Isotonik; one sale a year on last year's bundle; old passes made free after two or three years (Audio Damage pattern); a co-op bundle if the friend joins.
- **Upfront:** A$149 a year for Apple, plus a Mac if neither person has one (one-off hardware cost). Time: about 3 to 5 extra days a year for the taster and sale.
- **Ongoing:** about A$150 a year fixed.
- **Revenue guess [inference]:** A$500 to about A$5,000 a year once there are 8 to 12 plugins. Wide uncertainty.
- **Trades away:** the A$149 Apple fee before demand is certain; some single sales to the taster; publisher margin if Isotonik takes TAP.
- **Move to Path 3 when:** one plugin clearly outsells the rest, or yearly revenue covers the Apple fee several times over, or a publisher or store approaches you.

#### Path 3: Ambitious ("The Label")

- **What:** Path 2, plus: the previous year's collection as one Steam product; a Windows code-signing certificate if SmartScreen complaints appear; listing on a reseller such as Plugin Boutique; online activation through a plugin-specific service (Moonbase) if sharing becomes visible; a stronger launch for each release (video, press notes to plugin news sites).
- **Upfront:** about A$150 per Steam product, a few hundred A$ a year for a code-signing certificate (not priced in this research), more time for marketing.
- **Ongoing:** A$150 Apple + certificate + Steam fees per new collection.
- **Revenue guess [inference]:** A$2,000 to A$20,000 a year is possible, but most surveyed firms sit under US$20,000 [47]. This is the path where the JUCE Starter cap (US$20,000) starts to matter.
- **Trades away:** hobby scale. It needs regular marketing time, which competes with building plugins and with a fixed release schedule.

### 3.3 Comparison of the paths

| | Path 1: Shelf | Path 2: Catalogue | Path 3: Label |
|---|---|---|---|
| Upfront A$ | 0 | 149 + maybe a Mac | 149 + certificate + 150 per Steam product |
| Fixed A$ a year | 0 | about 150 | about 150 + certificate + Steam per product |
| Extra time a year beyond plugins | Low | Medium | High |
| Platforms | Windows | Windows, Mac | Windows, Mac, maybe Linux |
| Funnel | Browser version | Browser version + free taster plugin | All, plus press and resellers |
| Protection | Signed licence file | Signed licence file | Licence file, activation if needed |
| Revenue guess | A$0 to 1,500 | A$500 to 5,000 | A$2,000 to 20,000 |
| Biggest risk | Nobody finds it | Mac cost before demand | Marketing load breaks the schedule |

## 4. Where the findings change or contradict the business plan

| Plan section | What the plan says | What this research found | Suggested change |
|---|---|---|---|
| 5. Pricing, yearly pass | Open question which store lets a buyer receive later additions | itch bundles appear frozen at purchase; creators hand out keys by hand [73] | Build the pass as one itch project and upload each plugin to it; test with a A$1 test product before launch **[inference]** |
| 5. Pricing | Pass framed as unique | Puremagnetik runs the same shape as a subscription, then sells singles and a catalogue bundle [39][40][41] | Copy Puremagnetik's sequence: pass price well below the sum of singles, singles at full price later, a catalogue bundle at year end |
| 7. Website and funnel | Free browser version is the funnel | Most indie makers use a free *plugin* as the funnel; no evidence of browser-demo conversion was found | Keep the browser version, add a yearly free taster plugin, and measure web-demo to store clicks |
| 6. Copy protection | Signed licence file, maybe activation | Consistent with u-he, Sonic Charge, Vital [11][28][33] | No change. Add: offline only, any number of personal machines, as Sonic Charge does |
| 4. Store | Gumroad and Lemon Squeezy as fallbacks | Gumroad: one seller's payouts stopped for months [58]; minimum payout reportedly US$100 [81]. Lemon Squeezy is steering users to Stripe Managed Payments [82]. Paddle wants custom terms under US$10 [83] | Treat itch as the main store and Moonbase or Gumroad as fallbacks only after checking payouts to Australia |
| 4. Store, Steam | "Worth a later look" | US$100 *per product* [77] | Put one yearly collection on Steam, not single plugins |
| 2. TAP's format | Open | Max for Live sells through publishers (Isotonik), Ableton's store (Surreal Machines) or the maker's own store; `.amxd` files stay editable after freezing [60] | Sell TAP as Max for Live now (own store, maybe Isotonik); accept that the patch is readable, or keep its core in a compiled external |
| 8. Release cadence | Every six weeks is respectable | Puremagnetik sustains monthly; Sokpop's patrons asked for fewer releases [67] | Six weeks or monthly is fine; a predictable date matters more than frequency **[inference]** |
| 10. Costs, break-even | About 20 sales a year covers Apple | Realistic first-year sales per plugin may be in single or low double digits [87] | Keep Mac behind a demand signal, as the plan already says |
| 11. Tax | GST threshold A$75,000; MoR collects overseas VAT | Exports of digital products are generally GST-free; ATO rules let a platform take responsibility for GST on some sales [88] | No change; ask an accountant how itch's Payouts mode is treated for an Australian seller |

## Open questions

1. Does an itch.io buyer of a project automatically get files uploaded to that project later? Test with a cheap test product before building the pass around it.
2. What revenue split and terms does Isotonik offer Max for Live developers?
3. Can an Australian seller receive Gumroad and Moonbase payouts reliably in 2026?
4. How does itch.io's Payouts mode treat Australian GST for an Australian seller selling to Australian buyers?
5. Do plugin buyers use Steam for audio plugins at all? No first-party data was found.
6. What does a free browser version actually convert at? Track clicks from the web demo to the store from the first release.
7. Can a Max for Live device's core logic be shipped in a compiled external to reduce copying, and does that work on both Windows and Mac?
8. What was in Sean Costello's "Why is Supermassive free?" post? Fetch it once `valhalladsp.com` is allowed.
9. Current Kilohearts, Plugin Alliance, Tokyo Dawn and Aberrant DSP prices from their own pages (only secondary or undated extracts were seen).
10. Windows code-signing options available to Australians, with prices (carried over from the plan).

## Sources

All seen as search-engine extracts on 2026-11-09; none could be fetched. "(secondary)" marks a page not owned by the developer or platform.

1. Valhalla DSP FAQ, pricing: https://valhalladsp.com/faq/pricing/why-are-your-plugins-so-affordable-you-should-charge-more-money-for-these-seriously-im-not-trying-to-be-cheesy-marketing-guy-i-get-asked-this-question-a-lot
2. Valhalla DSP FAQ, bundles: https://valhalladsp.com/faq/pricing/i-want-to-buy-several-of-your-plugins-can-i-get-a-bundle-deal-i-get-asked-this-one-a-lot-too-the-yin-to-the-yang-of-the-above-question
3. Valhalla DSP FAQ, Black Friday: https://valhalladsp.com/faq/pricing/do-you-have-black-friday-sales
4. Valhalla DSP FAQ, resellers: https://valhalladsp.com/faq/purchasing/does-valhalla-dsp-sell-plugins-through-resellers
5. Valhalla DSP, "Why is Valhalla Supermassive Free?": https://valhalladsp.com/2020/05/27/why-is-supermassive-free/
6. Bonedo, Supermassive review (secondary): https://www.bonedo.de/artikel/valhalla-dsp-supermassive-test
7. Airwindows Consolidated: https://www.airwindows.com/consolidated/
8. Airwindows posts on Patreon and MIT licence: https://www.airwindows.com/page/116/
9. KVR, Airwindows Consolidated release (secondary): https://www.kvraudio.com/news/airwindows-releases-airwindows-consolidated---clap-vst3-audio-unit-and-lv2-free-60736
10. Synth Anatomy, Consolidated update March 2026 (secondary): https://synthanatomy.com/2026/03/airwindows-consolidated.html
11. u-he FAQ, copy protection: https://u-he.com/support/faq/licenses/copy-protection/
12. u-he FAQ, demos: https://u-he.com/support/faq/licenses/demos/
13. KVR forum thread with posts attributed to u-he (forum): https://www.kvraudio.com/forum/viewtopic.php?p=6285121
14. Tokyo Dawn Labs, TDR Nova: https://www.tokyodawn.net/tdr-nova/
15. Tokyo Dawn Labs, TDR Nova GE: https://www.tokyodawn.net/tdr-nova-ge/
16. KVR, Nova update and pricing (secondary): https://www.kvraudio.com/news/tokyo-dawn-labs-updates-nova-standard-and-ge-32000
17. Kilohearts Essentials: https://kilohearts.com/products/kilohearts_essentials
18. Kilohearts subscription: https://kilohearts.com/subscribe
19. Kilohearts FAQ, subscriber rewards: https://kilohearts.com/faq/what_are_the_subscriber_rewards
20. Kilohearts Ultimate (with retailer price listings, secondary): https://kilohearts.com/products/kilohearts_ultimate ; https://equipboard.com/items/kilohearts-ultimate
21. Attack Magazine, interview with Caspar Bock (developer speaks): https://www.attackmagazine.com/features/interview/a-smooth-operator-caspar-bock-founder-of-baby-audio/
22. Winter Music Conference, interview with Caspar Bock Soerensen, 2026 (developer speaks): https://wintermusicconference.com/2026/08/12/caspar-bock-soerensen-interview-baby-audio/
23. Plugin Boutique, Baby Audio listing (retailer, secondary): https://www.pluginboutique.com/manufacturers/263-baby-audio
24. KVR, Aberrant DSP Lofi Oddity (secondary): https://kvraudio.com/news/aberrant-dsp-releases-free-plugin-lofi-oddity-for-macos-and-windows-vst3-au-and-aax-59436
25. KVR, SketchCassette II (secondary): https://www.kvraudio.com/news/aberrant-dsp-releases-sketchcassette-ii-for-macos-and-windows-vst3-au-and-aax-50080
26. Plugin Boutique, Aberrant DSP listing (retailer, secondary): https://www.pluginboutique.com/manufacturers/283-Aberrant-DSP
27. Sonic Charge products: https://cdn.soniccharge.com/products
28. Sonic Charge forum, Synplant 2 licence (developer post): https://beta.soniccharge.com/forum/topic/2543-synplant-2-how-many-computers-is-the-license-for
29. Attack Magazine, Serum 2 (secondary): https://www.attackmagazine.com/news/serum-2-is-finally-here-and-its-free-for-previous-owners/
30. KVR, Serum 2 free update (secondary): https://www.kvraudio.com/news/xfer-records-releases-serum-2---free-update-63286
31. Splice support, About Rent-to-Own: https://support.splice.com/en/articles/8652687-about-rent-to-own
32. Splice, Serum page: https://splice.com/plugins/15493-serum
33. KVR, Vital product listing (secondary): https://www.kvraudio.com/product/20186
34. MusicTech, Vital tiers (secondary): https://www.musictech.net/news/matt-tytel-vital-free-synth/
35. Linux Magazine, Vital GPLv3 (secondary): https://www.linux-magazine.com/index.php/Issues/2021/248/FOSSPicks/(offset)/9
36. Surge Synth Team, Surge XT: https://surge-synth-team.org/surge-xt
37. Audio Damage, support: https://www.audiodamage.com/pages/support
38. Audio Damage, about: https://www.audiodamage.com/pages/about
39. Puremagnetik, subscription FAQ: https://puremagnetik.com/pages/lore-subscription-faq
40. Synth Anatomy, Puremagnetik pricing 2023 (secondary): https://synthanatomy.com/2023/07/puremagnetik-foam-tensol-leems-sphine-new-pedal-style-synths-and-fx-plugins.html
41. Puremagnetik, The Cube: https://puremagnetik.com/products/plugin-compendium
42. Plugin Alliance press release, Unfiltered Audio partnership: https://files.plugin-alliance.com/images/global/mediaresources/pressreleases/Plugin%20Alliance%20Teams%20Up%20with%20Unfiltered%20Audio.pdf
43. Plugin Alliance, Unfiltered Audio collection and MEGA: https://www.plugin-alliance.com/collections/unfiltered-audio ; https://plugin-alliance.com/en/products/pa-mega-bundle.html
44. Gearnews, MEGA subscription launch (secondary): https://www.gearnews.com/plugin-alliance-mega-bundle-subscription-goes-live-with-full-yearly-access-for-250
45. Synth Anatomy, Dirk Ulrich reacquires Plugin Alliance, July 2026 (secondary): https://synthanatomy.com/2026/07/dirk-ulrich-reacquires-plugin-alliance-and-brainworx-from-inmusic.html
46. Bedroom Producers Blog, Komplete 26 (secondary): https://bedroomproducersblog.com/2026/05/05/native-instruments-komplete-26/
47. Moonbase, State of Audio Plugin Companies 2025 (vendor survey): https://moonbase.sh/reports/state-of-audio-plugin-companies-2025/ ; https://moonbase.sh/articles/state-of-audio-plugin-companies-2025-report/
48. Moonbase, 12 Pricing Models for Music Software (vendor article): https://moonbase.sh/articles/12-pricing-models-for-music-software/
49. Isotonik Studios, about and Collective: https://isotonikstudios.com/about ; https://isotonikstudios.com/product-category/isotonik-collective
50. Isotonik Studios, Push 3 Control Master: https://isotonikstudios.com/product/push-3-control-master/
51. Isotonik Studios, Iota: https://isotonikstudios.com/product/iota-granular-synthesiser
52. Isotonik Studios, data.train: https://isotonikstudios.com/product/data-train-by-dillon-bastan/
53. Isotonik Studios, Strange Mod: https://isotonikstudios.com/product/strange-mod-by-dillon-bastan/
54. Ableton blog, Dillon Bastan interview (developer speaks): https://www.ableton.com/fr/blog/dillon-bastan-water-ripples-and-sound-waves
55. Ableton, Inspired by Nature: https://www.ableton.com/en/packs/inspired-nature/
56. Ableton, Dub Machines: https://www.ableton.com/en/packs/dub-machines/
57. KVR, Surreal Machines VST/AU release (secondary): https://www.kvraudio.com/news-print.php?id=37540
58. Akihiko Matsumoto, Gumroad product page with payout notice: https://akihiko-matsumoto.gumroad.com/l/AbletonTapDelay
59. Cycling '74 support, Max for Live device questions: https://support.cycling74.com/hc/en-us/articles/360049847974-Common-questions-about-Max-for-Live-devices
60. Cycling '74 docs, Freezing Max for Live devices: https://docs.cycling74.com/userguide/m4l/live_freezing/
61. Ableton blog, Learn Synthesis in Your Browser: https://www.ableton.com/en/blog/learn-synthesis-in-your-browser/
62. Cycling '74 RNBO, Learning Synths: https://rnbo.cycling74.com/explore/learning-synths-and-rnbo
63. Sevish, Leimma and Apotome (secondary): https://sevish.com/2021/leimma-apotome-two-new-web-tools-for-microtonal-music/
64. Bungee web demo: https://bungee.parabolaresearch.com/bungee-web-demo
65. GDC Vault, One Year of Sokpop Games (Sokpop speak): https://gdcvault.com/play/1025669/One-Year-of-Sokpop-Games
66. PC Gamer, Sokpop two games a month (secondary): https://www.pcgamer.com/somehow-the-sokpop-collective-has-released-two-games-a-month-for-two-years
67. Game Pilgrim, Sokpop interview, January 2021 (developers speak): https://gamepilgrim.com/2021/01/20/developer-interview-sokpop-collective/
68. GameDiscoverCo, Sokpop and Steam, 2022 (Aran Koning quoted): https://newsletter.gamediscover.co/p/how-one-of-sokpops-almost-100-steam
69. EUGames, Sokpop article quoting Tijmen Tio (secondary): https://eugames.pages.dev/posts/the-sokpop-collective-an-alternative-vision-of-game-development
70. itch.io docs, payments: https://itch.io/docs/creators/payments
71. itch.io, Introducing co-op bundles: https://itch.io/updates/introducing-co-op-bundles
72. itch.io docs, bundles: https://itch.io/docs/creators/bundles
73. itch.io forum posts on bundle contents after purchase (forum): https://itch.io/post/14646466 ; https://itch.io/post/15412673/view-in-topic
74. itch.io docs, access control: https://www.itch.io/docs/creators/access-control
75. itch.io docs, pushing builds with butler: https://itch.io/docs/butler/pushing
76. itch.io, audio plugin and VST tools listing: https://itch.io/tools/newest/tag-audio-plugin/tag-vst
77. Steam Direct: https://partner.steamgames.com/steamdirect
78. Steamworks docs, app fee: https://partner.steamgames.com/doc/gettingstarted/appfee
79. Fortune, Steam revenue share tiers (secondary): https://fortune.com/2018/12/01/steam-valve-developer-revenue-agreement/
80. Acoustica, Humble Bundle announcement: https://acoustica.com/news/humble-bundle-make-music-change-lives
81. Gumroad fees and payouts 2026 (secondary): https://www.wearefounders.uk/gumroad-fees-2026-what-sellers-actually-pay-per-sale/ ; https://roo.beehiiv.com/p/gumroad-fees-2026
82. Lemon Squeezy, 2026 update: https://www.lemonsqueezy.com/blog/2026-update
83. Paddle pricing: https://paddle.com/pricing
84. Moonbase docs, fees: https://moonbase.sh/docs/fees ; https://moonbase.sh/pricing/
85. KVR forum, companies using serial, keyfile or watermark protection (forum): https://www.kvraudio.com/forum/viewtopic.php?p=9015436
86. Dubspot, Plugin Boutique Black Friday listing (secondary): https://blog.dubspot.com/fr/black-friday/plugin-boutique
87. JUCE forum, "Indie VST Developer Expectations" (developer reports own sales): https://forum.juce.com/t/indie-vst-developer-expectations/20518
88. ATO, Australian consumers importing goods and services (EDP rules): https://www.ato.gov.au/businesses-and-organisations/gst-excise-and-indirect-taxes/gst/in-detail/rules-for-specific-transactions/international-transactions/australian-consumers-importing-goods-and-services
89. MakeUseOf, Audio Damage plugins guide (secondary): https://www.makeuseof.com/audio-damage-vst-plugins-guide/
90. Sokpop Patreon post on switching to one game a month (as cited in the business plan): https://www.patreon.com/posts/important-switch-47586913
91. Rock Paper Shotgun report mirror, Sokpop after 100 games (secondary): https://rockpapershotgunz.pages.dev/posts/sokpop-collective-are-releasing-their-100th-game-and-aiming-for-100-more
