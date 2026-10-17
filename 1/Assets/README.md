# Assets

Runtime asset paths are declared once in `src/app/Configuration.cpp` and copied beside the executable by CMake.

- `valley-of-the-five-lakes-first-lake-canada-jasper-national-3840x2160-3887.jpg` — application background.
- `Loader Games Pictures/RUST.png` — WIC-compatible PNG converted locally from the supplied `RUST.webp` with the owner-provided `imgforge` tool.
- `Loader Games Pictures/CALL OF DUTY WARZONE.jpg` — Warzone card artwork.
- `Loader Games Pictures/VALORANT.jpg` — Valorant card artwork.
- `Loader Games Pictures/VALVE.png` — Counter-Strike 2 card artwork.
- `icons/` — supplied icon library retained for future customization; Design 1 currently draws its compact controls directly with ImGui for consistent scaling.
- `Loader INSPIRATION/` — visual references only; these files are not copied or used as runtime UI assets.

When replacing an image, retain its configured relative filename or update `Configuration.cpp`. JPEG and PNG are the preferred runtime formats because they are decoded with Windows Imaging Component without another image dependency.
