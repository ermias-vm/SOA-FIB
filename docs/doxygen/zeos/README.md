# ZeOS Doxygen Documentation

This directory contains the Doxygen configuration for generating comprehensive documentation for the ZeOS educational operating system.

## Prerequisites

- **Doxygen**: Install Doxygen to generate the documentation.
  - On Ubuntu/Debian: `sudo apt install doxygen`
  - On macOS: `brew install doxygen`
  - On other systems, download from [doxygen.nl](https://www.doxygen.nl/download.html)

- **GraphViz** (optional, for diagrams): `sudo apt install graphviz` or equivalent.

## Generating Documentation

To generate the documentation automatically, run the following command from the `zeos/` directory:

```bash
make doxygen
```

This will:
1. Change to the `docs/doxygen/zeos/` directory
2. Run `doxygen Doxyfile`
3. Generate HTML and LaTeX documentation in `docs/doxygen/zeos/output/`

## Viewing Documentation

After generation, open `docs/doxygen/zeos/output/html/index.html` in your web browser to view the HTML documentation.

## Manual Generation

If you prefer to generate manually:

```bash
cd docs/doxygen/zeos
doxygen Doxyfile
```

## Configuration

The `Doxyfile` is configured to:
- Document C and header files from `../../zeos` and `../../zeos/include`
- Generate HTML and LaTeX output
- Include class and collaboration diagrams (if GraphViz is installed)
- Extract all documented and undocumented entities
- Use professional styling and layout

## Customization

Edit `Doxyfile` to customize the documentation generation. Common options:
- `PROJECT_NAME`: Change the project name
- `INPUT`: Add or modify input directories
- `GENERATE_LATEX`: Set to NO if you don't need PDF output
- `HAVE_DOT`: Set to NO if GraphViz is not installed

## Troubleshooting

- If diagrams don't appear, ensure GraphViz is installed.
- For missing documentation, check that functions and classes have proper Doxygen comments (e.g., `/** @brief Description */`).
- If paths are incorrect, verify the relative paths in `INPUT` and `STRIP_FROM_PATH`.