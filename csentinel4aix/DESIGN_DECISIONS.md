# Design Decisions

This document explains the architectural choices made in C-Sentinel, why they were made, and what trade-offs were considered. It's intended both as documentation and as a demonstration of how a systems architect approaches design.

## Why C?

**Decision**: Use C as the primary language for the system prober.

**Rationale**:
1. **Minimal dependencies**: A C binary can be deployed to virtually any UNIX system without runtime dependencies. No Python interpreter, no Node.js, no container runtime.
2. **Deterministic behaviour**: Memory allocation and deallocation are explicit. There's no garbage collector that might pause at inopportune moments.
3. **Direct system access**: We're reading `/proc`, calling `sysinfo()`, using `stat()`. C is the natural language for this.
4. **Resource efficiency**: The prober should be lightweight enough to run on production systems without impacting performance. A typical run uses <2MB RAM.

**Trade-offs accepted**:
- Longer development time compared to Python
- Manual memory management introduces potential for bugs
- JSON serialization is more tedious without native support

**Mitigations**:
- Strict coding standards (`-Wall -Wextra -Werror`)
- Use of static analysis tools (cppcheck)
- Careful buffer sizing with defined limits

## The Hybrid Architecture

**Decision**: C for the prober, Python for API communication, Flask for dashboard.

**Rationale**:
The problem naturally splits into three distinct domains:
1. **System probing**: Low-level, performance-sensitive, needs direct OS access → C
2. **API communication**: HTTP requests, JSON parsing of responses, error handling → Python
3. **Dashboard**: Web interface, database queries, real-time updates → Flask/PostgreSQL

Forcing all of this into C would mean pulling in `libcurl`, a JSON parsing library, and a web framework—adding complexity and dependencies for the API layer, the opposite of what we want for the lightweight prober.

```
┌─────────────────────────────────────────────────────────────────┐
│                      Web Dashboard                              │
│  Flask + PostgreSQL + Chart.js                                  │
│  (Multi-host, charts, history, security posture, alerts)        │
│  (Multi-user auth, 2FA, API keys, session management)           │
└─────────────────────────────────────────────────────────────────┘
                              ▲
                              │ JSON via HTTP POST
                              │
┌─────────────────────────────────────────────────────────────────┐
│                     C Prober (99KB)                             │
│  /proc parsing, SHA256, network scan, auditd integration        │
│  baseline learning, risk scoring, process chains                │
└─────────────────────────────────────────────────────────────────┘
```

## SHA256 Checksums (v0.3.0)

**Decision**: Implement SHA256 in pure C rather than using OpenSSL or linking to system libraries.

**Rationale**:
1. **Zero dependencies**: The prober remains a self-contained binary
2. **Portability**: Works on any system without library version concerns
3. **Cryptographic integrity**: SHA256 is industry-standard, verifiable by external tools
4. **Audit-ready**: Config file checksums can be compared against known-good values

**Implementation**:
- Based on RFC 6234 / FIPS 180-4
- ~250 lines of C
- Verified against system `sha256sum` for correctness

**Previous approach**: djb2 hash (16 chars) - fast but not cryptographically secure. Replaced with full SHA256 (64 chars) for proper integrity verification.

**Trade-off**: SHA256 is slower than djb2. Acceptable—we're checksumming a handful of config files, not gigabytes of data.

## Auditd Integration (v0.4.0)

**Decision**: Summarise auditd logs rather than forwarding them raw.

**Rationale**:
Security tools like auditd and AIDE answer "what happened?" with precision. C-Sentinel asks "what's weird?" The combination is powerful:

| Without auditd | With auditd |
|----------------|-------------|
| "3 unusual ports" | "3 unusual ports + 3 failed logins + /etc/shadow accessed by python script spawned from web server" |

The key insight: **context is everything**. An LLM seeing "file accessed" is one thing; seeing "file accessed by python3 spawned from apache2" is a security incident.

**Design principles**:
1. **Summarise, don't dump** - Aggregate counts, not raw logs
2. **Process ancestry** - Track the process chain (apache2 → bash → python3)
3. **Baseline deviation** - "3 failures" vs "3 failures (400% above normal)"
4. **Privacy-preserving** - Hash usernames, redact IPs in dashboard

**What we capture**:
- Authentication failures (count + hashed usernames + deviation from baseline)
- Sudo/privilege escalation events
- Sensitive file access with process chain
- Executions from /tmp or /dev/shm (malware indicators)
- SELinux/AppArmor denials

