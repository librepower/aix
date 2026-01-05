# Example Fingerprints

This directory contains pre-captured system fingerprints for testing and demonstration.

## Files

### `healthy_webserver.json`
A well-maintained production web server with:
- Reasonable uptime (45 days)
- Low load average
- Healthy memory usage (42%)
- No zombie processes or stuck jobs
- Proper file permissions

**Use case**: Baseline comparison, testing the "no issues" path.

### `troubled_appserver.json`
An application server showing multiple problems:
- **Memory pressure**: 94.4% used, Java process consuming 8GB
- **Zombie processes**: 2 defunct processes (parent not reaping children)
- **High FD count**: Python process with 312 open file descriptors
- **Stuck process**: MySQL in uninterruptible sleep (state 'D')
- **World-writable config**: `/etc/security/limits.conf` is chmod 777
- **Log explosion**: 15GB debug log file

**Use case**: Testing detection of common production issues.

### `drifted_webserver.json`
A "twin" of `healthy_webserver.json` that has drifted:
- Different kernel version (5.15.0-89 vs 5.15.0-91)
- Higher load (1.82 vs 0.42)
- Different nginx config checksum
- Recently restarted (12 days vs 45 days uptime)
- Extra sysctl.conf that doesn't exist on healthy server

**Use case**: Testing drift detection with `sentinel-diff`.

## Testing Commands

```bash
# Analyze the troubled server
./sentinel_analyze.py --json < examples/troubled_appserver.json

# Compare the "identical" web servers
./bin/sentinel-diff examples/healthy_webserver.json examples/drifted_webserver.json

# Test with local LLM (no API costs)
./sentinel_analyze.py --local < examples/troubled_appserver.json
```

## Creating Your Own Examples

Capture real fingerprints from your systems:

```bash
# On each server
./bin/sentinel > server_name_$(date +%Y%m%d).json

# Then compare
./bin/sentinel-diff server_a.json server_b.json
```

## What Makes a Good Test Case

1. **Realistic**: Based on actual production issues
2. **Specific**: Includes concrete values (PIDs, paths, sizes)
3. **Documented**: Explains what the issue is and why it matters
4. **Varied**: Covers different types of problems

The examples here are based on real-world scenarios from 30 years of UNIX administration.
