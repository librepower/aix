# Real-World Sample: First Production Test

This document shows C-Sentinel's first real-world test on a Debian 12 validator node. Within minutes of deployment, it identified genuine anomalies - demonstrating the value of semantic observability.

## The Test Environment

- **Host**: Debian 12 cloud VM (validator node)
- **Uptime**: 13.2 days
- **RAM**: 3.83 GB
- **Kernel**: Linux 6.1.0-41-cloud-amd64

## Quick Analysis Output

```
C-Sentinel Quick Analysis
========================
Hostname: axioma-validator
Uptime: 13.2 days
Load: 0.17 0.50 0.58
Processes: 113 total

Potential Issues:
  Zombie processes: 0
  High FD processes: 0
  Long-running (>7d): 97
  Config permission issues: 0
```

**Initial assessment**: System appears healthy. 97 long-running processes is normal for a 13-day uptime on a validator node.

## AI-Powered Analysis (Claude)

When we sent the full fingerprint to Claude for semantic analysis:

```
## Summary
This axioma-validator system appears healthy at first glance - low load, 
good memory availability, no zombies or stuck processes. However, there's 
a subtle but significant security concern with recent passwd file 
modifications and an unusual ownership issue with resolv.conf that 
warrants investigation.

## Concerns

**HIGH**: Recent passwd modification during business hours
- `/etc/passwd` was modified TODAY at 14:45:01Z, just 40 minutes before this probe
- This is unusual for a stable validator system that should have minimal 
  user account changes
- Could indicate unauthorized user creation, privilege escalation, or compromise

**MEDIUM**: Abnormal resolv.conf ownership
- `/etc/resolv.conf` owned by UID 996 instead of root (UID 0)
- UID 996 typically belongs to systemd-resolve or similar service account
- While not immediately dangerous, this deviates from standard security practices

**LOW**: Load average trending upward
- 1-minute: 0.01, 5-minute: 0.1, 15-minute: 0.32
- Suggests recent activity increase or potential background process buildup

## Recommendations

1. **Investigate passwd changes immediately**:
   journalctl -u systemd-logind --since "14:40" --until "14:50"
   last -f /var/log/wtmp | head -20
   awk -F: '$3 >= 1000 {print $1, $3, $4, $5}' /etc/passwd

2. **Verify resolv.conf ownership**:
   systemctl status systemd-resolved
   ls -la /etc/resolv.conf

3. **Monitor for additional changes**:
   auditctl -w /etc/passwd -p wa -k user_changes
   auditctl -w /etc/shadow -p wa -k shadow_changes
```

## Policy Engine Report

All suggested commands passed validation:

```
## 🛡️ C-Sentinel Policy Engine Report

### ✅ Safe Commands
8 command(s) passed policy validation.
```

## The Investigation

Following the AI's recommendation, we checked:

```bash
$ cat /etc/passwd | tail -5
polkitd:x:995:995:polkit:/nonexistent:/usr/sbin/nologin
william:x:1000:1003::/home/william:/bin/bash
_chrony:x:105:111:Chrony daemon,,,:/var/lib/chrony:/usr/sbin/nologin
postgres:x:106:113:PostgreSQL administrator,,,:/var/lib/postgresql:/bin/bash
ollama:x:999:994::/usr/share/ollama:/bin/false
```

**Mystery solved!** The `/etc/passwd` modification was from installing Ollama (a local LLM runner) 40 minutes earlier. The new `ollama` user was created by the installer.

## Why This Matters

This test demonstrates the core value proposition of C-Sentinel:

| Layer | Role | Result |
|-------|------|--------|
| **C Prober** | Captured accurate file modification timestamp | ✅ Correct |
| **AI Analysis** | Identified it as anomalous for a stable system | ✅ Correct |
| **Policy Engine** | Validated all suggested commands as safe | ✅ Correct |
| **Human Operator** | Provided context: "I just installed Ollama" | ✅ Case closed |

**Traditional monitoring** would have shown green lights across the board - CPU OK, memory OK, disk OK. It would have completely missed the `/etc/passwd` modification because it doesn't understand *context*.

**C-Sentinel** surfaces the anomaly and lets the AI reason about it. The human makes the final determination with full context.

## Bug Found and Fixed

During this same test, we discovered a bug in the FD counting logic:

**Problem**: Kernel threads return `-1` for FD count (can't read `/proc/[pid]/fd`). Since `open_fd_count` is `uint32_t`, this wrapped to 4,294,967,295 - triggering false "high FD" alerts for 107 processes.

**Fix**: Added bounds checking: `p->open_fd_count > 100 && p->open_fd_count < 100000`

**Lesson**: Always test on real systems, not just containers. This bug would never have appeared in a minimal test environment.

## Comparison: Local vs Cloud LLM

We tested with both a local LLM (TinyLlama 1.1B via Ollama) and Claude Sonnet:

| Aspect | TinyLlama (Local) | Claude (Cloud) |
|--------|-------------------|----------------|
| Response time | ~45 seconds | ~3 seconds |
| Passwd detection | ❌ Missed entirely | ✅ Flagged as HIGH |
| resolv.conf ownership | ❌ Hallucinated details | ✅ Correctly identified |
| Recommendations | Generic, some hallucinated | Specific, actionable |
| Cost | Free | ~$0.002 per analysis |

**Conclusion**: For production use, cloud LLMs provide significantly better analysis. Local LLMs are useful for air-gapped environments or cost-sensitive development, but expect reduced accuracy.

---

*This sample was captured during actual testing on January 1, 2026. System details have been lightly sanitized but findings are genuine.*
