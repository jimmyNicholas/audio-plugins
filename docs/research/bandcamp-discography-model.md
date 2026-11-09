# Research: the "Bandcamp discography" model for many small, cheap tools

Researched 2026-11-09. Purpose: to test whether Jimmy can sell many small, quick, unique items (plugins, Max for Live devices, sample packs, tools) cheaply one by one, plus "everything" as one cheap "full digital discography", as music labels do on Bandcamp. It also checks whether Bandcamp allows plugins at all, what it costs an Australian seller, and how the model compares with itch.io, Gumroad, Lemon Squeezy and Payhip. The work follows the same three stages as [`indie-plugin-monetisation.md`](indie-plugin-monetisation.md): **Observe**, **Apply**, **Invest**. Section 4 lists where the findings change [`plugin-business-plan.md`](../business/plugin-business-plan.md).

## How to read this

- **Method caveat (important):** one page fetch was tried (`get.bandcamp.help`) and failed DNS because the host is outside the session's network allowlist, so no further fetches were tried. **Every claim rests on search-engine extracts, not full reads.** No claim is tagged **[read]**. Several key Bandcamp help pages (bonus item file types, the full discography article, the Stripe country list) came back truncated, so the exact rules are partly unconfirmed. To read them first-hand, `get.bandcamp.help` and `bandcamp.com` must be allowed in the session's network settings.
- **Evidence tags:** **[extract]** means the claim comes from a search-engine extract of the cited page. **[inference]** means the researcher's reasoning, not a source. **(secondary)** after a citation means the page belongs to a journalist, retailer, forum user or aggregator, not to Bandcamp, the store or the seller. Secondary sources are used for context only.
- **Dates:** every price, count and fee was seen in search extracts on 2026-11-09 unless a different date is given. Bandcamp catalogue counts change every release; treat them as "about".
- **Money:** AUD figures convert USD at about A$1.50 to US$1, as the business plan does. **No seller in this survey publishes revenue or sales figures.** No revenue figure below is sourced; all are labelled **[inference]**.
- **Confidence per case:** High = a first-party source plus an independent source agree. Medium = one first-party source, or several secondary sources that agree. Low = a single secondary source, or inference.

## TL;DR

| # | Question or case | Answer in one line | Confidence |
|---|---|---|---|
| 1 | How the full discography offer works | Seller opts in; needs at least 3 purchasable albums (2015 rule); discount none or 5% to 90%; followers are told when it is switched on; can be gifted | Medium |
| 2 | Does it include future releases? | **No source says it does.** It adds the releases that exist at purchase. Bandcamp's separate **subscription** feature is the tool for future releases, and it is recurring | Medium (negative result) |
| 3 | Can you sell plugins on Bandcamp? | **Not as a normal product.** Tracks must be lossless audio; bonus items are limited to specific file types, and Bandcamp said it avoided general zips because they "could contain exe's". Two Max for Live sellers use **merch listings and email the device by hand** | Medium |
| 4 | Sample packs on Bandcamp? | **Yes, commonly**, as "albums" of WAV tracks. Soul Chemist (89 releases, 90% off discography) and Samples Unlimited (96 releases) do exactly the discography model | Medium |
| 5 | Fees | 15% of digital sales, 10% once you pass US$5,000 in 12 months; processing on sales under US$8.07 is 5% + US$0.05, which suits A$5 items | Medium |
| 6 | Tax | Bandcamp calculates, collects and remits GST for Australian buyers and VAT for the EU, UK and others; elsewhere it is the seller's job | Medium |
| 7 | Payouts to Australia | PayPal by default; Stripe payouts are rolling out but Australia's place on the list was not confirmed | Low |
| 8 | Ownership in 2026 | Songtradr since 2023 (about half the staff cut); about 10 engineers laid off in June 2026 (secondary); no 2026 sale found | Medium |
| 9 | Audience | Music fans first; followers are emailed every new release; a smaller producer audience buys sample packs | Medium |
| 10 | Geometric Lullaby | Vaporwave label, about 186 to 187 releases, discography at 90% off, releases "free to download", subscription with merch discount | Medium |
| 11 | "Signal wave" | Most likely **signalwave**, a vaporwave and slushwave subgenre whose artists sell whole catalogues "hugely discounted" on Bandcamp. No plugin or seller of that name found | Medium |
| 12 | Which store can sell "everything, including future items"? | **itch.io single project** (docs: new files reach buyers automatically), **Payhip bundles** (new products reach existing buyers after about an hour), **Gumroad** (update files and email buyers). Bandcamp discography and itch bundles are "everything so far" | Medium |

**Main findings [inference, from the cases cited below]:**

1. **Bandcamp is a music store, not a software store.** Plugins and `.amxd` devices cannot be first-class Bandcamp releases and cannot be inside the discography offer except, at best, as bonus files, and the bonus file list probably excludes zips and executables. Plan plugins elsewhere.
2. **Sample packs and music fit Bandcamp's discography model very well**, and real sample sellers run it with 89 to 96 releases at deep discounts.
3. **The discography offer rewards a big back catalogue**: the more small releases, the larger the headline discount (90% is common). It does not deliver future items, so it is a "catch up on everything" offer, not a pass.
4. **itch.io's own docs now confirm the yearly pass works as one project**: new uploads reach earlier buyers automatically, unless an upload's minimum price is higher than what the buyer paid. This answers the business plan's first open question.
5. **Recommended combination:** plugins and devices on itch (singles plus a growing yearly pass project); music and sample packs on Bandcamp with the discography offer; each store points to the other. Payhip is the fallback if a true "everything forever" bundle is wanted.

## Stage 1: Observe

### 1.1 How Bandcamp's full discography offer works. Confidence: Medium

