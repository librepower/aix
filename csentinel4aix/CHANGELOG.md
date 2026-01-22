# Changelog

All notable changes to C-Sentinel will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [0.6.0-2] - 2026-01-22

### Added
- **Audit trail rotation** - Optional script to prevent `/audit/trail` from filling disk
  - `setup-audit-rotation.sh` - Interactive setup (requires user confirmation)
  - `audit-rotate.sh` - Rotates when >100MB, keeps 4 backups, runs daily at 3am
- Safe for existing installations - does not modify audit configuration

## [0.6.0] - 2026-01-22

### Added
- **AIX native audit integration** - Full support for AIX audit subsystem via `-a` flag
  - Parses `/audit/bin1`, `/audit/bin2`, and `/audit/trail` using `auditpr`
  - Detects authentication events (success/failure), privilege escalation (su/sudo)
  - Brute force detection (5+ consecutive auth failures)
  - Risk scoring and security posture assessment
- **Full file integrity mode** (`-F` flag) - PowerSC RTC-comparable monitoring
  - 171 critical AIX files across 20 security categories
  - Based on CIS AIX Benchmark, DoD STIG, and PowerSC RTC file lists
- **Expanded default configs for AIX** - 12 key files monitored by default
- **sentinel-push** now includes `-a` flag by default

### Changed
- MAX_CONFIG_FILES increased from 64 to 256 for full mode support

### Fixed
- JSON serialization for AIX kernel processes with special characters

## [0.5.8] - 2026-01-03

### Added
- Dashboard authentication with session management
- Event History tab with filtering
- Email alerts for high-risk events
- Risk trend sparkline

## [0.4.0] - 2026-01-02

### Added
- Auditd integration (Linux)
- Brute force detection
- Process chains tracking

## [0.3.0] - 2026-01-01

### Added
- Web Dashboard
- SHA256 checksums
- Baseline learning
- Network probe

## [0.1.0] - 2025-12-25

### Added
- Initial release
