# yq - YAML/JSON/XML processor

![AIX 7.3](https://img.shields.io/badge/AIX-7.3+-blue)
![Go](https://img.shields.io/badge/Go-1.24-00ADD8)
![License](https://img.shields.io/badge/license-MIT-green)

yq is a lightweight and portable command-line YAML, JSON, and XML processor. It's like jq but for YAML.

## Installation

```bash
dnf install yq
```

## Quick Start

```bash
# Read a YAML value
yq '.key' file.yaml

# Update a YAML value
yq '.key = "value"' file.yaml

# Convert YAML to JSON
yq -o json file.yaml

# Convert JSON to YAML
yq -P file.json

# Merge YAML files
yq '. *= load("other.yaml")' base.yaml
```

## Common Options

| Option | Description |
|--------|-------------|
| `-o json` | Output as JSON |
| `-o xml` | Output as XML |
| `-P` | Pretty print (YAML) |
| `-i` | Edit file in place |

## Links

- [yq GitHub](https://github.com/mikefarah/yq)
- [LibrePower](https://librepower.org)

## License

MIT