**What we don't capture**:
- Successful logins (noise)
- Raw usernames (privacy)
- Command arguments (could contain secrets)
- File contents (never)

## Locale-Aware Time Windows (v0.5.2)

**Decision**: Use `strftime("%x")` for ausearch timestamps instead of hardcoded date format.

**The Bug**:
Initial implementation used `MM/DD/YYYY` format for ausearch `-ts` parameter. This worked in US locale but failed in UK/EU locales where the system expects `DD/MM/YYYY`.

**The Symptom**:
- Cumulative counts grew unboundedly (773 sudo commands when there should have been ~20)
- ausearch was re-counting all events since midnight instead of since last probe

**The Fix**:
```c
/* Use strftime with %x for locale-aware date format */
char datebuf[32];
strftime(datebuf, sizeof(datebuf), "%x", tm);

snprintf(buf, bufsize, "%s %02d:%02d:%02d",
         datebuf, tm->tm_hour, tm->tm_min, tm->tm_sec);
```

**Lesson**: Never assume date format. Always use locale-aware formatting or explicit timestamps.

## Explainable Risk Scoring (v0.5.1)

**Decision**: Every risk score must come with human-readable explanations of why.

**The Problem**:
A "Risk Score: 25" means nothing without context. Is it auth failures? Suspicious file access? Process anomalies? Users (and LLMs) need to know *why*.

**Implementation**:
```c
typedef struct {
    char reason[RISK_FACTOR_REASON_LEN];
    int weight;  /* Points added to score */
} risk_factor_t;
```

Each factor that contributes to the score is tracked:
```json
"risk_factors": [
  {"reason": "10 auth failures (200% above baseline - high)", "weight": 30},
  {"reason": "Brute force attack pattern detected", "weight": 10},
  {"reason": "2 sensitive file(s) accessed", "weight": 4}
]
```

**Design Principle**: "Prefer features that change conclusions, not features that add more data." (GPT-4 analysis feedback)

## Learning/Calibration Indicator (v0.5.3)

**Decision**: Show users how "confident" the baseline comparison is.

**The Problem**:
A system with only 3 baseline samples cannot reliably detect "above normal" deviations. Users need to know when the system is still learning.

**Implementation**:
| Samples | Status | Confidence |
|---------|--------|------------|
| < 10 | Learning | Low |
| 10-50 | Calibrating | Medium |
| > 50 | *(hidden)* | High |

**UX Choice**: The badge disappears at 50+ samples rather than showing "Ready" forever. Absence of the badge *is* the signal of full confidence.

**Auto-Update Decision**: Each probe automatically updates the baseline rather than requiring explicit `--learn` runs. This ensures the baseline evolves with normal system changes.

## Security Posture Summary (v0.5.5)

**Decision**: Generate a plain English summary of security status—no LLM required.

**The Problem**:
Security dashboards show numbers. Numbers require interpretation. A busy engineer glancing at a dashboard should immediately understand "is this system okay?"

**Implementation**:
Rule-based prose generation:
```javascript
if (riskScore === 0) {
    sentences.push('This system shows no security concerns.');
} else {
    sentences.push(`This system shows ${posture.toLowerCase()} with a risk score of ${riskScore}.`);
}
```

**Example Output**:
> "This system shows no security concerns. Authentication patterns are normal with no failures detected. No privilege escalation activity detected. Overall posture: **HEALTHY**."

**Why Not Use an LLM?**
1. **Latency**: Rule-based is instant; LLM adds seconds
2. **Cost**: Every dashboard load would cost tokens
3. **Determinism**: Same data should always produce same summary
4. **Offline**: Works without API connectivity

## Risk Trend Sparkline (v0.5.6)

**Decision**: Show 24-hour risk score history as a mini chart.

**The Problem**:
A single point-in-time score doesn't answer "Is this getting worse or better?"

**Implementation**:
- 100x30px canvas
- Pure JavaScript (no additional library)
- Color-coded by current risk level
- Uses existing fingerprint data (no new API)

**Visual Design Choices**:
- Filled area under line (easier to see trends)
- Dot on current value (shows latest data point)
- "24h" label (sets context)
- Color matches risk level (green/yellow/orange/red)

**Data Source**: 
```sql
SELECT audit_risk_score FROM fingerprints 
WHERE host_id = ? 
ORDER BY captured_at DESC 
LIMIT 50
```

