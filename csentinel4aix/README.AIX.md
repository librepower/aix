# C-Sentinel para AIX 7.1/7.2/7.3

## Guía de Compilación e Instalación

### Requisitos Previos

- AIX 7.1, 7.2 o 7.3
- GCC (disponible en `/opt/freeware/bin/gcc` via AIX Toolbox)
- Librerías del sistema: `libperfstat`, `libodm`, `libcfg`

### Compilación

```bash
# Clonar el repositorio
cd /tmp
git clone https://github.com/librepower/c-sentinel4aix c-sentinel
cd c-sentinel

# Compilar usando el Makefile específico para AIX
/opt/freeware/bin/make -f Makefile.aix

# Verificar binarios
ls -la bin/
```

### Instalación

```bash
# Instalar en /opt/freeware/bin (requiere root)
/opt/freeware/bin/make -f Makefile.aix install

# O copiar manualmente
cp bin/sentinel /opt/freeware/bin/
cp bin/sentinel-diff /opt/freeware/bin/
```

## Uso

### Comandos Básicos

```bash
# Análisis rápido del sistema
sentinel -q

# Análisis completo con salida JSON
sentinel -j > fingerprint.json

# Incluir monitoreo de red
sentinel -n -q

# Modo watch (monitoreo continuo)
sentinel -w -i 60

# Ver ayuda
sentinel -h
```

### Opciones Disponibles

**IMPORTANTE:** En AIX, usar **opciones cortas** (`-x`) en lugar de largas (`--xxx`)

| Opción | Descripción |
|--------|-------------|
| `-h` | Mostrar ayuda |
| `-q` | Modo rápido (solo resumen) |
| `-v` | Verbose (incluir todos los procesos) |
| `-j` | Salida JSON |
| `-n` | Incluir información de red |
| `-a` | Incluir eventos de auditoría AIX |
| `-F` | Modo completo de integridad (~171 archivos críticos) |
| `-w` | Modo watch (monitoreo continuo) |
| `-i SEC` | Intervalo en segundos (default: 60) |
| `-b` | Comparar contra baseline |
| `-l` | Aprender baseline actual |
| `-c` | Mostrar configuración |

## Funcionalidades

### ✅ Completamente Funcionales

1. **Monitoreo de Sistema**
   - Uptime, load average, uso de memoria
   - Información del kernel y hostname

2. **Monitoreo de Procesos**
   - 100% de los procesos detectados
   - Estado, memoria, threads, edad
   - Detección de zombies y procesos stuck
   - Análisis de cadenas de procesos (process chains)

3. **Monitoreo de Archivos de Configuración**
   - Checksums SHA256
   - Permisos y ownership
   - Detección de drift

4. **Baseline Learning**
   - Aprendizaje de estado normal del sistema
   - Detección de desviaciones

5. **Auditoría AIX** (nuevo en v0.6.0)
   - ✅ Integración nativa con subsistema de auditoría AIX
   - ✅ Detección de autenticación (éxitos/fallos)
   - ✅ Detección de escalada de privilegios (su/sudo)
   - ✅ Detección de fuerza bruta (5+ fallos consecutivos)
   - ✅ Puntuación de riesgo y evaluación de postura de seguridad

6. **Integridad de Archivos Completa** (nuevo en v0.6.0)
   - ✅ 171 archivos críticos AIX con flag `-F`
   - ✅ Comparable a IBM PowerSC RTC
   - ✅ Basado en CIS AIX Benchmark, DoD STIG
   - ✅ 20 categorías: autenticación, audit, red, SSH, boot, cron, etc.

### ⚠️ Limitaciones Conocidas

1. **Network Monitoring**
   - ✅ Detecta listeners y conexiones
   - ✅ **Obtiene PIDs y nombres de proceso para 70+ puertos conocidos**
   - ✅ Soporta: SSH, bases de datos (PostgreSQL, MySQL, Oracle, DB2), IBM middleware, SAP, etc.
   - ⚠️ Puertos no estándar muestran `[unknown]` (comportamiento esperado)

