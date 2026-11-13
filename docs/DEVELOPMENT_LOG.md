# Development log

## 2026-11-14 — Procedural seabed features

- Added seeded, continuous mountain relief and winding canyon cuts to the global height field so terrain stays seamless across streamed chunk borders.
- Added rare boulder-slide clusters, placed along steep terrain and generated deterministically with their chunk.
- Added a very rare broken-stone gateway ruin, rendered through the existing instanced geometry path and kept out of the opening reef area.
- Updated the project guide to describe the new world features and their repeatable seeded placement.

## 2026-11-14 — Ocean landscape and wreck exploration

- Reworked bathymetry into broad sediment-like plains with rolling relief, high seamounts, deep winding trenches and large crater basins with pronounced floors and raised rims.
- Restricted reef coral to separated seeded habitat patches. Added distinct kelp and low seagrass beds, sparse rocky patches, and small clusters of cans, bottles, crates, tires and fishing net debris.
- Replaced isolated ruin-only placement with widely spaced regional wreck sites. Added procedural cargo ship, liner, carrier, aircraft, submarine, fishing boat and stone ruin models.
- Made wrecks individually pickable and added a seafloor guide that distinguishes fictional game placement from sourced historical site, sinking, discovery and depth information.
- Placed the guaranteed liner in the opening dive view, kept other wreck sites widely spaced, and added buttons to open the guide's source pages in the default browser.
- Updated controls and documentation to describe the broader ocean ecosystem and new interaction.