- **Opt-in, on the profile page:** it is not enabled by default. If the seller is eligible, a "Full Digital Discography Pricing" section appears on the Profile page; if it does not, "you probably don't meet all the requirements yet" **[extract]** [1].
- **Eligibility:** a 2015 Bandcamp blog post says at least three purchasable albums are needed **[extract]** [2]. The help article says the option "isn't yet available at the label level" **[extract]** [1], but Geometric Lullaby, a label, shows a discography offer [10], so that line looks out of date or depends on account set-up **[inference]**.
- **Discount:** no discount, or 5% to 90% off the sum of the releases **[extract]** [2][3 (secondary)]. The 90% ceiling matches what sellers show ("save 90%") [10][15].
- **Followers are told:** once it is enabled and the profile saved, Bandcamp notifies all followers automatically **[extract]** [1].
- **Gifting:** buyers can send the bundle as a gift to an email address **[extract]** [1 via secondary guide].
- **What buyers receive:** Geometric Lullaby's wording is "Get the entire ... Discography Instantly for Streaming and Download. This adds all of our music to your Bandcamp Collection" **[extract]** [10]. So the buyer gets every current purchasable digital release, in their collection, with Bandcamp's normal formats (MP3, FLAC and so on) **[inference]**.
- **Future releases:** **no source found says later releases are added.** The help article extract does not mention it; the wording "available on Bandcamp" and "adds all of our music" describes what exists at purchase **[extract, negative result]** [1][10]. The tool Bandcamp built for future releases is the **artist subscription** (fans pay monthly or yearly for access to future music, optionally the back catalogue, plus extras and a merch discount) **[extract]** [4 (secondary)][5]. Conclusion: discography = "everything so far" **[inference, Medium]**.
- **Merch is separate:** the discography covers digital releases; nothing found says merch items are included **[inference]**.

### 1.2 Can you sell software or plugins on Bandcamp? Confidence: Medium

**Short answer: not as a normal Bandcamp product.** Bandcamp's upload rules and bonus item rules are built for audio. The sellers found who sell devices on Bandcamp use a manual workaround.

What the sources say:

- **Tracks must be lossless audio:** uploads must be `.aiff`, `.wav` or `.flac`, at least 16-bit, 44.1 kHz, stereo **[extract]** [6]. A "release" is therefore always audio.
- **Bonus items exist, for albums only:** "media you can add to your digital album that will be included automatically in the zip file when a fan downloads your album", added via "add bonus item" in the album editor; not possible on standalone tracks **[extract]** [7]. Examples Bandcamp gives: PDF booklets, tabs, sheet music, videos, alternate art **[extract]** [7].
- **File types are restricted:** the help article has a section "Here are all the file types we support", but its list did not come through **[extract, truncated]** [7]. Bandcamp's 2009 post on the feature said it would "rather allow the specific file types than just generally allow zip", because zips "could contain exe's and other scary stuff" **[extract]** [8]. **[inference]** A `.vst3` bundle, `.dll`, `.exe`, `.amxd` or `.zip` is probably not on the list. **Verify the current list before relying on this.**
- **Terms of use:** the terms define "Content" broadly, including software and scripts, and require that you own or control all rights to what you upload **[extract]** [9]. That is a definition, not a permission. No clause found either allows or bans selling software; the Acceptable Use and Content Moderation Policy could not be read in full **[extract, negative result]** [9][37].
- **AI rule (January 2026):** "music or audio generated wholly or in substantial part by AI is not permitted" **[extract]** [37]. This covers audio, not tools; it matters if any sample pack or demo audio used generative AI **[inference]**.
- **Merch listings:** a Bandcamp merch item can include a digital album or track download **[extract]** [32]. No help page found describes a merch item that is itself a download. Digital pre-orders cannot be attached to non-music merch **[extract]** [32]. A 2012 user request on Bandcamp's blog asked for downloadable merch such as sheet-music PDFs; no later answer was found **[extract]** [18].

How the sellers actually do it:

| Seller | What | How it is sold on Bandcamp | Source |
|---|---|---|---|
| HIDE (H-4) | Max for Live devices numbered X02 to X13 (Vela, ReTrig, BlackBox and others) | **Merch** listings; Vela €15; "M4L Bundle #2 [X06-X10]" of five devices €36.90; device "ships by email within 24 to 48 hours"; page says redistribution not allowed | **[extract]** [11] |
| Datashader | SeqTool#1 Max for Live device | **Merch** listing for the Pro version: "We will send it to you by email after purchase"; free Lite version on maxforlive.com | **[extract]** [12][13] |
| Peaks of Valleys | "Textures (Ableton Live Pack)", 9 instrument racks | Listed as an **album**, US$3 or more, 24-bit audio; how the Live files are delivered was not visible (bonus item is the likely route) | **[extract]** [14] |
| SampleScience | Dozens of VST plugins and sample libraries | Bandcamp used only for **audio demo** albums; plugins sold on Gumroad and its own site | **[extract]** [19][20 (secondary)] |
| Akihiko Matsumoto | Ableton project and Max for Live packs | Bandcamp album links out to a **Gumroad** pack | **[extract]** [14 (search summary)] |

**Pattern [inference]:** device sellers on Bandcamp treat the device as "merch" and email it by hand. That does not scale to a large catalogue, carries a 1 to 2 day delay, sits outside the discography offer, and relies on Bandcamp tolerating a non-physical "merch" item that its help pages do not describe. **For Jimmy, plugins should not be planned on Bandcamp.**

### 1.3 Fees, payments, tax and ownership. Confidence: Medium

**Fees:**