2. **Auditoría AIX**
   - ✅ Soportado completamente
   - Requiere que auditoría esté habilitada: `/usr/sbin/audit start`

3. **Opciones Largas**
   - ❌ Opciones `--xxx` no funcionan
   - Usar opciones cortas `-x`

## Ejemplos de Uso

### Ejemplo 1: Análisis Rápido

```bash
$ sentinel -q
C-Sentinel Quick Analysis
========================
Hostname: LP_AIX734
Uptime: 5.2 days
Load: 1.24 0.97 0.97
Memory: 6.5% used
Processes: 407 total

Potential Issues:
  Zombie processes: 0
  High FD processes: 0
  Long-running (>7d): 2
  Config permission issues: 0
```

### Ejemplo 2: Análisis con Red

```bash
$ sentinel -n -q
C-Sentinel Quick Analysis
========================
Hostname: LP_AIX734
Uptime: 5.7 days
Load: 0.69 0.70 0.86
Memory: 6.6% used
Processes: 418 total

Potential Issues:
  Zombie processes: 0
  High FD processes: 0
  Long-running (>7d): 2
  Config permission issues: 0

Network:
  Listening ports: 13
  Established connections: 3
  Unusual ports: 6 ⚠

  Listeners:
    *:22 (tcp6) - sshd              ✅ PID detectado
    *:22 (tcp4) - sshd              ✅ PID detectado
    *:25 (tcp4) - sendmail          ✅ PID detectado
    *:657 (tcp) - rmcd              ✅ PID detectado
    *:1334 (tcp4) - writesrv        ✅ PID detectado
    *:5000 (tcp4) - python3.12      ✅ PID detectado
    ::1:5432 (tcp6) - postgres_64   ✅ PID detectado
    *:32768 (tcp) - aso             ✅ PID detectado
```

### Ejemplo 3: Salida JSON

```bash
$ sentinel -j -q > fingerprint.json
$ cat fingerprint.json
{
  "sentinel_version": "0.6.0",
  "probe_time": "2026-01-05T02:29:01Z",
  "system": {
    "hostname": "LP_AIX734",
    "kernel": "AIX 3",
    "uptime_days": 5.22,
    "load_average": [0.57, 0.78, 0.91],
    "memory_total_gb": 256.00,
    "memory_free_gb": 239.37,
    "memory_used_percent": 6.5
  },
  "process_summary": {
    "total_count": 408,
    "zombie_count": 0,
    "high_fd_count": 0,
    "stuck_count": 0
  }
}
```

### Ejemplo 4: Baseline Learning

```bash
# Primera vez: aprender estado normal
$ sentinel -l
Baseline learned from current system state
Saved to: ~/.sentinel/baseline.dat

# Comparar contra baseline en futuras ejecuciones
$ sentinel -b -q
C-Sentinel Quick Analysis
========================
...
Baseline Comparison:
  New listeners: 1
  Missing listeners: 0
  Config changes: 0
  Process count anomaly: no
```

### Ejemplo 5: Auditoría de Seguridad AIX

```bash
# Habilitar auditoría (si no está activa)
$ /usr/sbin/audit start

# Análisis con eventos de seguridad
$ sentinel -q -n -a
C-Sentinel Quick Analysis
========================
Hostname: LP_AIX734
Uptime: 22.7 days
...

Security (AIX audit):
  Auth successes: 45
  Auth failures: 3
  su/sudo events: 12
  Risk score: 15 (low)
```

### Ejemplo 6: Integridad Completa de Archivos

```bash
# Verificar ~171 archivos críticos (comparable a PowerSC RTC)
$ sentinel -F -q
Full AIX file integrity mode: checking 171 critical files
C-Sentinel Quick Analysis
========================
Hostname: LP_AIX734
...

# JSON con todos los checksums
$ sentinel -F -j > full-integrity.json

# Categorías monitoreadas:
# - /etc/security/* (autenticación, usuarios, grupos)
# - /etc/ssh/* (configuración SSH)
# - /etc/security/audit/* (configuración de auditoría)
# - /usr/bin/passwd, /usr/bin/su, etc. (binarios SUID)
# - /unix, /usr/lib/boot/* (kernel)
# - Y 15 categorías más...
```

