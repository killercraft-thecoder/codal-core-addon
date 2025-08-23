# codal-core-addon

`codal-core-addon` provides additional header files and utilities for use with [codal-core](https://github.com/lancaster-university/codal-core).  
It is **not** a standalone library — it extends `codal-core` and requires target-specific components to function.

## Requirements

To use this repository, you must have:

- **[codal-core](https://github.com/lancaster-university/codal-core)**  
  The main CODAL runtime library.
- **Target-specific CODAL support**  
  Board definitions, drivers, and configuration files for your hardware target  
  (e.g., micro:bit, custom boards, etc.).

## Installation

1. Clone or add `codal-core-addon` as a submodule alongside your `codal-core` and target repository.
2. Ensure your build system includes:
   - `codal-core`
   - Your target-specific sources
   - This add-on
3. Include the headers from `codal-core-addon` where needed.

## Notes

- This repository is intentionally **not** named `codal-core` to avoid confusion.
- It will **not** compile or function on its own — it must be used with `codal-core` and a valid CODAL target.