No new endpoint needed—data already returned by existing host API.

## Email Alerting (v0.5.4)

**Decision**: Send email alerts on high-risk events with per-host cooldown.

**Alert Triggers**:
- Risk score ≥ 16 (high or critical)
- Brute force attack detected
- Any execution from /tmp
- Any execution from /dev/shm

**Cooldown Design**:
```python
_last_alert = {}  # hostname -> datetime

def send_alert_email(hostname, subject, body):
    now = datetime.now()
    last = _last_alert.get(hostname)
    if last and (now - last).total_seconds() < cooldown_minutes * 60:
        return False  # Cooldown active
```

**Why 60-minute cooldown?**
- Prevents alert fatigue during ongoing attacks
- Gives ops time to respond before getting another ping
- Configurable via environment variable

**Email Content**:
- Clear subject with emoji and severity
- Host and timestamp
- List of triggered alerts
- Risk factors with weights
- Direct link to dashboard

## Multi-User Authentication (v0.6.0)

**Decision**: Database-backed multi-user system with role-based access control.

**The Problem**:
Single-password authentication doesn't scale for teams. Different team members need different access levels, and there's no audit trail of who did what.

**Role Design**:
| Role | Capabilities |
|------|-------------|
| **Admin** | Full access: manage users, view audit logs, all operations |
| **Operator** | Acknowledge events, reset counters, view all data |
| **Viewer** | Read-only access to dashboards and host data |

**Why These Three Roles?**
- Maps to common team structures (security admin, ops engineer, developer)
- Simple enough to understand immediately
- Granular enough for meaningful access control

**Implementation**:
```python
def require_role(*roles):
    def decorator(f):
        @wraps(f)
        def decorated(*args, **kwargs):
            user_role = session.get('role', 'viewer')
            if user_role not in roles:
                return jsonify({'error': 'Insufficient permissions'}), 403
            return f(*args, **kwargs)
        return decorated
    return decorator
```

**Backward Compatibility**:
Single-password mode still works. Multi-user activates when users exist in the database. No migration required for existing deployments.

## Two-Factor Authentication (v0.6.0)

**Decision**: Industry-standard TOTP rather than SMS or proprietary 2FA.

**Why TOTP?**
1. **No external dependencies**: Works offline, no SMS gateway needed
2. **User choice**: Works with Google Authenticator, Authy, 1Password, etc.
3. **Industry standard**: RFC 6238, well-understood security properties
4. **Free**: No per-authentication costs

**Implementation**:
```python
import pyotp

# Setup: Generate secret, show QR code
secret = pyotp.random_base32()
totp = pyotp.TOTP(secret)
uri = totp.provisioning_uri(name=username, issuer_name='C-Sentinel')

# Login: Verify code
if totp.verify(user_code, valid_window=1):
    # Allow login
```

**Security Decisions**:
- Secret stored hashed? No—TOTP secrets must be retrievable to generate codes
- Secret stored encrypted? No—adds complexity, key management problem
- Secret stored plain? Yes—protected by database access controls
- Valid window of 1: Allows for slight clock drift (±30 seconds)

**UX Flow**:
1. User enables 2FA in profile
2. QR code displayed (one time only)
3. User scans with authenticator app
4. User enters code to verify setup
5. All future logins require password + TOTP code

## Personal API Keys (v0.6.0)

**Decision**: Per-user API keys that inherit the user's role permissions.

**The Problem**:
A single global API key means:
- No accountability (who used it?)
- All-or-nothing access (can't give read-only to some integrations)
- Rotation affects everyone

**Design**:
```sql
CREATE TABLE user_api_keys (
    id SERIAL PRIMARY KEY,
    user_id INTEGER REFERENCES users(id),
    key_hash VARCHAR(64) NOT NULL,    -- SHA256, never store raw
    key_prefix VARCHAR(12) NOT NULL,  -- "sk_abc123" for identification
    name VARCHAR(100) NOT NULL,       -- "CI/CD Pipeline"
    expires_at TIMESTAMP,             -- Optional expiry
    last_used TIMESTAMP,
    active BOOLEAN DEFAULT TRUE
);
```

**Security Properties**:
- Keys are hashed (SHA256)—raw key shown only once at creation
- Key prefix (`sk_...`) allows identification without exposing full key
- Keys inherit user's role (viewer key can't reset counters)
- Keys can be disabled without deletion (for investigation)
- Per-key expiration (compliance requirement)
- Last-used tracking (detect unused keys)