## Troubleshooting

### Error: "command not found"
```bash
# Verificar que GCC esté instalado
which /opt/freeware/bin/gcc

# Si no está, instalar AIX Toolbox for Linux Applications
# https://www.ibm.com/support/pages/aix-toolbox-linux-applications
```

### Error de compilación: "libperfstat not found"
```bash
# Verificar que las librerías estén instaladas
ls -la /usr/lib/libperfstat.a

# Si no existe, instalar bos.perf.libperfstat
installp -a -d /dev/cd0 bos.perf.libperfstat
```

### Performance lento
```bash
# El análisis de red puede ser lento en sistemas con muchos procesos
# Usar sin -n para análisis más rápido
sentinel -q

# O aumentar prioridad
nice -n -10 sentinel -q
```

## Dashboard Web (Opcional)

El dashboard web también funciona completamente en AIX con las dependencias correctas.

### Prerequisitos del Dashboard

```bash
# Instalar PostgreSQL y dependencias Python
/opt/freeware/bin/dnf install -y postgresql-server postgresql postgresql-devel python3-pip python3-devel

# Verificar instalación
/opt/freeware/bin/psql --version
/opt/freeware/bin/pip3 --version
```

### Configuración de PostgreSQL

```bash
# Crear directorio de datos
mkdir -p /var/lib/pgsql/data
chown -R postgres:postgres /var/lib/pgsql

# Inicializar base de datos
su - postgres -c "/opt/freeware/bin/initdb -D /var/lib/pgsql/data"

# Iniciar PostgreSQL
su - postgres -c "/opt/freeware/bin/pg_ctl -D /var/lib/pgsql/data -l /var/lib/pgsql/logfile start"

# Crear base de datos y usuario
su - postgres -c "/opt/freeware/bin/psql -c \"CREATE DATABASE sentinel;\""
su - postgres -c "/opt/freeware/bin/psql -c \"CREATE USER sentinel WITH ENCRYPTED PASSWORD 'your-password';\""
su - postgres -c "/opt/freeware/bin/psql -c \"GRANT ALL PRIVILEGES ON DATABASE sentinel TO sentinel;\""
```

### Instalación del Dashboard

```bash
# Instalar dependencias Python
PATH=/opt/freeware/bin:$PATH pip3 install flask psycopg2-binary gunicorn

# Ejecutar migración de base de datos
su - postgres -c "/opt/freeware/bin/psql -d sentinel -f dashboard/migrate.sql"

# Nota: Agregar columnas adicionales requeridas por la versión actual
su - postgres -c "/opt/freeware/bin/psql -d sentinel -c '
ALTER TABLE fingerprints
ADD COLUMN IF NOT EXISTS exit_code INTEGER,
ADD COLUMN IF NOT EXISTS process_count INTEGER,
ADD COLUMN IF NOT EXISTS zombie_count INTEGER,
ADD COLUMN IF NOT EXISTS memory_percent REAL,
ADD COLUMN IF NOT EXISTS load_1m REAL,
ADD COLUMN IF NOT EXISTS listener_count INTEGER,
ADD COLUMN IF NOT EXISTS unusual_port_count INTEGER,
ADD COLUMN IF NOT EXISTS uptime_days REAL,
ADD COLUMN IF NOT EXISTS audit_enabled BOOLEAN,
ADD COLUMN IF NOT EXISTS audit_risk_score INTEGER,
ADD COLUMN IF NOT EXISTS audit_risk_level VARCHAR(20),
ADD COLUMN IF NOT EXISTS audit_auth_failures INTEGER,
ADD COLUMN IF NOT EXISTS audit_sudo_count INTEGER,
ADD COLUMN IF NOT EXISTS audit_brute_force BOOLEAN;'"
```

