# Compatibilidad Linux para AIX

**Una capa de configuración para las herramientas GNU de IBM en AIX Toolbox**

Proporciona una experiencia de línea de comandos GNU/Linux familiar para administradores de AIX que vienen de entornos Linux.

## Qué Hace Este Paquete

Seamos claros sobre lo que es y lo que no es:

**No compilamos ninguna de estas herramientas.** IBM ya hizo ese trabajo, y lo hizo bien. Las GNU coreutils, grep, sed, awk y docenas de otras herramientas ya están disponibles en [IBM AIX Toolbox](https://www.ibm.com/support/pages/aix-toolbox-open-source-software-overview).

**Lo que IBM proporciona** (instalado como dependencias):
- GNU coreutils (ls, cp, mv, cat, head, tail, wc, sort, etc.)
- GNU grep, sed, awk, find, diff
- GNU tar con soporte gzip/bzip2/xz
- vim, tmux, jq, tree, y más

**Lo que nosotros añadimos:**
- Un perfil de shell que pone las herramientas GNU primero en tu PATH
- Aliases para operaciones comunes (`ll`, `la`, `lt`, etc.)
- Comandos emulados que no existen en AIX (`watch`, `pgrep`, `pkill`, `free`)
- Wrappers `systemctl` y `service` para AIX SRC (System Resource Controller)
- Documentación bilingüe

El mérito donde corresponde: **IBM hizo el trabajo duro de portar estas herramientas. Nosotros solo las hacemos más fáciles de usar por defecto.**

## Instalación

### Vía Repositorio DNF (Recomendado)

```bash
# Añadir repositorio LibrePower (una vez)
curl -fsSL https://aix.librepower.org/install.sh | sh

# Instalar
dnf install linux-compat

# Activar para tu usuario
linux-compat-setup install
source ~/.linux-compat-profile
```

### Instalación Manual

```bash
# Descargar RPM
curl -LO https://github.com/librepower/aix/releases/download/linux-compat-v2.1/linux-compat-2.1-1.librepower.aix7.3.noarch.rpm

# Instalar (dnf resuelve dependencias automáticamente)
dnf install ./linux-compat-2.1-1.librepower.aix7.3.noarch.rpm

# Activar para tu usuario
linux-compat-setup install
source ~/.linux-compat-profile
```

## Qué Obtienes

### Herramientas GNU Nativas (de IBM AIX Toolbox)

Estos son binarios reales, no emulaciones:

```bash
ls -lh --color=auto    # GNU ls con tamaños humanos y colores
grep -rP "patrón"      # Grep recursivo con regex Perl
find . -name "*.log" -mtime -1   # GNU find con todas las opciones
sed -i 's/viejo/nuevo/g'   # Edición in situ
tar -xzf archivo.tar.gz    # Extracción directa
```

### Gestión de Servicios (wrappers systemctl/service)

AIX usa SRC (System Resource Controller) en lugar de systemd. Proporcionamos wrappers familiares:

```bash
# Estilo systemctl (recomendado)
systemctl status              # Listar todos los servicios (lssrc -a)
systemctl start sshd          # Iniciar servicio (startsrc -s sshd)
systemctl stop sshd           # Detener servicio (stopsrc -s sshd)
systemctl restart sshd        # Reiniciar servicio
systemctl reload sshd         # Recargar config (refresh -s sshd)
systemctl is-active sshd      # Verificar si está activo

# Operaciones de grupo (usar prefijo @)
systemctl start @tcpip        # Iniciar todos los servicios TCP/IP
systemctl stop @nfs           # Detener todos los servicios NFS
systemctl status @tcpip       # Estado del grupo TCP/IP
systemctl list-groups         # Mostrar grupos disponibles

# Alternativa estilo SysV
service sshd start
service sshd stop
service sshd status

# Aliases rápidos
services                      # Listar todos (lssrc -a)
services-active               # Solo servicios activos
services-down                 # Solo servicios detenidos
```

**Nota:** `enable`/`disable` no están disponibles—AIX usa `/etc/rc.tcpip`, `/etc/inittab` para configuración de arranque.

### Comandos Emulados

Estos no existen nativamente en AIX, así que proporcionamos equivalentes como funciones de shell:

```bash
watch -n 5 'ps -ef | wc -l'   # Repetir comando cada 5 segundos
pgrep java                     # Encontrar PIDs por nombre de proceso
pkill -9 zombie                # Matar procesos por nombre
free                           # Uso de memoria (usa svmon)
```

### Aliases Convenientes

```bash
# Listado de directorios
ll                  # ls -lh con colores
la                  # ls -lha (mostrar ocultos)
lt                  # ls -lht (ordenar por tiempo)
lS                  # ls -lhS (ordenar por tamaño)

# Navegación
..                  # cd ..
...                 # cd ../..
mkcd dirname        # mkdir + cd

# Búsqueda
ff patrón           # Encontrar archivos por nombre
fif texto           # Buscar en archivos (grep -rn)
psg proceso         # ps -ef | grep

# Sistema
sysinfo             # Resumen rápido del sistema
top                 # Abre topas
df                  # Espacio en disco (legible)
path                # Mostrar entradas PATH numeradas
```

## Garantías de Seguridad

1. **Solo shells interactivos** - Scripts usando `#!/bin/sh` o `#!/bin/ksh` NO son afectados
2. **Comandos nativos preservados** - `/usr/bin/ls`, `/usr/bin/ps`, etc. siempre funcionan
3. **Reversible instantáneamente** - `linux-compat-setup uninstall` elimina todo
4. **Sin modificaciones al sistema** - Solo toca el directorio home del usuario

### Cómo Funciona

El perfil se carga desde `~/.profile` y verifica `$-` para modo interactivo:

```bash
case $- in
    *i*) ;;      # Interactivo - aplicar configuración
    *)   return 0 ;;  # No interactivo - no hacer nada
esac
```

## Gestionando la Instalación

```bash
# Verificar estado actual
linux-compat-setup status

# Deshabilitar temporalmente (sesión actual)
export LINUX_COMPAT_DISABLE=1
exec $SHELL

# Desinstalar completamente
linux-compat-setup uninstall

# Reinstalar
linux-compat-setup install
```

## Referencia Rápida

Después de activar, escribe `linuxhelp` para una tarjeta de referencia rápida.

## Requisitos

- AIX 7.2 o posterior
- DNF/YUM configurado con repositorio AIX Toolbox
- bash (incluido en dependencias)

## Agradecimientos

- **IBM** por portar herramientas GNU a AIX y mantener AIX Toolbox
- **Proyecto GNU** por las excelentes herramientas de línea de comandos
- **Comunidad AIX** por el soporte continuo de la plataforma

## Licencia

GPL-3.0 - Ver [LICENSE](../LICENSE)

---

*Parte de [LibrePower](https://librepower.org) - Desbloqueando Power Systems a través del código abierto*

*Mantenido por [LibrePower](https://librepower.org)*
