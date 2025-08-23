# codal-core-addon

`codal-core-addon` is a small collection of extra headers, helpers, and other odds‑and‑ends for use with [codal-core](https://github.com/lancaster-university/codal-core).  
It’s not a standalone library — think of it as a box of spare parts you might need when building CODAL projects.

## Requirements

To make use of anything here, you’ll need:

- **[codal-core](https://github.com/lancaster-university/codal-core)**  
  The main CODAL runtime library.
- **Target-specific CODAL support**  
  Board definitions, drivers, and configuration files for your hardware target  
  (e.g., micro:bit, custom boards, etc.).

## Installation

1. Clone or add `codal-core-addon` as a submodule alongside your `codal-core` and target repository.
2. Make sure your build system includes:
   - `codal-core`
   - Your target-specific sources
   - This add-on
3. Pull in the bits you need from `codal-core-addon` in your code.

## Notes

- This repo is intentionally **not** named `codal-core` to avoid confusion.
- Contents are a mix of potentially useful extras — you may not need all of them.
- Will **not** compile or function on its own — it must be used with `codal-core` and a valid CODAL target.