**Key Format**:
```
sk_29e321e32ed853f696d04cbe0aa9b4146d50858d2771518a
│  └─────────────────────────────────────────────────────┘
│                    48 random hex chars
└── "secret key" prefix
```

The `sk_` prefix makes keys visually identifiable and greppable in logs.

## Session Management (v0.6.0)

**Decision**: Database-backed sessions with device tracking and revocation.

**The Problem**:
Flask's default cookie-based sessions don't allow:
- Seeing who's logged in
- Forcing logout of compromised sessions
- Device/browser tracking
- Session expiry independent of cookie expiry

**Implementation**:
```sql
CREATE TABLE user_sessions (
    id SERIAL PRIMARY KEY,
    user_id INTEGER REFERENCES users(id),
    session_token VARCHAR(64) UNIQUE NOT NULL,
    created_at TIMESTAMP DEFAULT NOW(),
    last_active TIMESTAMP DEFAULT NOW(),
    expires_at TIMESTAMP NOT NULL,
    ip_address VARCHAR(45),
    user_agent VARCHAR(512)
);
```

**Session Token**:
- `secrets.token_hex(32)` = 64 character cryptographically secure token
- Stored in Flask session cookie
- Validated against database on every request

**Why Update last_active?**
- Enables "active now" indicators
- Supports idle timeout policies
- Helps identify abandoned sessions

**Revocation**:
- Individual session revocation (compromised device)
- "Logout all others" (password change, suspected compromise)
- Automatic expiry cleanup (database hygiene)

## Admin Audit Log (v0.6.0)

**Decision**: Log all user actions for compliance and security investigation.

**What We Log**:
| Action | Details Captured |
|--------|-----------------|
| `login` | IP address, 2FA used |
| `login_failed` | Username attempted, IP address |
| `logout` | - |
| `password_changed` | - |
| `create_user` | New username, role |
| `update_user` | Fields changed |
| `delete_user` | Username deleted |
| `2fa_enabled` | - |
| `2fa_disabled` | - |
| `create_api_key` | Key name, prefix |
| `delete_api_key` | Key prefix |
| `acknowledge_events` | Host, event count |
| `reset_audit` | Host |

**What We Don't Log**:
- Passwords (obviously)
- Full API keys
- Dashboard page views (too noisy)
- Read-only API calls

**Retention**: Logs kept indefinitely. Future consideration: configurable retention policy.

## Real Client IP Detection (v0.6.0)

**Decision**: Read `X-Forwarded-For` header when behind a reverse proxy.

**The Problem**:
Behind nginx/Caddy, `request.remote_addr` is always `127.0.0.1`. Login notification emails were showing "New login from 127.0.0.1"—useless for security.

**Implementation**:
```python
def get_client_ip():
    # X-Forwarded-For can contain: client, proxy1, proxy2
    forwarded_for = request.headers.get('X-Forwarded-For')
    if forwarded_for:
        return forwarded_for.split(',')[0].strip()
    
    # Fallback for nginx
    real_ip = request.headers.get('X-Real-IP')
    if real_ip:
        return real_ip.strip()
    
    return request.remote_addr
```

**Security Consideration**:
`X-Forwarded-For` can be spoofed by clients. This is acceptable because:
1. We trust our reverse proxy to set it correctly
2. It's used for logging/display, not authorization
3. The alternative (always showing 127.0.0.1) is worse

## Modern Toast Notifications (v0.6.0)

**Decision**: Replace JavaScript `alert()` with custom toast notifications.

**The Problem**:
`alert()` dialogs are:
- Modal (blocks the entire page)
- Ugly (browser chrome, no styling)
- Disruptive (requires click to dismiss)
- Dated (screams "1990s web development")

**Implementation**:
```javascript
const Toast = {
    show(message, type = 'info', duration = 4000) {
        const toast = document.createElement('div');
        toast.className = `toast-enter ${this.getTypeClasses(type)}`;
        // ... build toast HTML
        this.container.appendChild(toast);
        setTimeout(() => this.dismiss(toast), duration);
    },
    success(msg) { return this.show(msg, 'success'); },
    error(msg) { return this.show(msg, 'error'); },
    // ...
};
```

**Design Choices**:
- Slide in from right (unobtrusive)
- Auto-dismiss after 4 seconds (no action required)
- Manual dismiss available (X button)
- Color-coded by type (green/red/yellow/blue)
- Stacks multiple toasts vertically

