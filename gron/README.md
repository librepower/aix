# gron - Make JSON greppable

![AIX 7.3](https://img.shields.io/badge/AIX-7.3+-blue)
![Go](https://img.shields.io/badge/Go-1.24-00ADD8)
![License](https://img.shields.io/badge/license-MIT-green)

gron transforms JSON into discrete assignments to make it easier to grep and see the absolute path to each value. It also works in reverse with `gron -u`.

![gron Demo](demo/gron-demo.gif)

## Installation

```bash
# Add LibrePower repo (if not already added)
curl -fsSL https://aix.librepower.org/install.sh | sh

# Install gron
dnf install gron
```

## Quick Start

```bash
# Transform JSON to greppable format
echo '{"name":"alice","age":30}' | gron
# json = {};
# json.age = 30;
# json.name = "alice";

# Grep for specific values
curl -s https://api.example.com/users | gron | grep 'email'

# Convert back to JSON
echo 'json.name = "alice";' | gron -u
# {"name":"alice"}
```

## Common Options

| Option | Description |
|--------|-------------|
| `-u` | Reverse: ungron back to JSON |
| `-c` | Colorize output |
| `-s` | Stream mode for large files |
| `-v` | Show values only |
| `--help` | Show help |

## Examples

```bash
# Explore API responses
curl -s api.github.com/users/octocat | gron | grep 'url'

# Find all array indices
cat data.json | gron | grep '\['

# Modify and reconstruct JSON
cat config.json | gron | sed 's/8080/9090/' | gron -u > new-config.json
```

## Build Notes

- Built with Go 1.24.11 (official)
- 64-bit XCOFF binary
- Size: ~6.5 MB

## Links

- [gron GitHub](https://github.com/tomnomnom/gron)
- [LibrePower](https://librepower.org)

## License

MIT