### Iniciar el Dashboard

```bash
# Crear script de inicio
cat > /opt/freeware/bin/start-sentinel-dashboard.sh << 'EOF'
#!/bin/sh
cd /path/to/c-sentinel/dashboard
export DB_HOST=localhost
export DB_PORT=5432
export DB_NAME=sentinel
export DB_USER=sentinel
export DB_PASSWORD=your-password
export JWT_SECRET=your-random-secret-key
export SENTINEL_API_KEY=your-api-key
/opt/freeware/bin/python3.12 app.py
EOF

chmod +x /opt/freeware/bin/start-sentinel-dashboard.sh

# Iniciar dashboard
/opt/freeware/bin/start-sentinel-dashboard.sh
```

El dashboard estará disponible en `http://localhost:5000`

### Enviar Fingerprints al Dashboard

```bash
# Desde el mismo servidor AIX
sentinel -j -n | curl -X POST \
  -H "Content-Type: application/json" \
  -H "X-API-Key: your-api-key" \
  -d @- http://localhost:5000/api/ingest

# Desde otro servidor AIX
sentinel -j -n | curl -X POST \
  -H "Content-Type: application/json" \
  -H "X-API-Key: your-api-key" \
  -d @- http://dashboard-server:5000/api/ingest

# Agregar a crontab para monitoreo continuo (cada 5 minutos)
*/5 * * * * sentinel -j -n | curl -s -X POST \
  -H "Content-Type: application/json" \
  -H "X-API-Key: your-api-key" \
  -d @- http://dashboard-server:5000/api/ingest >/dev/null 2>&1
```

### Verificar Funcionamiento

```bash
# Health check
curl http://localhost:5000/health

# Ver hosts monitoreados
curl http://localhost:5000/api/hosts

# Ver estadísticas
curl http://localhost:5000/api/stats
```

## Diferencias con Linux

| Aspecto | Linux | AIX |
|---------|-------|-----|
| System info | sysinfo() | libperfstat |
| Process info | /proc/[pid]/stat (texto) | /proc/[pid]/psinfo (binario) |
| Network info | /proc/net/tcp | netstat -an |
| Network PIDs | Directo desde /proc/net/tcp | ✅ **Heurísticas (70+ puertos)** |
| Audit | auditd/ausearch | ✅ AIX audit nativo (auditpr) |
| File integrity | Manual | ✅ 171 archivos con `-F` |
| Long options | Soportado | No soportado |
| Dashboard | Funciona | ✅ Funciona (requiere PostgreSQL) |

## Soporte y Documentación

- **Repositorio:** https://github.com/librepower/c-sentinel4aix
- **Documentación completa:** [AIX_PORT_STATUS.md](AIX_PORT_STATUS.md)
- **Issues:** https://github.com/librepower/c-sentinel4aix/issues

## Licencia

MIT License - Ver archivo LICENSE para detalles

---

**Versión:** 0.6.0-aix
**Última actualización:** 2026-01-22
**Compatibilidad:** AIX 7.1, 7.2, 7.3
**Auditoría AIX:** ✅ Soportada (requiere `audit start`)
**Integridad Completa:** ✅ 171 archivos críticos con `-F`
**Dashboard:** ✅ Completamente funcional

---

## Rotación de Audit Trail (Opcional)

El audit trail de AIX (`/audit/trail`) puede crecer indefinidamente y llenar el disco. C-Sentinel incluye un script de rotación opcional:

```bash
# Configurar rotación (interactivo, requiere confirmación)
/opt/freeware/libexec/sentinel/setup-audit-rotation.sh
```

**Comportamiento:**
- Rota cuando trail supera 100MB
- Ejecuta diariamente a las 3am
- Mantiene 4 backups
- No modifica la configuración de audit existente

**Rotación manual:**
```bash
/opt/freeware/libexec/sentinel/audit-rotate.sh
```