**Confirm Dialogs**:
Also replaced `confirm()` with styled modal overlay for destructive actions like session revocation and user deletion.

## User Email Notifications (v0.6.0)

**Decision**: Email users on security-relevant account events.

**Events That Trigger Email**:
| Event | Why Notify |
|-------|-----------|
| New login | Detect unauthorized access |
| Password changed | Confirm intentional change |
| Account created | Welcome + credentials |
| 2FA enabled/disabled | Security status change |
| Role changed | Access level change |

**Email Content Philosophy**:
- Clear subject with `[C-Sentinel]` prefix
- State what happened
- State when it happened
- State from where (IP, browser)
- Provide action if unauthorized
- Link to dashboard

**Example**:
```
Subject: [C-Sentinel] New login from 81.79.231.105

Hello william,

A new login to your C-Sentinel account was detected:

  Time: 03 January 2026 at 23:27
  IP Address: 81.79.231.105
  Browser: Chrome

If this was you, no action is needed.

If you did NOT log in, someone may have access to your account.
Please change your password immediately and contact your administrator.
```

## Event History with Acknowledgement (v0.5.0)

**Decision**: Store discrete security events, not just aggregate counts.

**The Problem**:
Cumulative counts tell you "100 sudo commands happened." They don't tell you *when*, and they don't let you mark events as "investigated."

**Implementation**:
```sql
CREATE TABLE audit_events (
    id SERIAL PRIMARY KEY,
    host_id INTEGER REFERENCES hosts(id),
    event_type VARCHAR(50),  -- 'auth_failure', 'sudo', etc.
    count INTEGER,
    details JSONB,
    acknowledged BOOLEAN DEFAULT FALSE
);
```

**Acknowledgement Design**:
- Events can be individually or bulk acknowledged
- Acknowledged events hidden by default but retrievable
- "Reset" clears cumulative totals and acknowledges all events
- Audit trail preserved (events not deleted)

## Systemd Service Design (v0.3.0)

**Decision**: Provide a production-ready systemd service unit with security hardening.

**Rationale**:
Watch mode (`--watch`) is useful for development, but production deployments need:
- Automatic restart on failure
- Clean shutdown handling
- Resource limits
- Security isolation
- Logging to journald

**Security hardening applied**:
```ini
NoNewPrivileges=yes      # Can't gain privileges via setuid
ProtectSystem=strict     # / is read-only
ProtectHome=read-only    # /home is read-only
PrivateTmp=yes           # Isolated /tmp
ReadWritePaths=/var/lib/sentinel  # Only place it can write
```

**Dedicated user**: The service runs as `sentinel` user with no home directory and `/usr/sbin/nologin` shell.

**Exit code handling**:
```ini
SuccessExitStatus=0 1 2
```

This tells systemd that exit codes 0 (OK), 1 (WARNING), and 2 (CRITICAL) are all valid results—only exit code 3 (ERROR) triggers a restart.

## Web Dashboard Design (v0.3.0)

**Decision**: Flask + PostgreSQL + Chart.js for multi-host monitoring.

**Rationale**:
The CLI prober is excellent for single-host diagnostics, but teams need:
- At-a-glance view of multiple hosts
- Historical trending (memory, load over time)
- Alerting when hosts go stale or critical
- Drill-down to individual host details

**Why Flask?**
- Lightweight, no magic
- Easy to deploy (gunicorn + nginx)
- Sufficient for internal tooling
- Familiarity with Python ecosystem

**Why PostgreSQL?**
- Already running on target system
- JSONB type for flexible fingerprint storage
- Excellent for time-series queries
- Reliable, battle-tested

**Why not Prometheus/Grafana?**
- Adds operational complexity
- Prometheus is metrics-focused, not fingerprint-focused
- We want semantic data (process lists, config checksums), not just numbers
- Building our own gives full control over the data model

**Data model**:
```sql
hosts (id, hostname, first_seen, last_seen)
fingerprints (id, host_id, captured_at, data JSONB, exit_code, ...)
audit_events (id, host_id, event_type, count, details, acknowledged)
users (id, username, email, password_hash, role, totp_secret, ...)
user_sessions (id, user_id, session_token, ip_address, ...)
user_api_keys (id, user_id, key_hash, name, expires_at, ...)
user_audit_log (id, user_id, action, details, ip_address, ...)
```