- Revenue share: 15% of digital sales, dropping to 10% once you reach US$5,000 in sales and staying there while you have made at least US$5,000 in the past 12 months; 10% on physical items, excluding shipping and tax **[extract]** [21].
- Payment processing, on top: for transactions under US$8.07, Bandcamp switches to the processor's alternate rate of **5% + US$0.05**; at or above US$8.07, about 2.2% + US$0.30 by card or 1.9% + US$0.30 by PayPal **[extract]** [22 (forum quoting the help page)][23]. Bandcamp's own summary: 4% to 7% on an average purchase **[extract]** [23].
- PayPal bulk payout fee: 1% capped at US$1 **[extract]** [22].
- Artist subscriptions: Stripe 2.9% + US$0.30 **[extract]** [24]; the current revenue share on subscriptions was not confirmed.
- Account costs: a free artist account includes the store, messaging and follower notifications; Bandcamp Pro is US$10 a month (batch uploads, private streams); label accounts from US$20 a month **[extract]** [25][26]. **[inference]** Jimmy needs only a free artist account.
- Free downloads: 200 free downloads a month are included for free or name-your-price-with-zero releases; more can be bought **[extract]** [27].

**Worked example [inference, from the fees above]:** a A$5 (US$3.33) sample pack nets about US$2.61 (78%) on Bandcamp at 15%; the same on itch at itch's default 10% nets about US$2.60, or about US$2.93 at a 0% itch share [itch fees from 38]. A US$20 discography nets about US$16.26 (81%) on Bandcamp. So Bandcamp's small-sale processing rate offsets its higher share at A$5 prices.

**Payouts:**

- Default: PayPal, 24 to 48 hours after a sale, or monthly; Bandcamp recommends a PayPal Business account **[extract]** [28].
- Bandcamp is moving payouts to Stripe in phases; where Stripe is unsupported, sellers keep PayPal. Stripe payouts land up to 3 days later, plus 24 hours outside the US **[extract]** [28][29]. Whether Australia is on Bandcamp's Stripe list was **not confirmed** (the list was truncated) **[extract, negative result]** [29].

**Tax (GST and VAT):**

- Bandcamp "is responsible for calculating, collecting, and remitting sales taxes (including VAT, US state sales tax, GST ...)" for certain transactions in listed places; the extract lists **Australia**, Canada, the EU, Iceland, New Zealand, Norway, Singapore, South Africa, South Korea, Switzerland and the UK, plus some US states. Elsewhere "it is your responsibility to collect and remit" **[extract]** [30].
- **[inference]** For an Australian seller under the A$75,000 GST threshold, Bandcamp handling GST on Australian buyers' purchases is convenient, but how the ATO treats that for the seller is a question for an accountant (as with itch in the business plan).
- Bandcamp collects US tax information and issues forms to sellers earning over US$5,000 a year **[extract]** [30].

**Bandcamp Fridays:**

- Bandcamp waives its revenue share for sales from 00:00 to 23:59 Pacific Time; processing fees still apply **[extract]** [31].
- 2026 dates: 6 February, 6 March, 1 May, 7 August, 4 September, 2 October, 6 November, 4 December **[extract]** [31 (secondary, Hypebot and RouteNote, matching)]. Next one after today: **4 December 2026**.
- Bandcamp Fridays had paid out US$154 million since March 2020, US$19 million of it in 2025; artists get an average of 82% of sales on normal days; over US$1.5 billion paid to artists and labels in total (December 2025) **[extract]** [33 (secondary, quoting Bandcamp)].

**Ownership and staff (seen 2026-11-09):**

- Epic Games bought Bandcamp in 2022 and sold it to Songtradr in 2023; Songtradr said "50% of employees received offers", so about half the staff (including half of Bandcamp Daily's editorial staff) left; Songtradr promised to keep "a marketplace and music community with an artist-first revenue share" **[extract]** [34 (secondary)].
- June 2026: Hypebot reports Bandcamp confirmed laying off about 10 engineers (another source says "most of the remaining engineers"), with Songtradr saying its commitment to artists "has not changed" **[extract]** [35 (secondary)].
- No 2026 sale, new owner or CEO change was found **[extract, negative result]**.
- New products since the sale: **Bandcamp Clubs** (September 2025), human-curated subscriptions at US$13 a month per club; members keep the monthly albums **[extract]** [36 (secondary)].
- January 2026: the "Keeping Bandcamp Human" AI policy **[extract]** [37].
- **[inference]** The platform keeps shipping features and paying out, but a thin engineering team raises the risk of slow fixes or policy drift. Do not make Bandcamp the only store for anything.

### 1.4 Audience and discovery. Confidence: Medium

- **Followers are emailed every release:** "We automatically notify your Bandcamp followers whenever you release new music or merch", and the notifications are emails **[extract]** [25][39].
- **Buying follows the artist:** fans follow by clicking "follow" or simply by making a purchase **[extract]** [39 (Bandcamp blog, 2013)].
- **Wishlists:** a fan can wishlist an item and Bandcamp periodically emails a digest with buy links (older post) **[extract]** [39].
- **Fan feeds:** fans can follow other fans and see their purchases **[extract]** [39].
- **Messaging:** artists can message followers, delivered on Bandcamp and by email **[extract]** [40].
- **Scale:** Bandcamp's 2024 release spoke of 47 million fans and 5 million artists **[extract]** [33 (secondary, press release)]. No data on how many are producers.
- **Producer audience exists but is smaller:** Bandcamp Daily ran "Build Your Own Beats With These Recent Sample Packs" (November 2020), listing "six of the best on Bandcamp" **[extract]** [41]. Sample sellers such as Soul Chemist, Samples Unlimited and Analog Samples sell there [15][16][17]. No evidence was found of a plugin-buying audience on Bandcamp **[extract, negative result]**.
- **Overlap with Jimmy's music:** vaporwave, slushwave, signalwave and dark ambient scenes are strongly Bandcamp-based (Geometric Lullaby, Cryo Chamber, signalwave artists) [10][42][ambient doc]. **[inference]** A Bandcamp music audience could overlap with buyers of lo-fi, tape and slushwave tools, but there is no evidence yet that music fans there buy plugins.

### 1.5 The named starting points

#### Geometric Lullaby. Confidence: Medium

- **What it is:** a vaporwave label founded by Dennis Mikula (also of Ghost Bath) in 2018; one wiki says Pittsburgh, another line says Iowa; describes itself as specialising in "high quality physical releases of vaporwave, ambient, and experimental artists"; sister labels Hushtones and Lifesick Lullaby **[extract]** [10][43 (tertiary wiki)].
- **What it sells:** digital releases on Bandcamp, plus vinyl and tapes (for example a t e l e p a t h split vinyl in 2024; a t e l e p a t h reissue via Discware) **[extract]** [44 (secondary)][45].
- **Count:** "all 186 Geometric Lullaby releases ... save 90%" in one extract, "187 releases" in another, so the catalogue grows continuously **[extract]** [10].
- **Discography offer:** yes, at the 90% maximum **[extract]** [10]. The money price was not visible.
- **Pricing of singles:** the subscription page says "every Geometric Lullaby release will remain free to download"; the top digital subscription tier gives 10% off merch **[extract]** [10]. **[inference]** Releases are name-your-price (possibly with a zero minimum), the discography is a "support us" purchase, and income comes mainly from physical editions and subscriptions.
- **Cadence:** about 186 releases from 2018 to 2026 is roughly 23 a year, or about two a month **[inference from counts]**.
- **Lesson for Jimmy [inference]:** a label can give everything away digitally and still sell a discography, because fans pay to support and collect. It works because of a large, steady release stream and physical products.

#### "Signal wave". Confidence: Medium (identification)

- **How identified:** searches for "Signalwave bandcamp", "Signal Wave" plus VST and Max for Live found **no plugin, device or tool seller of that name** **[extract, negative result]** [46][47]. The nearest tool is Isotonik's "Signal" (now "Modulat"), a Max for Live modulation kit, which does not fit the discography model **[extract]** [47].
- **Most likely meaning:** **signalwave**, a vaporwave subgenre of sound collage from TV and broadcast sources [48 (secondary)]. A 2024 Substack post calls signalwave and "broken transmission" subgenres of slushwave and says "these artists have their entire catalogs hugely discounted on Bandcamp" **[extract]** [42 (secondary)]. That matches the person's link between signalwave, slushwave and cheap discographies, and Jimmy's slushwave research.
- **Examples:** CT57 (Toronto), 13 releases on Bandcamp (2022 to 2024 dates seen) **[extract]** [49]; Signals From Nowhere, a label "specializing in Signalwave" **[extract]** [48]; "The Signalwave Christmas Special 2025" by The Signalwave Community **[extract]** [46]. Discount levels per artist were not seen.
- **Second candidate, Low:** a specific artist or label named "Signalwave" could exist that search did not surface. Ask the person for a link.

### 1.6 Other "everything, cheap" sellers of tools, devices and packs

| Seller | Store | Items | Single price | "Everything" price and discount | Future items included? | Confidence |
|---|---|---|---|---|---|---|
| The Soul Chemist | Bandcamp | 89 sample pack releases | Not seen | Full discography, **90% off** | No (Bandcamp discography) | Medium [15] |
| Samples Unlimited (Eliot Lipp and Michna) | Bandcamp, Gumroad mirror | 96 sample pack releases | Not seen | Discography bundle offered; discount not seen | No | Medium [16] |
| HIDE | Bandcamp (merch) | About 13 Max for Live devices | €15 (Vela) | Bundles of 3 to 5 devices (five for €36.90, about 50% off five singles) | No (fixed bundles) | Medium [11] |
| Datashader | Bandcamp (merch) + maxforlive.com | Max for Live devices | Pro paid, Lite free | Not seen | n/a | Medium [12][13] |
| dnksaus | Gumroad | 7 Max for Live devices | Not seen | "All devices at a discounted price"; **"will also grant access to future Max devices released"** | **Yes** | Medium [50] |
| Slim Collective | Gumroad | 2 VSTs + 4 sample packs + gift | Not seen | Everything Bundle US$145 | Not stated | Low [51] |
| Aurora DSP | Own site, resellers | Plugins (5 at first release) | Not seen | Everything Bundle about US$349 list, US$149 on a 2025 sale; "all current and future releases" | **Yes** | Low (resellers) [52 (secondary)] |
| Techivation | Own site | Plugins | Not seen | "Full Bundle" excludes future releases; "Full Access Bundle" includes them | Two tiers | Low [53 (search summary)] |
| ELV Games | itch.io | Game sound effect packs, new packs weekly | Not seen | Bundle buyers get a **24-hour claim window** for each new pack | **Yes, if claimed in 24 h** | Medium [54] |
| Pixelloops | itch.io | Music and sound effect packs | Not seen | Bundle that "will continue growing" | Stated yes | Low [55] |

**Update behaviour [inference]:** the sellers who promise future items all use a store where they can change a product after sale (Gumroad files, itch project files or claims, their own site). The Bandcamp sellers only offer "everything so far".

### 1.7 Comparing stores for "everything so far" versus "everything, including future"

| Store | "Everything so far" | "Everything including future" | How | Confidence |
|---|---|---|---|---|
| Bandcamp discography | Yes, automatic, 5% to 90% off | **No** (not documented) | Seller toggles it on | Medium [1][2] |
| Bandcamp subscription | Optional back catalogue | Yes, while subscribed | Monthly or yearly recurring fee | Medium [4][5][24] |
| itch.io bundle | Yes | **No**: forum reports say contents are frozen at purchase | Bundle tool | Medium [56 (forum)] |
| itch.io single project | Yes | **Yes**: "you can publish new files for your project and they will automatically be made available on the Download Key page", except uploads with a minimum price higher than what the buyer paid | Upload each new item to the same project | Medium [57] |
| Gumroad | Yes | **Yes** in practice: update the product's files and send an update to past buyers (dnksaus does this) | Manual upload; Gumroad's API cannot update products | Medium [50][58 (secondary)][59] |
| Payhip bundle | Yes | **Yes**: new products added to a bundle appear in existing buyers' accounts after about a one-hour delay | Add products to the bundle | Medium [60] |
| Lemon Squeezy | Yes | Probably, by updating files **[inference]**; Lemon Squeezy is steering users to Stripe Managed Payments [61] | Unverified | Low |

### 1.8 Why the model works or fails

**Evidence [extract]:** the sellers who run it have large catalogues (89, 96, 186 releases) and use the maximum 90% discount [10][15][16]. Signalwave artists are described as discounting "entire catalogs" [42]. Geometric Lullaby gives singles away and still sells a discography and a subscription [10]. Bandcamp emails followers each time a release goes up and when the discography offer is enabled [1][25].

**Mechanics [inference]:**

- **Catalogue growth raises the value:** the discography price is usually set as a percentage of the summed prices, so every new item makes the headline saving larger without lowering any single price.
- **Collectors and completionists:** in vaporwave and sample-pack scenes, owning the whole run is part of the appeal; a 90% saving makes "catch up on everything" an easy yes for a new fan.
- **Each release is an ad for the discography:** new followers arrive through a release, then see "buy everything for X".
- **Anchoring:** a "save 90%" next to a sum of 100 small prices makes a A$20 to A$40 purchase feel like a bargain.
- **Where it fails:** (a) with few items the discount is small and unconvincing; (b) without future items, the people most likely to buy everything (early fans) gain nothing and may feel punished for buying early; (c) at 90% off, single sales lose their point for anyone who wants more than a few items; (d) on Bandcamp it only works for audio, so it does not cover plugins.

## Stage 2: Apply to Jimmy

Jimmy, from the plan: Australian, hobby scale, cost-sensitive; Max for Live prototypes; many small cheap JUCE plugins, Windows first; free browser versions as funnel; a bank of plugins on a schedule; dislikes subscriptions; maybe a collaborator; light protection. He also makes or researches ambient, dark ambient and slushwave.

| Idea | Fit | Why **[inference]** |
|---|---|---|
| Plugins as Bandcamp releases | **Does not fit** | Releases must be audio; executables and zips probably not allowed as bonus items [6][7][8]. |
| Plugins as Bandcamp "merch" emailed by hand (HIDE, Datashader) | **Does not fit** | Manual delivery breaks the pipeline, sits outside the discography, and depends on Bandcamp tolerating a use its help pages do not describe. |
| Max for Live devices (TAP) on Bandcamp | **Does not fit** | Same as plugins; keep TAP on itch or Isotonik. |
| Sample packs, impulse responses, textures as Bandcamp albums | **Fits well** | Quick to make from the plugins themselves (for example "100 slushwave tape loops made with Drift"); audio is what Bandcamp is for; discography works; GST handled. |
| Jimmy's own music on Bandcamp | **Fits well** | Ambient and slushwave audiences live there [10][42]; followers get release emails; music can promote the tools. |
| Bandcamp discography over the sample packs and music | **Fits** | Once there are 3 or more albums [2]. Discount 50% to 90% as the catalogue grows. |
| Bandcamp subscription for future releases | **Does not fit** | Recurring; Jimmy dislikes subscriptions. |
| itch yearly pass as one growing project | **Fits well (now confirmed)** | itch docs say new files reach buyers automatically [57]. Keep every upload's minimum price at or below the pass price. |
| itch catalogue bundle | **Fits for closed years** | Frozen at purchase [56], which is fine for "Pass 2027" once 2027 ends. |
| Payhip "everything forever" bundle | **Partly fits** | True future access [60], but a second store to run, and Payhip's tax role was not confirmed in the plan. Fallback only. |
| Gumroad "all devices + future" (dnksaus) | **Partly fits** | Works, but US$0.50 flat fee per sale and earlier payout worries (see the monetisation doc). |
| Bandcamp Fridays as release days for packs or music | **Fits** | No Bandcamp share on those days; fans shop then [31][33]. |
| Free browser versions linking to both stores | **Fits** | Plugin page: "get the plugin" (itch); "hear the sample pack / music" (Bandcamp). |

## Stage 3: Investment and trade-offs

All in AUD at about A$1.50 to US$1. All time and revenue figures are **[inference]**, not sourced.

### 3.1 Costs of adding Bandcamp

| Item | Upfront | Ongoing |
|---|---|---|
| Artist account | A$0 [25] | A$0 |
| Bandcamp Pro (batch uploads) | Not needed | Would be about A$15 a month [25] |
| Label account | Not needed (one artist name or a shared brand as an artist) | Would be from about A$30 a month [26] |
| Revenue share | | 15% of digital, 10% after US$5,000 a year [21]; 0% on Bandcamp Fridays [31] |
| Processing | | 5% + US$0.05 under US$8.07; about 2.2% + US$0.30 above [22] |
| PayPal account | A$0 | Payout fee 1% capped at US$1 [22]; PayPal's own currency conversion (not researched) |
| Time to set up account and profile | 2 to 4 hours | |
| Time per sample pack release | 1 to 3 days to make; 30 to 60 minutes to upload, tag and write up | Manual: no upload API was found **[inference, not confirmed]**, so the release pipeline cannot publish to Bandcamp |
| Time per music release | Already part of making music | |

**Fixed cost: A$0.** The real cost is time per release and a manual step outside the pipeline.

### 3.2 Trade-offs against the plan's yearly pass and itch plan

| | itch yearly pass (plan) | Bandcamp discography (this doc) |
|---|---|---|
| What it covers | Plugins and devices | Audio only: sample packs, music |
| Future items | Yes, one project with growing files [57] | No [1][10] |
| Discount control | Seller sets pass price | 5% to 90% off the sum [2] |
| Fees on a A$5 item | Your share (0% to 10%) + US$0.30 + 2.9% [38] | 15% + 5% + US$0.05 [21][22] |
| Fees on a A$30 bundle | About 10% + US$0.30 + 2.9% | About 15% + 2.2% + US$0.30 |
| Tax | VAT in itch Payouts mode; AU GST unconfirmed [38] | Collects and remits AU GST, EU and UK VAT and more [30] |
| Audience | Games-first; some tool buyers | Music fans; some producers; followers emailed each release |
| Automation | butler uploads from CI | Manual |
| Reason to buy early | The pass gets every release that year | None; early buyers pay more than later discography buyers |
| Platform risk | itch is small but stable **[inference]** | Ownership churn, engineering layoffs [34][35] |

### 3.3 Recommended combination

**"Two shelves" [inference]:**

1. **itch.io for software:** each plugin and device sold singly (A$5 to A$12); a yearly pass as one itch project whose files grow through the year; at year end the pass becomes a fixed itch bundle at a discount and a new pass starts. Every upload in the pass project has a minimum price no higher than the pass price, so pass buyers always get it [57].
2. **Bandcamp for audio:** Jimmy's music, plus a small sample pack made with each plugin (loops, one-shots, impulse responses), priced A$2 to A$5 or name-your-price. Turn on the full digital discography once there are three or more albums, starting near 50% off and moving towards 80% to 90% as the catalogue passes about 20 items. Release packs on Bandcamp Fridays where the schedule allows.
3. **Cross-links, not cross-delivery:** each sample pack's description and PDF bonus item links to the plugin that made it, its free browser version and the itch pass. Each itch page links to the matching Bandcamp pack. Do not try to deliver plugins through Bandcamp.
4. **Fallback:** if a true "everything, forever" product is wanted later (for example a lifetime pass), test Payhip bundles [60] before itch or Gumroad.

**Revenue guess [inference, very wide]:** sample packs at A$3 to A$5 might sell a handful to a few dozen copies a year each on Bandcamp, and the discography a few copies a year at first. Bandcamp's value is reach into music fans and the follower email list, not large income.

### 3.4 Risks

| Risk | Likelihood **[inference]** | Mitigation |
|---|---|---|
| Bandcamp rejects or removes non-audio files | Medium, if plugins are attempted | Do not put plugins on Bandcamp; bonus items only PDFs, images, audio |
| Discography buyers feel cheated when new items arrive | Low to medium | Say plainly "includes everything released so far"; keep new packs cheap |
| Discography at 90% undercuts singles | Medium | Start near 50%; raise the discount as the catalogue grows |
| Bandcamp policy or service decline under Songtradr | Medium | Keep the email list and the main software store elsewhere |
| PayPal-only payouts, fees and conversion to AUD | Medium | Confirm whether Stripe payouts are offered to Australia; compare PayPal's conversion rate |
| AI policy | Low | Make demo audio and packs without generative AI; keep notes on how each was made |
| Manual Bandcamp step breaks the release schedule | Medium | Bank packs with plugins; upload the Bandcamp draft when banking |
| Collaborator split on Bandcamp | Medium | Bandcamp splits were not researched; one person holds the account by written agreement, as in the plan |

## 4. Where this changes the current plan

| Plan section | What the plan says | What this research found | Suggested change |
|---|---|---|---|
| Open question 1, and 5. Pricing (yearly pass) | Which store lets a buyer receive later additions? | itch docs: new files on a project reach earlier buyers automatically, except uploads priced above what they paid [57]. itch bundles stay frozen [56]. Payhip bundles also deliver later products [60] | Build the pass as one itch project; keep each upload's minimum at or below the pass price; still run one cheap test purchase |
| 4. Store | itch first; Gumroad, Lemon Squeezy as fallbacks | Bandcamp cannot sell plugins as normal products [6][7][8]; it is strong for audio and collects AU GST [30] | Add Bandcamp as a **second store for audio only** (music, sample packs); do not list plugins there |
| 4. Store fallbacks | Gumroad or Lemon Squeezy | Payhip bundles are the clearest "everything including future" mechanism found [60] | Add Payhip as the fallback for a future-inclusive bundle; check its tax role first |
| 5. Pricing model | Singles, bundles, yearly pass | Bandcamp's discography rewards big catalogues with up to 90% off [2][10][15] but excludes future items | Add a "Bandcamp discography" of sample packs and music, starting near 50% off |
| 7. Website and funnel | Browser version, email list, store | Bandcamp emails followers every release and every discography change [1][25] | Use Bandcamp follows as a second list; link both ways between plugin pages and packs |
| 8. Release cadence | Fixed schedule from a bank | Bandcamp Fridays fall on fixed dates (2026: 8 dates) [31] | Where possible, release sample packs or music on Bandcamp Fridays |
| 9. Release pipeline | Pipeline uploads with butler; store pages by hand | No Bandcamp upload API was found **[inference]** | Add a manifest field for the companion Bandcamp pack and a "Bandcamp draft created" check |
| 10. Costs | About A$0 fixed in Phase 1 | Bandcamp adds A$0 fixed; per-sale 15% plus processing [21][22] | No change to fixed costs |
| 11. Tax | MoR collects overseas VAT and GST | Bandcamp collects GST for Australian buyers and VAT for the EU, UK and others; elsewhere the seller is responsible [30] | Ask the accountant about Bandcamp alongside itch |
| Business plan section 2 (TAP) | TAP format open | Max for Live devices on Bandcamp go through hand-emailed merch only [11][12] | No Bandcamp route for TAP; keep itch or Isotonik |

## Open questions

1. What file types does Bandcamp currently allow as bonus items? Read "What are bonus download items and how do I use them?" once `get.bandcamp.help` is allowed. If `.zip` is allowed, check the terms before adding any software.
2. Does Bandcamp's help page say anything about future releases and the full discography? Read the full article.
3. Is Australia on Bandcamp's Stripe payout list?
4. What does Bandcamp charge as revenue share on artist subscriptions today?
5. Does Bandcamp allow a non-physical "merch" item, as HIDE and Datashader use? Ask Bandcamp support before copying them.
6. How did Peaks of Valleys deliver an Ableton Live pack inside a Bandcamp album?
7. Who exactly is "Signal wave"? Ask the person for a link if it is a specific seller.
8. Geometric Lullaby's discography price in money, and whether its singles are free or name-your-price.
9. Does Payhip act as merchant of record for Australian sellers, and what are its per-sale fees on A$5 items?
10. Does itch's "minimum price higher than what the buyer paid" rule apply per upload, and can a pass project hold uploads with no individual price? Test with a cheap test project.
11. Can Bandcamp split revenue between collaborators?

## Sources

All seen as search-engine extracts on 2026-11-09; none could be fetched. "(secondary)" marks a page not owned by Bandcamp, the store or the seller.

1. Bandcamp help, "How do I enable the purchase of my full digital discography?": https://get.bandcamp.help/en/articles/15263410-how-do-i-enable-the-purchase-of-my-full-digital-discography
2. Bandcamp blog, "Bundle up, winter's a-comin'" (2015): https://blog.bandcamp.com/2015/11/23/bundle-up-winters-a-comin
3. Hypebot, Bandcamp catalogue discount bundles (secondary): https://www.hypebot.com/bandcamp-adds-catalog-discount-bundles-just-in-time-for-christmas
4. FACT, Bandcamp rolls out subscriptions (secondary): https://factmag.com/2015/09/18/bandcamp-rolls-out-subscription-service-to-all-users ; The Fader (secondary): https://www.thefader.com/2015/09/17/bandcamp-opens-up-artist-new-patronage-feature-to-all-users
5. Example artist subscription pages: https://kledmone.bandcamp.com/subscribe ; https://aleksistristanshaw.bandcamp.com/subscribe
6. Bandcamp help, lossless uploads and higher resolution audio: https://get.bandcamp.help/en/articles/15263231-how-and-why-should-i-upload-lossless-files ; https://get.bandcamp.help/en/articles/15263308-can-i-upload-higher-resolution-audio
7. Bandcamp help, "What are bonus download items and how do I use them?": https://get.bandcamp.help/en/articles/15263435-what-are-bonus-download-items-and-how-do-i-use-them
8. Bandcamp blog, bonus download items (2009): https://blog.bandcamp.com/2009/08/24/bonus-download-items/
9. Bandcamp Terms of Use: https://bandcamp.com/terms_of_use
10. Geometric Lullaby on Bandcamp (community, subscription and track pages): https://geometriclullaby.bandcamp.com/community ; https://geometriclullaby.bandcamp.com/track/gravity ; https://geometriclullaby.bandcamp.com/track/data-reef
11. HIDE on Bandcamp, Max for Live merch: https://hide.bandcamp.com/merch/vela-hide-device-x07 ; https://hide.bandcamp.com/merch/m4l-bundle-2-x06-x10 ; https://hide.bandcamp.com/merch/retrig-hide-device-x02 ; https://hide.bandcamp.com/merch/m4l-sound-objects-pack-x11-x13
12. Datashader on Bandcamp, SeqTool#1: https://datashader.bandcamp.com/merch/datashader-seqtool-1-max4live-device
13. maxforlive.com, Datashader device: https://maxforlive.com/library/device.php?id=10290
14. Peaks of Valleys, "Textures (Ableton Live Pack)": https://peaksofvalleys.bandcamp.com/album/textures ; Akihiko Matsumoto Gumroad pack: https://akihiko-matsumoto.gumroad.com/l/ProfundumAbletonProjectPack
15. The Soul Chemist on Bandcamp: https://soulchemistsamples.bandcamp.com/album/dark-vintage-sample-pack-red-drum-rare-horror-and-spooky-samples-for-boom-bap-trap-drill
16. Samples Unlimited on Bandcamp and Gumroad: https://samplesunlimited.bandcamp.com/music ; https://samplesunlimited.gumroad.com
17. Analog Samples on Bandcamp: https://analogsamples.bandcamp.com/merch
18. Bandcamp blog, merch post (2012) with a user request for downloadable merch: https://blog.bandcamp.com/2012/08/01/merch
19. SampleScience on Bandcamp: https://samplescience2.bandcamp.com/music
20. SampleScience site and Gumroad (secondary listings): https://www.samplescience.info/ ; https://samplescience.gumroad.com/l/ydSvm
21. Bandcamp help, "What are Bandcamp's fees?": https://get.bandcamp.help/hc/en-us/articles/23020665520663-What-are-Bandcamp-s-fees ; https://bandcamp.com/artists
22. KVR forum post quoting Bandcamp's processor fee text (forum): https://www.kvraudio.com/forum/viewtopic.php?p=8880875
23. Bandcamp help, payment processor fees for digital sales: https://get.bandcamp.help/en/articles/15263218-how-much-are-payment-processor-fees-for-digital-sales
24. Bandcamp help, transaction fees for artist subscriptions: https://get.bandcamp.help/en/articles/15263255-how-much-are-transaction-fees-for-artist-subscriptions
25. Bandcamp for Artists: https://bandcamp.com/artists
26. Bandcamp for Labels: https://bandcamp.com/labels
27. Bandcamp help, 200 free downloads a month: https://get.bandcamp.help/en/articles/15263450-you-guys-said-i-d-get-200-more-codes-for-free-every-month-well-where-are-they
28. Bandcamp help, getting paid: https://get.bandcamp.help/en/articles/15263080-how-do-i-get-paid-and-how-often ; https://get.bandcamp.help/en/articles/15263052-will-the-move-to-stripe-change-how-often-i-get-paid ; https://bandcamp.com/bandcamp_payments
29. Bandcamp help, Stripe supported countries and unsupported countries: https://get.bandcamp.help/hc/en-us/articles/32854280223511 ; https://get.bandcamp.help/en/articles/15263293-what-if-my-country-is-unsupported-by-stripe
30. Bandcamp help, "What about taxes?" and required tax information: https://get.bandcamp.help/en/articles/15263230-what-about-taxes ; https://get.bandcamp.help/en/articles/15263045-required-tax-information
31. Bandcamp Friday help and 2026 dates: https://get.bandcamp.help/en/articles/15263119-bandcamp-friday-help ; Hypebot (secondary): https://www.hypebot.com/here-are-your-bandcamp-friday-dates-for-all-of-2026/ ; RouteNote (secondary): https://routenote.com/blog/bandcamp-friday-2026-dates/
32. Bandcamp help, merch and pre-orders: https://get.bandcamp.help/en/articles/15263170-how-do-i-sell-merch-on-bandcamp ; https://get.bandcamp.help/en/articles/15263134-how-do-pre-orders-work-with-physical-packages
33. Music Business Worldwide, Bandcamp Fridays totals (secondary, quoting Bandcamp): https://www.musicbusinessworldwide.com/bandcamp-fridays-hit-154m-in-payouts-since-2020-with-19m-paid-in-2025-alone ; https://www.musicbusinessworldwide.com/artists-have-earned-123m-via-bandcamp-fridays-since-2020-and-fans-have-paid-artists-and-their-labels-1-3bn-via-the-platform-to-date ; Benzinga press release (secondary): https://benzinga.com/z/38551917
34. Sale to Songtradr and 2023 layoffs (secondary): https://www.musicradar.com/news/bandcamp-epic-games-songtradr ; https://www.billboard.com/pro/epic-games-sells-bandcamp-songtradr/ ; https://exclaim.ca/music/article/bandcamp_lays_off_50_percent_of_employees_following_songtradr_acquisition_report
35. Hypebot, June 2026 engineer layoffs (secondary): https://www.hypebot.com/new-bandcamp-layoffs-why-artists-are-worried-and-where-to-turn/
36. Bandcamp Clubs (secondary): https://www.musicbusinessworldwide.com/bandcamp-launches-human-curated-bandcamp-clubs-at-13-a-month/ ; help page: https://get.bandcamp.help/hc/en-us/articles/34496035819031
37. Bandcamp blog, "Keeping Bandcamp Human" (13 January 2026): https://blog.bandcamp.com/2026/01/13/keeping-bandcamp-human
38. itch.io docs, payments (fees, Payouts mode VAT): https://itch.io/docs/creators/payments
39. Bandcamp blog, Bandcamp for fans (2013): https://blog.bandcamp.com/2013/01/10/bandcamp-for-fans
40. Bandcamp blog, messaging features (2020): https://blog.bandcamp.com/2020/05/27/new-messaging-features-up
41. Bandcamp Daily, "Build Your Own Beats With These Recent Sample Packs" (19 November 2020): https://daily.bandcamp.com/lists/sample-packs-list
42. Spooky Rusty Substack, signalwave albums (2024, secondary): https://spookyrusty.substack.com/p/top-signalwave-albums-to-liminal
43. EverybodyWiki, Geometric Lullaby (tertiary): https://en.everybodywiki.com/Geometric_Lullaby
44. Utopia District, t e l e p a t h split vinyl (secondary): https://www.utopiadistrict.com/tag/telepath/
45. Discware on Bandcamp: https://discware.bandcamp.com/releases
46. Signalwave searches: https://nightcoverage.bandcamp.com/album/signalwave-is-dead ; https://bandcamp.com/hiscroes
47. Isotonik, Signal / Modulat (via search summary): https://isotonikstudios.com/?p=228682
48. Vapor95, "The mysterious music genre of signalwave" (secondary): https://vapor95.com/blogs/darknet/the-mysterious-music-genre-of-signalwave ; Signals From Nowhere: https://signalsfromnowhere.bandcamp.com/music
49. CT57 on Bandcamp: https://ct57.bandcamp.com/releases ; https://ct57.bandcamp.com/album/local-network
50. dnksaus, All Max For Live Devices Bundle: https://dnksaus.gumroad.com/l/m4l
51. Slim Collective, Everything Bundle: https://slimcollective.gumroad.com/l/EverythingBundle
52. Aurora DSP Everything Bundle (secondary, resellers and KVR): https://www.kvraudio.com/product/everything-bundle-by-aurora-dsp ; https://us.pluginsmasters.com/?p=45502 ; https://thebeatcommunity.com/2025/12/16/vstbuzz-offer-57-off-everything-bundle-by-aurora-dsp/
53. Techivation, Full Access: https://www.techivation.com/full-access/
54. ELV Games on itch.io: https://elvgames.itch.io/universal-sound-effects-bundle ; https://elvgames.itch.io/fantasy-fanfare-music-asset-bundle/devlog/714163/new-24-hours-claim-system
55. Pixelloops on itch.io: https://itch.io/profile/pixelloops
56. itch.io forum posts on bundle contents after purchase (forum): https://itch.io/post/14646466 ; https://itch.io/post/15412673/view-in-topic
57. itch.io docs, how buying works: https://itch.io/docs/creators/how-buying-works ; itch.io, updates to your purchases: https://itch.io/updates/updates-to-your-purchases
58. Indie Hackers, sending updates on Gumroad (secondary): https://www.indiehackers.com/post/need-help-how-do-you-send-product-updates-to-your-customers-on-gumroad-2f0b0f9ab1 ; Gumroad blog, send missed updates: https://gumroad.com/blog/p/send-missed-updates-to-customers-351c9f17-5275-4ff0-b663-9685958927c9
59. Gumroad API docs mirror, product updates not supported (secondary): https://www.mintlify.com/antiwork/gumroad/api/products/update
60. Payhip help, product bundles: https://help.payhip.com/article/212-product-bundles
61. Lemon Squeezy, 2026 update: https://www.lemonsqueezy.com/blog/2026-update