JSONB allows storing the full fingerprint while extracting key fields for efficient querying.

## Fingerprint Design

**Decision**: Capture a "fingerprint" of system state rather than streaming metrics.

**Rationale**:
Traditional monitoring tools (Prometheus, Datadog, Dynatrace) excel at time-series metrics. They answer "what is the CPU doing right now?" C-Sentinel aims to answer a different question: "What is the overall state of this system, and what might be wrong?"

A fingerprint is:
- A point-in-time snapshot
- Comprehensive (system info, processes, configs, network, security)
- Structured for semantic analysis
- Suitable for diff-comparison between systems

This enables use cases that streaming metrics cannot address:
- "Compare these two 'identical' non-prod environments"
- "What's changed since last week?"
- "Why is prod-1 behaving differently from prod-2?"

## "Notable" Process Selection

**Decision**: Don't include all processes in the JSON output—filter to interesting ones.

A system might have 500+ processes. Sending all of them to an LLM is:
- Wasteful of tokens/cost
- Noisy (most processes are uninteresting)
- Potentially revealing (process names can leak information)

**Selection criteria**:
| Flag | Condition | Rationale |
|------|-----------|-----------|
| `zombie` | State = 'Z' | Always a problem |
| `high_fd_count` | >100 open FDs | Potential leak |
| `potentially_stuck` | State 'D' for >5 min | I/O issues |
| `very_long_running` | Running >30 days | Should probably be restarted |
| `high_memory` | RSS >1GB | Resource hog |

## JSON Serialization Strategy

**Decision**: Hand-rolled JSON serialization rather than using cJSON or similar.

**Rationale**:
- One fewer dependency
- Our output schema is fixed and simple
- Full control over formatting (pretty-printed for readability)
- Educational value (demonstrates string handling in C)

**Trade-offs**:
- More code to maintain
- Potential for subtle escaping bugs

**Mitigation**: The `buf_append_json_string()` function handles all escaping centrally.

## Security Considerations

### Input validation
All paths and strings from external sources are length-limited. Buffer overflows are prevented by:
- Using `snprintf()` instead of `sprintf()`
- Using custom `safe_strcpy()` instead of `strcpy()`
- Defining `MAX_*` constants for all arrays

### Privilege model
The prober reads from `/proc` and specified config files. It requires:
- Read access to `/proc` (standard for all users)
- Read access to config files (may require appropriate group membership)
- Read access to audit logs (requires root or audit group)
- **No write access anywhere** (except baseline/config in its own directory)

### Dashboard security
- API key required for ingestion (global or per-user)
- Password authentication for web access
- Optional two-factor authentication (TOTP)
- Role-based access control
- Session management with revocation
- Audit logging of all user actions
- Sensitive data protected by role

### Sanitization
Before sending to LLM:
- IP addresses are redacted to `[REDACTED-IP]`
- Home directory paths are redacted
- Known secret environment variables are redacted
- Visible placeholders so analysts know data was present

## Lessons from 30 Years of UNIX

This tool embeds certain assumptions from experience:

1. **Zombies are never okay**: Some monitoring tools ignore them. We don't.
2. **Long-running processes deserve scrutiny**: A process that's been running for 30 days may have accumulated state, leaked memory, or holding stale connections.
3. **Config drift is insidious**: Two "identical" servers with one different sysctl setting have caused countless production incidents.
4. **World-writable configs are never intentional**: This is always either a mistake or a compromise.
5. **File descriptor leaks are slow killers**: The system runs fine until suddenly it doesn't.
6. **New listening ports are suspicious**: If a port wasn't open yesterday, ask why it's open today.
7. **Missing services are emergencies**: If a port was supposed to be listening and isn't, something failed.
8. **Context is everything**: "File accessed" is one thing; "file accessed by script spawned from web server" is an incident.
9. **Baselines should be learned, not configured**: What's "normal" for one system isn't normal for another.
10. **The simple approach often wins**: 99KB of C beats 100MB of dependencies.
11. **Explainability beats precision**: A score users understand is better than a score that's technically more accurate but opaque.
12. **Alert fatigue is real**: One actionable alert beats ten noisy ones.
13. **Authentication is table stakes**: Multi-user with 2FA should be the default, not a premium feature.
14. **Audit everything**: When something goes wrong, you need to know who did what and when.

These aren't just heuristics—they're battle scars.

---

*Last updated: January 2026*
*Author: William Murray*
